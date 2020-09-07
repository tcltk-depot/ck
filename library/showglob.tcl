#
# showglob.tcl --
#
# This file defines a command for retrieving Tcl global variables.
#
# Copyright (c) 1996 Christian Werner
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.

proc showglob args {
    set result {}
    if {[llength $args] == 0} {
	set args *
    }
    foreach i $args {
	foreach k [info globals $i] {
	    set glob($k) {}
	}
    }
    foreach i [lsort -ascii [array names glob]] {
	upvar #0 $i var
	if {[array exists var]} {
	    foreach k [lsort -ascii [array names var]] {
		lappend result [list set [list $i]($k) $var($k)]
	    }
	} else {
	    catch {lappend result [list set $i $var]}
	}
    }
    return [join $result "\n"]
}


