# Pty / standard-channel handling in Ck

This document describes how Ck's curses session is wired to the
process's standard channels and to the host terminal, why the previous
strategy (intercepting Tcl commands `puts`, `close`, `flush`, `read`,
`gets`, `exec`) was replaced, and how tests can drive a Ck session
against a private pty without disturbing the test harness's own tty.

## tl;dr

* `ck::open` is the single entry point that creates the Ck main window
  and starts a curses session.  Two modes:
  * **Controlling-tty mode** (default): takes over the process's tty,
    diverts `stdout`/`stderr` to a capture buffer for the duration of
    the session, drains the capture to the restored stderr at
    teardown.
  * **Explicit-pty mode** (`ck::open -pty $fd`): drives ncurses on a
    caller-supplied fd, leaves the host's `stdout`/`stderr` alone.
* `ck::pty open ?-rows N -cols M?` allocates a pty pair via
  `openpty(3)`, returns it as `{master slave}` Tcl channels for tests
  to drive.
* `pkgIndex.tcl` calls `ck::open .` automatically after loading the
  shared library, unless `::ck_no_auto_open` is set to a true value
  before `package require ck`.  The auto-open preserves backwards
  compatibility with older Ck applications.
* The Tcl-level `redirCommands` interception of
  `puts`/`close`/`flush`/`read`/`gets`/`exec` was removed.  The
  fd-level capture supersedes it and covers cases the script-level
  intercepts couldn't (e.g. `printf` from C extensions).

## Why the previous design didn't work

Ck inherited from Tk a strategy of renaming the standard Tcl I/O
commands and substituting wrappers that redirected output away from
`stdout` (which would corrupt the curses display).  The wrappers
captured the original command's `Tcl_CmdInfo` via
`Tcl_GetCommandInfo` and called the saved `cmdInfo.proc` after
adjusting the channel argument.

That model has three problems that compounded over time:

1. **It only catches Tcl-level output.**  An extension that calls
   `printf("...\n")` directly, or any code path that writes via
   `fwrite` to libc's `stdout` `FILE*`, completely bypasses the
   wrapper.  Same for the default `bgerror` fallback when it can't
   resolve `bgerror` itself: it goes straight to the underlying
   stderr.  In a curses session those writes corrupt the screen and
   the user has no way to recover them later.

2. **It silently swallows diagnostics.**  Background errors from
   `after`/`fileevent` callbacks, exit traces, leak reports from
   `valgrind`, and crash backtraces from a SEGV all rely on writes
   reaching the original stdout/stderr.  Under the redirect they
   either go nowhere visible or land in the middle of the curses
   buffer and disappear when the screen tears down.

3. **It's fragile under Tcl 9.**  Most of Tcl 9's core commands are
   pure objCmds, so the saved `cmdInfo.proc` is `NULL` and the
   wrapper crashes when it tries to call through it.  The previous
   `commit 8412c56 "Attempting to get tests working with stdout /
   stderr issues"` is the visible scar of that — the test suite was
   crashing on every dialog or `bgerror` invocation in a Tcl 9 build.

## The fd-swap design

When `ck::open` runs in controlling-tty mode, the C-level helper
`CkStdioSwap_Init` does:

1. `dup` fds 0, 1, 2 to high-numbered slots
   (`mainPtr->saved_stdin_fd`, `saved_stdout_fd`, `saved_stderr_fd`),
   each with `FD_CLOEXEC` set so a subsequent `exec` doesn't leak
   them.
2. `fdopen` the saved out and in fds as `FILE*`s
   (`mainPtr->uiOutFp`, `uiInFp`) with line buffering disabled, and
   pass them to `newterm(3)` as ncurses' display output and keyboard
   input streams.  ncurses now writes to the host's tty via the
   high-numbered dup, which still refers to the original
   open-file-description.
3. Open a capture target — `memfd_create(2)` on Linux,
   `tmpfile(3)` everywhere else — and `dup2` it over fds 1 and 2.
4. Flush Tcl's existing std channels so any in-flight buffered output
   reaches the original tty (via the dups, which still point at it),
   not the just-installed capture.

