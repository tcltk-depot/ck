# helpers.tcl — shared scaffolding for the pty-driven tests.
#
# Tests source this once after [package require tcltest], then call
# [ck::test::open_pty ...] to set up a private ck session and the
# [send_*] / [wait_for ...] helpers to drive it.

source [file join [file dirname [info script]] vt.tcl]

namespace eval ck::test {

    # open_pty ?-rows R? ?-cols C? — returns the master channel for
    # the caller to read/write.  Also stands up a vt screen of the same
    # size that test_state(vt) tracks alongside the byte stream, so
    # higher-level tests can assert on a parsed grid (vt_row, vt_cell)
    # instead of substring-matching raw escape sequences.
    proc open_pty {args} {
        variable test_state

        # default size: small enough that an off-by-one is loud
        set rows 24
        set cols 80
        foreach {opt val} $args {
            switch -- $opt {
                -rows { set rows $val }
                -cols { set cols $val }
                default { error "unknown option \"$opt\"" }
            }
        }

        lassign [ck::pty open -rows $rows -cols $cols] master slave
        ck::open -pty $slave -rows $rows -cols $cols .

        set test_state(master) $master
        set test_state(slave)  $slave
        set test_state(vt)     [::vt::new -rows $rows -cols $cols]

        # Feed ncurses' initial paint into the vt screen so the screen
        # already reflects the empty main window before the test body
        # runs — i.e. vt_row / vt_cell can be called immediately
        # without a prior pump.
        update
        ::vt::feed $test_state(vt) [read $master]

        return $master
    }

    # Reverse of open_pty: tear down the ck session, free the vt
    # screen, then close both ends of the pty.  Tolerates being called
    # twice.
    proc close_pty {} {
        variable test_state
        catch {destroy .}
        if {[info exists test_state(vt)]} {
            catch {::vt::destroy $test_state(vt)}
            unset test_state(vt)
        }
        if {[info exists test_state(master)]} {
            catch {chan close $test_state(master)}
            unset test_state(master)
        }
        if {[info exists test_state(slave)]} {
            catch {chan close $test_state(slave)}
            unset test_state(slave)
        }
    }

    # Inject literal bytes into the slave end (i.e., simulate keystrokes
    # arriving at ck's input fd).  Spins the event loop afterward so the
    # ncurses input read + binding dispatch completes synchronously
    # before the helper returns.
    proc send_keys {bytes} {
        variable test_state
        puts -nonewline $test_state(master) $bytes
        flush $test_state(master)
        update
    }

    # Poll a Tcl variable until it equals $target, or the timeout
    # expires.  Returns the value on success; raises on timeout — tests
    # should let that propagate so the failure surfaces.
    #
    # Crucially this checks the current value BEFORE entering vwait, to
    # avoid the standard race where the binding has already fired by the
    # time the test reaches vwait — vwait then waits for the *next*
    # modification and times out.  send_keys runs an update internally,
    # which may dispatch the binding in-line, so this race is the
    # default rather than the exception under the pty model.
    proc wait_for {varname target {timeout_ms 1000}} {
        upvar #0 $varname var
        set marker [list __ck_test_timeout__ [clock microseconds]]
        set after_id [after $timeout_ms [list set $varname $marker]]
        while {1} {
            if {[info exists var] && $var eq $target} {
                after cancel $after_id
                return $var
            }
            if {[info exists var] && $var eq $marker} {
                error "timed out after ${timeout_ms}ms waiting for\
                       \$$varname == $target (last value: [info exists var]\
                       ? \"[expr {[info exists var] ? $var : "<unset>"}]\")"
            }
            vwait $varname
        }
    }

    # Read everything pending on the master end.
    #
    # ncurses' redraw can land on the slave fd in multiple kernel
    # writes, and the test process may sample the master between them
    # (returning a short read).  Loop: drain → spin update → wait a
    # tick → repeat, until two consecutive iterations come up empty.
    # The dual-empty rule absorbs the short-write race; quiet_iters
    # raises the bar for noisy-screen tests if needed.
    #
    # Side effect: every byte read is also fed into the vt screen,
    # keeping vt_row / vt_cell in sync with whatever the master saw.
    proc read_master {{quiet_iters 2} {tick_ms 10}} {
        variable test_state
        set chan $test_state(master)
        set buf ""
        set quiet 0
        while {$quiet < $quiet_iters} {
            update
            set chunk [read $chan]
            if {$chunk eq ""} {
                incr quiet
                after $tick_ms [list set ::ck::test::__tick 1]
                vwait ::ck::test::__tick
                unset -nocomplain ::ck::test::__tick
            } else {
                append buf $chunk
                if {[info exists test_state(vt)]} {
                    ::vt::feed $test_state(vt) $chunk
                }
                set quiet 0
            }
        }
        return $buf
    }

    # Pump the master through the vt screen, then return row $r.  Most
    # tests just want "what's on the screen right now"; this is the
    # one-call helper for that — it folds in the read-and-feed pump so
    # the caller doesn't have to remember to drain the master first.
    proc vt_row {r} {
        variable test_state
        read_master
        return [::vt::row $test_state(vt) $r]
    }

    # Same shape as vt_row, for a single cell.
    proc vt_cell {r c} {
        variable test_state
        read_master
        return [::vt::cell $test_state(vt) $r $c]
    }

    # Return the entire screen as a list of row strings (post-pump).
    # Useful for "find the row containing X" or for printing the screen
    # state in a test failure diagnostic.
    proc vt_dump {} {
        variable test_state
        read_master
        return [::vt::dump $test_state(vt)]
    }

    # Search the parsed screen for a row containing the literal needle.
    # Returns the row index, or -1 if not found.  Saves test bodies
    # from writing the same lsearch / regexp loop repeatedly.
    proc vt_find_row {needle} {
        set rows [vt_dump]
        for {set r 0} {$r < [llength $rows]} {incr r} {
            if {[string first $needle [lindex $rows $r]] >= 0} {
                return $r
            }
        }
        return -1
    }
}
