# bgerror.tcl --
#
# This file contains a default version of the tkError procedure.  It
# posts a dialog box with the error message and gives the user a chance
# to see a more detailed stack trace.
#
# Copyright (c) 1992-1994 The Regents of the University of California.
# Copyright (c) 1994-1995 Sun Microsystems, Inc.
# Copyright (c) 1999-2023 Christian Werner
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.

if {[winfo depth .] > 1} {
    option add *ckerrorDialog.border {
	ulcorner hline urcorner vline lrcorner hline llcorner vline
    } widgetDefault
    option add *ErrorTrace.border {
	ulcorner hline urcorner vline lrcorner hline llcorner vline
    } widgetDefault
    option add *ckerrorDialog*background red interactive
    option add *ErrorTrace*background red interactive
}

# bgerror --
# This is the default version of bgerror.  It posts a dialog box containing
# the error message and gives the user a chance to ask to see a stack
# trace.
#
# Arguments:
# err -			The error message.

proc bgerror err {
    global errorInfo
    set info $errorInfo
    set button [ck_dialog .ckerrorDialog "Error in Tcl Script" \
	    "Error: $err" " OK " Skip Trace]
    if {$button == 0} {
        return
    } elseif {$button == 1} {
        return -code break
    }
    set w .ckerrorTrace
    catch {destroy $w}
    toplevel $w -class ErrorTrace
    place $w -relx 0.5 -rely 0.5 -anchor center
    set border [expr {[$w cget -border] ne {}}]
    if {$border} {
	label $w.title -text "Stack Trace for Error"
	place $w.title -y 0 -relx 0.5 -anchor center -bordermode ignore
    } else {
	$w configure -border {{ }}
	label $w.title -text "Stack Trace for Error"
	pack $w.title -side top -fill x
        frame $w.sep0 -height 1
	pack $w.sep0 -side top -fill x
    }
    button $w.ok -text OK -command [list catch [list destroy $w]]
    scrollbar $w.scroll -command [list $w.text yview] -takefocus 0
    text $w.text -yscrollcommand [list $w.scroll set]
    if {$border} {
	frame $w.sep -border hline
    } else {
	frame $w.sep -border {{ }}
    }
    pack $w.ok -side bottom -ipadx 1
    pack $w.sep -side bottom -fill x
    pack $w.scroll -side right -fill y
    pack $w.text -side left -expand 1 -fill both
    $w.text insert 0.0 $info
    $w.text mark set insert 0.0
    bind $w.text <Tab> {focus [ck_focusNext %W] ; break}
    update idletasks
    focus $w.ok
    tkwait window $w
}

