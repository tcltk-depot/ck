# ck.tcl --
#
# Initialization script normally executed in the interpreter for each
# curses wish-based application.  Arranges class bindings for widgets.
#
# Copyright (c) 1992-1994 The Regents of the University of California.
# Copyright (c) 1994-1995 Sun Microsystems, Inc.
# Copyright (c) 1995-2025 Christian Werner
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.

if {$tcl_platform(platform) eq "windows"} {
    curses encoding IBM437
    set env(TERM) win32
}

# Inhibit exec of unknown commands

set auto_noexec 1

# Add this directory to the begin of the auto-load search path:

if {[info exists auto_path]} {
    set auto_path [linsert $auto_path 0 $ck_library]
}

# ----------------------------------------------------------------------
# Read in files that define class bindings AND files that contribute
# default option-database entries (option add ...) for built-in
# dialogs.  Sourcing these explicitly here matters: under repeated
# ck::open / destroy . cycles in the same interp (notably under
# tcltest -singleproc), Ck_DestroyWindow tears the option database
# down with the main window, but auto_path-driven autoload only fires
# *once* per interp because [info procs ck_dialog] is still defined.
# Without re-sourcing, the second session's [toplevel -class Dialog]
# can't find *Dialog.border, [$w cget -border] returns "", and the
# dialog renders without its border / separator frames.
# ----------------------------------------------------------------------

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

# ----------------------------------------------------------------------
# Default bindings for keyboard traversal.
# ----------------------------------------------------------------------

bind all <Tab> {focus [ck_focusNext %W]}
bind all <BackTab> {focus [ck_focusPrev %W]}
if {$tcl_interactive} {
    bind all <Control-c> ckCommand
    ckCommand
}

