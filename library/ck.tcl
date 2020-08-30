# ck.tcl --
#
# Initialization script normally executed in the interpreter for each
# curses wish-based application.  Arranges class bindings for widgets.
#
# Copyright (c) 1992-1994 The Regents of the University of California.
# Copyright (c) 1994-1995 Sun Microsystems, Inc.
# Copyright (c) 1995-2020 Christian Werner
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.

# Insist on running with compatible versions of Tcl and Ck.

apply {{tv cv} {
    scan $tv "%d.%d" a b
    scan $cv "%d.%d" c d
    if {$a == 8} {
	if {$c != 8} {
	    error "wrong version of Ck loaded ($c.$d): need 8.X"
	}
	if {$d != $b} {
	    error "wrong version of Ck loaded ($c.$d): need 8.$b"
	} 
    } else {
	error "wrong version of Tcl loaded ($a.b): need 8.x"
    }
}} [info tclversion] $ck_version

if {$tcl_platform(platform) eq "windows"} {
    curses encoding IBM437
    set env(TERM) win32
}

# Inhibit exec of unknown commands

set auto_noexec 1

# Add this directory to the begin of the auto-load search path:

if {[info exists auto_path]} {
    set auto_path [concat $ck_library $auto_path]
}

# ----------------------------------------------------------------------
# Read in files that define all of the class bindings.
# ----------------------------------------------------------------------

source $ck_library/button.tcl
source $ck_library/entry.tcl
source $ck_library/listbox.tcl
source $ck_library/scrollbar.tcl
source $ck_library/text.tcl
source $ck_library/menu.tcl

# ----------------------------------------------------------------------
# Default bindings for keyboard traversal.
# ----------------------------------------------------------------------

bind all <Tab> {focus [ck_focusNext %W]}
bind all <BackTab> {focus [ck_focusPrev %W]}
if {$tcl_interactive} {
    bind all <Control-c> ckCommand
    ckCommand
}

