package require tcltest

namespace import tcltest::test

tcltest::outputChannel stderr

# Tests run in the test harness's process, which is itself attached to a
# real (or pty-wrapped) controlling terminal.  The legacy [package
# require ck] auto-creates the main window on that terminal, which (a)
# corrupts the test harness's stdout, and (b) makes it impossible for
# tests to drive ck against a private pty.  Suppress the auto-open
# globally; tests that exercise widget behaviour are responsible for
# opening their own pty session via [ck::pty open] + [ck::open -pty].
set ::ck_no_auto_open 1

tcltest::loadTestedCommands

# Filter out emacs/etc. transient files; everything else uses the
# pty-driven model now.
tcltest::configure -notfile {*#*}

# Test configuration options that may be set are:
# (currently none)

tcltest::configure -singleproc true
tcltest::configure -testdir [file dirname [file normalize [info script]]]

if {[info exists env(TEMP)]} {
    tcltest::configure -tmpdir $::env(TEMP)/cffi-test/[clock seconds]
} else {
    if {[file exists /tmp] && [file isdirectory /tmp]} {
	tcltest::configure -tmpdir /tmp/cffi-test/[clock seconds]
    } else {
	error "Unable to figure out TEMP directory. Please set the TEMP env var"
    }
}

tcltest::configure {*}$argv

# ERROR_ON_FAILURES for github actions
set ErrorOnFailures [info exists env(ERROR_ON_FAILURES)]
# NOTE: Do NOT unset ERROR_ON_FAILURES if recursing to subdirectories
unset -nocomplain env(ERROR_ON_FAILURES)
if {[tcltest::runAllTests] && $ErrorOnFailures} {exit 1}

# if calling direct only (avoid rewrite exit if inlined or interactive):
if { [info exists ::argv0] && [file tail $::argv0] eq [file tail [info script]]
     && !([info exists ::tcl_interactive] && $::tcl_interactive)
 } {
    proc exit args {}
}

exit