After step 4, every `write(1, ...)` and `write(2, ...)` from the rest
of the process — Tcl `puts`, libc `printf`, third-party loggers,
subprocess output that inherited the parent's fds — goes into the
capture buffer.  ncurses output goes to the tty via the
`uiOutFp`/`uiInFp` `FILE*`s.  The two streams don't interfere.

At teardown (`destroy .` or process exit via `Ck_ExitCmd`), the
companion helper `CkStdioSwap_Restore` runs:

1. Flushes Tcl/libc buffers that are still pointing at fds 1/2.
2. `dup2`s the saved fds back over 1 and 2 (so subsequent writes
   resume reaching the original tty).
3. Closes the saved stderr fd, which we own.  The saved stdin/stdout
   fds are owned by ncurses via the `FILE*`s and get freed by the
   subsequent `delscreen()`.
4. Drains the capture fd to the now-restored stderr — this is what
   makes background errors and stray printfs visible after the curses
   screen tears down, instead of vanishing.
5. Closes the capture fd.

The `CkMainInfo` struct grew six fields to track this state: four fd
ints, plus the two `FILE*` pointers.  All default to `-1` / `NULL`
when no swap is active, so `CkStdioSwap_Restore` is safe to call on
an uninitialised mainPtr (used in failure paths).

The order in `Ck_DestroyWindow` matters: `CkStdioSwap_Restore` must
run **before** `delscreen()`, because `delscreen` `fclose`s the saved
stdin/stdout `FILE*`s — which closes the underlying high-numbered
fds — and the dup we put on fds 1/2 in step 2 of Restore needs the
open file description to still be alive at that moment so it can
inherit the kernel-level reference.

## Why the redirCommands subsystem was removable

Once the fd-swap is doing its job, every reason `redirCommands`
existed is gone:

* `puts stdout` writes to fd 1 → capture buffer → drained to stderr
  at end → user sees it.
* `puts stderr` writes to fd 2 → same.
* C-level `printf` writes to fd 1 → capture → user sees it.
* Background errors via the default fallback go to stderr → capture
  → user sees them.

The Tcl-level wrappers used to drop or redirect these silently; the
fd-swap preserves them and shows them at a sensible time (after the
screen has been put back in a sane state).  The wrapper functions
(`PutsCmd`, `CloseCmd`, `FlushCmd`, `ReadCmd`, `GetsCmd`, `ExecCmd`),
the `RedirInfo` struct, the `redirCommands[]` table, and the
`CleanupRedirInfo` cleanup proc are all gone — about 290 lines of
ckWindow.c removed.

`exec` had a small extra feature (the `-endwin` option that suspended
curses around the child process so it could draw its own UI to the
tty).  That feature wasn't actually used in practice — and if anyone
needs it, it's a few lines of pure-Tcl wrapping `endwin` /
`reset_prog_mode` around `exec` rather than an objCmd-replacement
hack.

## Explicit-pty mode

`ck::open -pty $fd` short-circuits all of the above.  The caller has
already arranged a pty (typically via `ck::pty open` from a test, or
by other means in a more elaborate embedding scenario), and Ck's job
is just to bind the curses session to that fd.  `CkStdioSwap_Init`
takes a different path:

1. `dup` the supplied pty fd twice — once for read, once for write —
   so the two `FILE*`s ncurses gets (`r` and `w` modes) don't share
   buffer state.
2. `fdopen` each, hand them to `newterm`.
3. **Don't touch fds 0/1/2.**  The host process's standard streams
   stay unchanged; the test harness can keep using them for diagnostic
   output, tcltest progress, etc.
4. Don't allocate a capture buffer.  `saved_stderr_fd` and
   `capture_fd` are left at `-1`, which `CkStdioSwap_Restore` uses as
   its sentinel for "no swap to undo".

The optional `-rows N -cols M` flags on `ck::open` — when both are
supplied — issue a `TIOCSWINSZ` on the pty before `newterm` reads the
size, so the curses screen comes up at exactly the dimensions the
test wants regardless of the test process's own tty geometry.

## ck::pty open

