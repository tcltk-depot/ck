/*
 * ckStdioSwap.c --
 *
 *	Stdio capture for Ck sessions.  ncurses needs exclusive access to the
 *	real terminal, but Tcl extensions, libc, and any package the host
 *	app loads can write to fds 1/2 at any time and corrupt the display.
 *	The Tk/Ck heritage of intercepting puts/close/flush at the Tcl
 *	command layer only addresses the Tcl side and is fragile under
 *	objCmd-only Tcl 9.  Instead, when a Ck main window is created we:
 *
 *	  1. dup() the existing fds 0/1/2 to high-numbered slots,
 *	  2. fdopen() the saved out/in fds for ncurses' newterm(),
 *	  3. open a capture target (memfd_create on Linux, tmpfile()
 *	     elsewhere) and dup2() it onto fds 1 and 2,
 *	  4. invalidate Tcl's cached std channels so Tcl re-derives them
 *	     against the new fd 1/2.
 *
 *	On teardown we restore fds 1/2 from the saved copies, invalidate
 *	the std channels again, and dump the captured bytes to the now-
 *	restored stderr so background errors and stray printfs are not lost.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include "ckPort.h"
#include "ck.h"

#include <unistd.h>
#include <fcntl.h>

#include <sys/ioctl.h>

#ifdef __linux__
#  include <sys/syscall.h>
#  ifdef __NR_memfd_create
#    include <linux/memfd.h>
#    define HAVE_MEMFD_CREATE 1
#  endif
#endif

/*
 * OpenCaptureFd --
 *
 *	Returns a writable, seekable, readable fd to use as the capture
 *	target.  Prefers memfd_create() (Linux 3.17+) so the buffer never
 *	hits the filesystem; falls back to tmpfile() so the call always
 *	succeeds on a sane Unix.  Returns -1 if both fail.
 */
static int
OpenCaptureFd(void)
{
#ifdef HAVE_MEMFD_CREATE
    int fd = (int) syscall(__NR_memfd_create, "ck-stdio-capture",
			   (unsigned int) MFD_CLOEXEC);
    if (fd >= 0) {
	return fd;
    }
#endif
    {
	FILE *fp = tmpfile();
	if (fp != NULL) {
	    int fd = dup(fileno(fp));
	    int saved_errno = errno;
	    fclose(fp);
	    if (fd >= 0) {
		(void) fcntl(fd, F_SETFD, FD_CLOEXEC);
		return fd;
	    }
	    errno = saved_errno;
	}
    }
    return -1;
}

/*
 *----------------------------------------------------------------------
 *
 * CkStdioSwap_Init --
 *
 *	Set up the FILE*s ncurses' newterm() will use, and (in the default
 *	path) divert the host's stdout/stderr to a capture buffer.
 *
 *	If opts->pty_fd >= 0, use that fd directly: dup it twice, fdopen
 *	one for read and one for write, and leave fds 0/1/2 of the host
 *	process untouched.  This is the path tests use after creating a
 *	pty pair via ck::pty open.
 *
 *	If opts is NULL or opts->pty_fd < 0, the default path runs: dup
 *	fds 0/1/2 to high-numbered slots, redirect 1/2 to a capture
 *	buffer (memfd_create or tmpfile), and build FILE*s on the saved
 *	0/1 fds for newterm.  CkStdioSwap_Restore drains the capture to
 *	the restored stderr at teardown.
 *
 *	On success, mainPtr->uiOutFp / mainPtr->uiInFp hold the FILE*s to
 *	pass to newterm() and the bookkeeping fields are populated.  On
 *	failure returns TCL_ERROR with all fields cleared and the host's
 *	original fds untouched.
 *
 *----------------------------------------------------------------------
 */
