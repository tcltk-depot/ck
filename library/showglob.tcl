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
    array set glob {}
    foreach i $args {
	set sub global
	if {[string match *::* $i]} {
	    set sub vars
	}
	foreach k [info $sub $i] {
	    set glob($k) {}
	}
    }
    foreach i [lsort -dictionary [array names glob]] {
	upvar #0 $i var
	if {[array exists var]} {
	    foreach k [lsort -dictionary [array names var]] {
		lappend result [list set [list $i]($k) $var($k)]
	    }
	} else {
	    catch {lappend result [list set $i $var]}
	}
    }
    return [join $result "\n"]
}


