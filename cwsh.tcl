# cwsh.tcl - similar to the the binary but libck is loaded
set tcl_interactive [expr {[llength $argv] == 0}]
package require Ck
array set ckPriv {}
if {!$tcl_interactive} {
    apply {argv {
	set rest [lassign $argv file]
	switch -- $file {
	    -e - -en - -enc - -enco - -encod - -encodi - -encodin - -encoding {
		unset file
		lassign $argv dummy enc file
	    }
	}
	try {
	    if {[info exists enc]} {
		uplevel \#0 source -encoding $enc [list $file]
	    } else {
		uplevel \#0 source [list $file]
	    }
	} on error msg {
	    ckCommand
	    bgerror $msg
	}
    }} $argv
}
vwait ckPriv(forever)