int
CkStdioSwap_Init(CkMainInfo *mainPtr, const CkOpenOptions *opts)
{
    int s0 = -1, s1 = -1, s2 = -1, cap = -1;
    FILE *outFp = NULL, *inFp = NULL;
    int pty_fd = (opts != NULL) ? opts->pty_fd : -1;

    mainPtr->saved_stdin_fd  = -1;
    mainPtr->saved_stdout_fd = -1;
    mainPtr->saved_stderr_fd = -1;
    mainPtr->capture_fd      = -1;
    mainPtr->uiOutFp         = NULL;
    mainPtr->uiInFp          = NULL;

    /*
     * Explicit-pty path: caller handed us an open fd (typically the
     * slave end of a pty pair).  Don't touch fds 0/1/2 — the host
     * process keeps full control of its own stdio.  Just dup the pty
     * twice so we can fdopen one for read and one for write without
     * the two FILE*s sharing buffer state.  Optionally apply
     * TIOCSWINSZ before newterm() reads the size.
     */
    if (pty_fd >= 0) {
	s0 = dup(pty_fd);
	s1 = dup(pty_fd);
	if (s0 < 0 || s1 < 0) {
	    goto fail;
	}
	(void) fcntl(s0, F_SETFD, FD_CLOEXEC);
	(void) fcntl(s1, F_SETFD, FD_CLOEXEC);

	if (opts != NULL && opts->rows > 0 && opts->cols > 0) {
	    struct winsize ws;
	    memset(&ws, 0, sizeof(ws));
	    ws.ws_row = (unsigned short) opts->rows;
	    ws.ws_col = (unsigned short) opts->cols;
	    (void) ioctl(pty_fd, TIOCSWINSZ, &ws);
	}

	inFp  = fdopen(s0, "r");
	outFp = fdopen(s1, "w");
	if (inFp == NULL || outFp == NULL) {
	    if (inFp  != NULL) { fclose(inFp);  s0 = -1; inFp  = NULL; }
	    if (outFp != NULL) { fclose(outFp); s1 = -1; outFp = NULL; }
	    goto fail;
	}
	setvbuf(outFp, NULL, _IONBF, 0);

	/*
	 * No fd swap: capture_fd stays -1, saved_stderr_fd stays -1, and
	 * saved_stdin_fd / saved_stdout_fd record the dup'd fds purely so
	 * Restore() / delscreen() can find them — they're owned by the
	 * FILE*s and freed when ncurses fcloses them.
	 */
	mainPtr->saved_stdin_fd  = s0;
	mainPtr->saved_stdout_fd = s1;
	mainPtr->uiOutFp         = outFp;
	mainPtr->uiInFp          = inFp;
	return TCL_OK;
    }

    /*
     * Default path: no caller-supplied pty.  Bail out early if we don't
     * have a real terminal on stdout — ncurses isn't going to work
     * anyway, so let the caller fall through to its existing failure
     * path with no fd surgery performed.
     */
    if (!isatty(STDOUT_FILENO)) {
	return TCL_ERROR;
    }

    s0 = dup(STDIN_FILENO);
    s1 = dup(STDOUT_FILENO);
    s2 = dup(STDERR_FILENO);
    if (s0 < 0 || s1 < 0 || s2 < 0) {
	goto fail;
    }
    (void) fcntl(s0, F_SETFD, FD_CLOEXEC);
    (void) fcntl(s1, F_SETFD, FD_CLOEXEC);
    (void) fcntl(s2, F_SETFD, FD_CLOEXEC);

    cap = OpenCaptureFd();
    if (cap < 0) {
	goto fail;
    }

    /*
     * Flush anything Tcl or libc has buffered against the old fds before
     * pulling them out from under it.
     */
    {
	Tcl_Channel chan;
	chan = Tcl_GetStdChannel(TCL_STDOUT);
	if (chan != NULL) Tcl_Flush(chan);
	chan = Tcl_GetStdChannel(TCL_STDERR);
	if (chan != NULL) Tcl_Flush(chan);
    }
    fflush(stdout);
    fflush(stderr);

    if (dup2(cap, STDOUT_FILENO) < 0 || dup2(cap, STDERR_FILENO) < 0) {
	goto fail;
    }
    /*
     * Tcl's std channels hold raw int fds, so they automatically follow the
     * dup2 — no cache invalidation needed (and Tcl_SetStdChannel(NULL,...)
     * actually closes the channel, which would drop any data still in
     * its buffer).
     */

    outFp = fdopen(s1, "w");
    inFp  = fdopen(s0, "r");
    if (outFp == NULL || inFp == NULL) {
	/*
	 * If only one fdopen succeeded we'd close that FILE* below; restore
	 * the dup2'd fds before returning so the caller sees a clean slate.
	 */
	if (outFp != NULL) { fclose(outFp); s1 = -1; outFp = NULL; }
	if (inFp  != NULL) { fclose(inFp);  s0 = -1; inFp  = NULL; }
	(void) dup2(s2, STDERR_FILENO);
	(void) dup2(s1 >= 0 ? s1 : s2, STDOUT_FILENO);
	goto fail;
    }
    setvbuf(outFp, NULL, _IONBF, 0);

    mainPtr->saved_stdin_fd  = s0;
    mainPtr->saved_stdout_fd = s1;
    mainPtr->saved_stderr_fd = s2;
    mainPtr->capture_fd      = cap;
    mainPtr->uiOutFp         = outFp;
    mainPtr->uiInFp          = inFp;
    return TCL_OK;

fail:
    if (cap >= 0) close(cap);
    if (s0  >= 0) close(s0);
    if (s1  >= 0) close(s1);
    if (s2  >= 0) close(s2);
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * CkStdioSwap_Restore --
 *
 *	Undo the fd-swap performed by CkStdioSwap_Init: put fds 1/2 back to
 *	the original tty, invalidate Tcl's std channels, then drain the
 *	capture fd to the restored stderr so any printf/puts that happened
 *	during the Ck session is preserved for the user (or test harness).
 *	Closes the saved fds and the capture fd; the FILE* wrappers are
 *	owned by ncurses and freed by delscreen().
 *
 *	Safe to call on a mainPtr that didn't successfully init the swap —
 *	the saved_*_fd == -1 sentinel makes this a no-op.
 *
 *----------------------------------------------------------------------
 */
void
CkStdioSwap_Restore(CkMainInfo *mainPtr)
{
    int cap = mainPtr->capture_fd;
    int sout = mainPtr->saved_stdout_fd;
    int serr = mainPtr->saved_stderr_fd;

    /*
     * Two paths through here:
     *
     *  - Default (fd-swap) session: saved_stderr_fd >= 0 and capture_fd >= 0.
     *    Reverse the dup2(capture, 1/2) so fds 1/2 point at the original tty
     *    again, then drain the capture fd to the restored stderr.
     *
     *  - Explicit -pty session: saved_stderr_fd < 0 and capture_fd < 0.
     *    The host's fds 0/1/2 were never touched.  Nothing to dup2 back, no
     *    capture buffer to drain — just clear the bookkeeping.  ncurses
     *    still owns the dup'd pty fds via uiOutFp/uiInFp; delscreen()
     *    fcloses them after we return.
     */
    if (serr >= 0) {
	/*
	 * Flush Tcl/libc buffers through the still-capture-backed std
	 * channels first — once dup2 happens, fd 1/2 are the user's tty
	 * again and later flushes would emit on the wrong target.
	 */
	Tcl_Channel chan;
	chan = Tcl_GetStdChannel(TCL_STDOUT);
	if (chan != NULL) Tcl_Flush(chan);
	chan = Tcl_GetStdChannel(TCL_STDERR);
	if (chan != NULL) Tcl_Flush(chan);
	fflush(stdout);
	fflush(stderr);

	if (sout >= 0) (void) dup2(sout, STDOUT_FILENO);
	(void) dup2(serr, STDERR_FILENO);
	(void) close(serr);

	/*
	 * Drain the capture to the now-restored stderr so the user sees
	 * whatever was emitted during the session (background errors,
	 * package logging, debug printfs).  Best-effort — partial writes
	 * are tolerated.
	 */
	if (cap >= 0) {
	    off_t end = lseek(cap, 0, SEEK_END);
	    if (end > 0 && lseek(cap, 0, SEEK_SET) == 0) {
		char buf[4096];
		ssize_t n;
		while ((n = read(cap, buf, sizeof(buf))) > 0) {
		    ssize_t off = 0;
		    while (off < n) {
			ssize_t w = write(STDERR_FILENO, buf + off,
					  (size_t)(n - off));
			if (w <= 0) break;
			off += w;
		    }
		}
	    }
	    close(cap);
	}
    }

    mainPtr->saved_stdin_fd  = -1;
    mainPtr->saved_stdout_fd = -1;
    mainPtr->saved_stderr_fd = -1;
    mainPtr->capture_fd      = -1;
    mainPtr->uiOutFp         = NULL;
    mainPtr->uiInFp          = NULL;
}