`ck::pty open ?-rows N? ?-cols M?` is a Tcl-level wrapper around
`openpty(3)`.  Both ends of the resulting pair are wrapped in Tcl
channels via `Tcl_MakeFileChannel`, registered with the interp, and
returned as a two-element list.  Channel options applied at creation
time:

| End      | -translation | -buffering | -blocking |
|----------|--------------|------------|-----------|
| master   | binary       | none       | 0         |
| slave    | binary       | none       | (default) |

Master is non-blocking and unbuffered because tests typically want to
read whatever's currently available without hanging when the curses
session hasn't written more yet, and write keystrokes that appear at
the slave end immediately rather than queuing.  Slave keeps its
default blocking behaviour so injected keystrokes don't silently get
short-written.

The slave channel name can be passed directly to `ck::open -pty`; the
fd is extracted via `Tcl_GetChannelHandle`.  Integer fds are also
accepted for callers that already have one in hand (e.g. from a prior
`fork`).

If `-rows` and `-cols` are both supplied (and both positive), the
helper applies the resize to the pty before returning, so a session
started with `ck::open -pty $slave` comes up at the requested size.

## Auto-open and the `::ck_no_auto_open` suppression

The legacy contract is that `package require ck` produces a usable
Ck environment with the main window already created on the
controlling tty.  `pkgIndex.tcl` preserves this by calling
`ck::open .` itself after loading the shared library:

```tcl
if {![info exists ::ck_no_auto_open] || !$::ck_no_auto_open} {
    uplevel #0 [list ::ck::open .]
}
```

A test that wants to drive Ck against a private pty sets the global
beforehand:

```tcl
set ::ck_no_auto_open 1
package require ck
lassign [ck::pty open -rows 24 -cols 80] master slave
ck::open -pty $slave .
```

A plain top-level global (rather than `::ck::no_auto_open` in a
namespace) is used so the test can set it without first creating
`::ck` — that namespace doesn't exist until the package has loaded.

`::ck_no_auto_open` is consulted only at `package require` time;
toggling it after the package has loaded has no effect on subsequent
`ck::open` calls.

## Multi-session support and a Tcl-level subtlety

Once `ck::open` was decoupled from `Ck_Init`, running a sequence of
`ck::open` / `destroy .` cycles in the same interp became a supported
pattern (tcltest's `-singleproc` mode does it for every test that
opens its own pty).  This exercised an existing latent bug in the
auxiliary library scripts: `library/dialog.tcl` (and similar files)
register option-database defaults via top-level `option add` calls,
relying on Tcl's autoloader to source the file the first time
`ck_dialog` is referenced.  After `Ck_DestroyWindow` clears the
option DB, a *second* `ck::open` re-sources `library/ck.tcl` — but
not the dialog/etc files, because their procs are still defined
from the first session, so autoload doesn't fire.  The result was
that dialogs in the second session came up without their borders
or separator frames.

Fix: `library/ck.tcl` now `source`s the auxiliary library files
explicitly:

```tcl
source [file join $ck_library button.tcl]
source [file join $ck_library entry.tcl]
source [file join $ck_library listbox.tcl]
source [file join $ck_library scrollbar.tcl]
source [file join $ck_library text.tcl]
source [file join $ck_library menu.tcl]
source [file join $ck_library dialog.tcl]
source [file join $ck_library bgerror.tcl]
source [file join $ck_library msgbox.tcl]
source [file join $ck_library clrpick.tcl]
source [file join $ck_library ckfbox.tcl]
```

Every `ck::open` re-sources all of them, which re-runs the
`option add` calls and repopulates the option database.  The cost is
trivial (these are small files of binding declarations) and the
behaviour is now consistent across every `ck::open` in the same
interp.

## File-handler placement

`Tcl_CreateFileHandler` for the curses input fd used to be hardwired
to fd 0 (`STDIN_FILENO`).  That stops being correct in explicit-pty
mode, where ncurses isn't reading from fd 0 at all.  The current code
attaches the handler to `mainPtr->saved_stdin_fd` instead, which:

* In default mode is `dup(STDIN_FILENO)` — shares an
  open-file-description with fd 0, so a `select` on either sees the
  same readable events and a read via ncurses' `FILE*` drains the
  same kernel buffer.
* In explicit-pty mode is `dup(pty_fd)` — the right fd to wait on for
  the test-driver's keystrokes.

The corresponding `Tcl_DeleteFileHandler` runs in `Ck_DestroyWindow`
before `delscreen()`, which would otherwise leave a handler queued
against a freed `mainPtr`.  Same for the SIGWINCH pipe, the event
source, the exit handler, the pending `DoRefresh` idle callback, and
the `refreshTimer` — all explicitly torn down so a subsequent
`ck::open` in the same interp gets a clean slate.

## Failure modes

* `ck::open` (default mode) when stdout isn't a tty: `CkStdioSwap_Init`
  returns `TCL_ERROR` early, the caller falls through to a plain
  `newterm(NULL, stdout, stdin)` call which fails the same way the
  pre-fd-swap code did.  No fd surgery is performed, so `stdout` is
  left untouched.
* `ck::open -pty $fd` with an invalid fd: `dup` fails, `CkStdioSwap_Init`
  returns `TCL_ERROR`, and `ck::open` reports "couldn't initialise
  curses".  No fds are leaked.
* `memfd_create` not available (non-Linux): `OpenCaptureFd` falls
  through to `tmpfile(3)`.  A hardened embedder running with a
  full `/tmp` could see capture allocation fail — `CkStdioSwap_Init`
  bails and capture-on-exit is silently disabled rather than
  preventing the session from starting.

## Tests that exercise this

* `tests/pty.test` — `ck::pty open` plumbing, `-rows`/`-cols`
  threading through `TIOCSWINSZ`, basic key injection through the
  master fd, multi-keystroke sequence dispatch.
* `tests/button.test` — full-loop validation: a button widget is
  drawn through real ncurses to a private pty, the rendered text is
  parsed back via `tests/vt.tcl` and asserted, then a `<Return>`
  keystroke is injected and the binding is observed firing.
* `tests/dialog.test` — the snapshot-style cases (`dialog-1.4`,
  `dialog-1.5`) drive `ck_dialog` through a private pty, snapshot
  the parsed grid mid-dialog, and assert on title/body text and
  box-drawing border characters.  These were the tests that
  initially hit the `option add` regression and now pass cleanly
  across ten consecutive `meson test` invocations.

## What changed in C

| File                      | Change |
|---------------------------|--------|
| `generic/ckStdioSwap.c`   | New.  fd-swap implementation + capture drain. |
| `generic/ckOpen.c`        | New.  `ck::open` / `ck::pty open` objCmds. |
| `generic/ck.h`            | `CkMainInfo` gains `saved_stdin_fd`, `saved_stdout_fd`, `saved_stderr_fd`, `capture_fd`, `uiOutFp`, `uiInFp`. |
| `generic/ckInt.h`         | `CkOpenOptions`, `CkCreateMainWindowEx`, `CkStdioSwap_*` prototypes. |
| `generic/ckWindow.c`      | Removed: `redirCommands[]`, `RedirInfo`, `InvokeRedirected`, `PutsCmd`/`CloseCmd`/`FlushCmd`/`ReadCmd`/`GetsCmd`/`ExecCmd`, `CleanupRedirInfo`.  Added: `CkCreateMainWindowEx`, full teardown of file handlers / event sources / exit handlers / idle callbacks / timers in `Ck_DestroyWindow`.  `Ck_Init` no longer auto-creates the main window; that's now `ck::open`'s job. |
| `generic/ckCmds.c`        | `Ck_ExitCmd` simplified; `Ck_DestroyWindow` does all teardown. |
| `generic/ckBorder.c`      | Unrelated cleanup of a clang `acsc[i] >= 128`-on-signed-char dead-code warning that was uncovered while debugging. |
| `library/ck.tcl`          | Sources auxiliary library files explicitly so `option add` defaults survive a `destroy .` / `ck::open` cycle. |
| `pkgIndex.tcl.in`         | Calls `ck::open .` after the load, gated on `::ck_no_auto_open`. |
