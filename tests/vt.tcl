# vt.tcl — minimal pure-Tcl VT100/220 parser, scoped to the subset of
# escape sequences ncurses (and therefore Ck) actually emits.  The
# point is to give tests a queryable cell grid instead of having to
# substring-match raw escape-soaked bytes from the master pty.
#
# Public API:
#
#   vt::new ?-rows R? ?-cols C?
#       Returns a screen handle (an opaque namespace-qualified name).
#
#   vt::feed $handle $bytes
#       Process bytes through the state machine, mutating the screen.
#
#   vt::row $handle $r
#       Return row $r (0-based) as a string of length $cols.  Trailing
#       whitespace is preserved so column-aligned assertions still work.
#
#   vt::cell $handle $r $c
#       Return the single character at ($r, $c).
#
#   vt::dump $handle
#       Return all rows as a list of strings.
#
#   vt::cursor $handle
#       Return {row col}, both 0-based.
#
#   vt::unhandled $handle
#       Return the list of CSI / escape sequences the parser saw but
#       didn't act on.  Useful when a test fails because the state
#       machine missed a sequence — extending the parser then becomes
#       a focused exercise rather than a guessing game.
#
#   vt::destroy $handle
#       Free the screen.
#
# Coverage:
#   - Cursor movement: CUU/CUD/CUF/CUB (CSI A/B/C/D),
#     CUP/HVP (CSI H/f), CHA (CSI G), VPA (CSI d).
#   - Erase: ED (CSI J), EL (CSI K), all three variants.
#   - Save/restore cursor: ESC 7/8 and CSI s/u.
#   - Scroll region: DECSTBM (CSI r), used by IND (ESC D)
#     and RI (ESC M) for in-region scroll.
#   - SGR (CSI m): parsed but not applied — cell attrs are not yet
#     tracked.  The parser simply consumes these without complaint.
#   - DEC private modes (CSI ? ... h/l): silently ignored.
#   - Charsets: ESC ( 0 / ESC ( B and SO (\x0e) / SI (\x0f) toggle a
#     line-drawing translation table, mapping `q` → ─, `x` → │, etc.
#     Both designate-and-activate (ESC ( 0) and explicit SO/SI work.

namespace eval ::vt {
    variable counter 0

    # DEC special graphics → Unicode mapping.  Built from the standard
    # VT100 charset as documented in xterm's ctlseqs and the original
    # DEC manuals — covers everything ncurses emits for borders and
    # progress-bar style decorations.
    variable dec_special
    array set dec_special {
        `   ◆  a   ▒  b   ␉  c   ␌
        d   ␍  e   ␊  f   °  g   ±
        h   ␤  i   ␋  j   ┘  k   ┐
        l   ┌  m   └  n   ┼  o   ⎺
        p   ⎻  q   ─  r   ⎼  s   ⎽
        t   ├  u   ┤  v   ┴  w   ┬
        x   │  y   ≤  z   ≥  \{  π
        |   ≠  \}  £  ~   ·
    }
}

# ----------------------------------------------------------------------
# Public API
# ----------------------------------------------------------------------

proc ::vt::new {args} {
    variable counter
    set rows 24
    set cols 80
    foreach {opt val} $args {
        switch -- $opt {
            -rows { set rows $val }
            -cols { set cols $val }
            default { error "vt::new: unknown option \"$opt\"" }
        }
    }
    if {$rows <= 0 || $cols <= 0} {
        error "vt::new: rows and cols must be positive"
    }

    set name [namespace current]::screen[incr counter]
    upvar #0 $name S

    set S(rows)         $rows
    set S(cols)         $cols
    set S(cur_row)      0
    set S(cur_col)      0
    set S(saved_row)    0
    set S(saved_col)    0
    set S(scroll_top)   0
    set S(scroll_bot)   [expr {$rows - 1}]
    set S(state)        ground
    set S(params)       {}
    set S(curparam)     ""
    set S(private)      0
    set S(line_drawing) 0
    set S(unhandled)    {}

    set blank [string repeat " " $cols]
    for {set r 0} {$r < $rows} {incr r} {
        set S(row,$r) $blank
    }
    return $name
}

proc ::vt::destroy {name} {
    upvar #0 $name S
    array unset S
}

proc ::vt::feed {name bytes} {
    upvar #0 $name S
    set i 0
    set n [string length $bytes]
    while {$i < $n} {
        set ch [string index $bytes $i]
        switch -- $S(state) {
            ground       { _ground   S $ch }
            escape       { _escape   S $ch }
            csi          { _csi      S $ch }
            osc          { _osc      S $ch }
            charset      { _charset  S $ch }
            esc_st       { _esc_st   S $ch }
        }
        incr i
    }
}

proc ::vt::row {name r} {
    upvar #0 $name S
    if {$r < 0 || $r >= $S(rows)} {
        error "vt::row: row $r out of range \[0, $S(rows))"
    }
    return $S(row,$r)
}

proc ::vt::cell {name r c} {
    upvar #0 $name S
    if {$r < 0 || $r >= $S(rows) || $c < 0 || $c >= $S(cols)} {
        error "vt::cell: ($r, $c) out of range"
    }
    return [string index $S(row,$r) $c]
}

proc ::vt::dump {name} {
    upvar #0 $name S
    set rows {}
    for {set r 0} {$r < $S(rows)} {incr r} {
        lappend rows $S(row,$r)
    }
    return $rows
}

proc ::vt::cursor {name} {
    upvar #0 $name S
    return [list $S(cur_row) $S(cur_col)]
}

proc ::vt::unhandled {name} {
    upvar #0 $name S
    return $S(unhandled)
}

# ----------------------------------------------------------------------
# State-machine handlers (private)
# ----------------------------------------------------------------------

proc ::vt::_ground {SVar ch} {
    upvar 1 $SVar S
    set b [scan $ch %c]
    if {$b >= 0x20 && $b != 0x7f} {
        # Printable.  Apply line-drawing translation if active.
        if {$S(line_drawing)} {
            variable dec_special
            if {[info exists dec_special($ch)]} {
                set ch $dec_special($ch)
            }
        }
        _put S $ch
        return
    }
    # NB: switch matches strings, not numbers, so the labels here are
    # decimal — "0x0a" as a switch label is the literal four-char
    # string, not byte 10.
    #
    #   7 BEL   8 BS    9 HT
    #  10 LF   11 VT   12 FF   13 CR
    #  14 SO   15 SI
    #  27 ESC
    switch -- $b {
        7 { }
        8 {
            if {$S(cur_col) > 0} { incr S(cur_col) -1 }
        }
        9 {
            set S(cur_col) [expr {($S(cur_col) | 7) + 1}]
            if {$S(cur_col) >= $S(cols)} {
                set S(cur_col) [expr {$S(cols) - 1}]
            }
        }
        10 -
        11 -
        12 { _newline S }
        13 { set S(cur_col) 0 }
        14 { set S(line_drawing) 1 }
        15 { set S(line_drawing) 0 }
        27 { set S(state) escape }
        default { }
    }
}

proc ::vt::_put {SVar ch} {
    upvar 1 $SVar S
    if {$S(cur_col) >= $S(cols)} {
        # auto-wrap to next line
        set S(cur_col) 0
        _newline S
    }
    if {$S(cur_row) < 0 || $S(cur_row) >= $S(rows)} {
        return
    }
    set row $S(row,$S(cur_row))
    set S(row,$S(cur_row)) \
        [string replace $row $S(cur_col) $S(cur_col) $ch]
    incr S(cur_col)
}

proc ::vt::_newline {SVar} {
    upvar 1 $SVar S
    if {$S(cur_row) >= $S(scroll_bot)} {
        # Scroll up within scroll region.
        set blank [string repeat " " $S(cols)]
        for {set r $S(scroll_top)} {$r < $S(scroll_bot)} {incr r} {
            set S(row,$r) $S(row,[expr {$r + 1}])
        }
        set S(row,$S(scroll_bot)) $blank
        # Cursor stays on bottom row of region.
    } else {
        incr S(cur_row)
    }
}

proc ::vt::_reverse_index {SVar} {
    upvar 1 $SVar S
    if {$S(cur_row) <= $S(scroll_top)} {
        # Scroll down within scroll region.
        set blank [string repeat " " $S(cols)]
        for {set r $S(scroll_bot)} {$r > $S(scroll_top)} {incr r -1} {
            set S(row,$r) $S(row,[expr {$r - 1}])
        }
        set S(row,$S(scroll_top)) $blank
    } else {
        incr S(cur_row) -1
    }
}

proc ::vt::_escape {SVar ch} {
    upvar 1 $SVar S
    switch -- $ch {
        "\["    { set S(state) csi
                  set S(params) {}
                  set S(curparam) ""
                  set S(private) 0 }
        "\]"    { set S(state) osc }
        "("     -
        ")"     -
        "*"     -
        "+"     { set S(state) charset }
        "7"     { set S(saved_row) $S(cur_row)
                  set S(saved_col) $S(cur_col)
                  set S(state) ground }
        "8"     { set S(cur_row) $S(saved_row)
                  set S(cur_col) $S(saved_col)
                  set S(state) ground }
        "M"     { _reverse_index S
                  set S(state) ground }                ;# RI
        "D"     { _newline S
                  set S(state) ground }                ;# IND
        "E"     { set S(cur_col) 0
                  _newline S
                  set S(state) ground }                ;# NEL
        "P"     -
        "X"     -
        "^"     -
        "_"     { set S(state) esc_st }                ;# DCS/SOS/PM/APC
        default {
            # =, >, c, ..., plus garbage — we just return to ground.
            lappend S(unhandled) "ESC $ch"
            set S(state) ground
        }
    }
}

proc ::vt::_csi {SVar ch} {
    upvar 1 $SVar S
    set b [scan $ch %c]
    if {$b >= 0x30 && $b <= 0x39} {
        # digit
        append S(curparam) $ch
        return
    }
    if {$ch eq ";"} {
        lappend S(params) $S(curparam)
        set S(curparam) ""
        return
    }
    if {$ch eq "?" && $S(params) eq "" && $S(curparam) eq ""} {
        set S(private) 1
        return
    }
    if {$b >= 0x40 && $b <= 0x7e} {
        # Final byte — flush current param then dispatch.
        if {$S(curparam) ne "" || [llength $S(params)] > 0} {
            lappend S(params) $S(curparam)
        }
        _csi_dispatch S $ch
        set S(state) ground
        set S(params) {}
        set S(curparam) ""
        set S(private) 0
        return
    }
    # Intermediate (sp..\x2f) — collected but ignored for now.
}

proc ::vt::_csi_param {params n {def 1}} {
    set v [lindex $params [expr {$n - 1}]]
    if {$v eq ""} { return $def }
    return $v
}

proc ::vt::_csi_dispatch {SVar ch} {
    upvar 1 $SVar S
    set p $S(params)

    if {$S(private)} {
        # DEC private modes (CSI ? Ps h/l, etc.).  Tracked extensions —
        # alt-screen (?1049), cursor visibility (?25), bracketed paste
        # (?2004) — would go here.  Currently no-op.
        return
    }

    switch -- $ch {
        "A" {
            set n [_csi_param $p 1 1]
            set S(cur_row) [expr {max(0, $S(cur_row) - $n)}]
        }
        "B" {
            set n [_csi_param $p 1 1]
            set S(cur_row) [expr {min($S(rows) - 1, $S(cur_row) + $n)}]
        }
        "C" {
            set n [_csi_param $p 1 1]
            set S(cur_col) [expr {min($S(cols) - 1, $S(cur_col) + $n)}]
        }
        "D" {
            set n [_csi_param $p 1 1]
            set S(cur_col) [expr {max(0, $S(cur_col) - $n)}]
        }
        "G" {
            # CHA — column absolute (1-based)
            set c [_csi_param $p 1 1]
            set S(cur_col) [expr {min($S(cols) - 1, max(0, $c - 1))}]
        }
        "d" {
            # VPA — row absolute (1-based)
            set r [_csi_param $p 1 1]
            set S(cur_row) [expr {min($S(rows) - 1, max(0, $r - 1))}]
        }
        "H" -
        "f" {
            # CUP/HVP — cursor position (row;col, 1-based)
            set r [_csi_param $p 1 1]
            set c [_csi_param $p 2 1]
            set S(cur_row) [expr {min($S(rows) - 1, max(0, $r - 1))}]
            set S(cur_col) [expr {min($S(cols) - 1, max(0, $c - 1))}]
        }
        "J" { _erase_display S [_csi_param $p 1 0] }
        "K" { _erase_line    S [_csi_param $p 1 0] }
        "m" {
            # SGR — parsed but not applied (no per-cell attrs yet).
        }
        "r" {
            # DECSTBM — set scroll region (top;bot, 1-based).  Per spec
            # the cursor moves to (1, 1) after this.
            set top [_csi_param $p 1 1]
            set bot [_csi_param $p 2 $S(rows)]
            set S(scroll_top) [expr {max(0, $top - 1)}]
            set S(scroll_bot) [expr {min($S(rows) - 1, $bot - 1)}]
            if {$S(scroll_bot) < $S(scroll_top)} {
                set S(scroll_top) 0
                set S(scroll_bot) [expr {$S(rows) - 1}]
            }
            set S(cur_row) 0
            set S(cur_col) 0
        }
        "s" {
            set S(saved_row) $S(cur_row)
            set S(saved_col) $S(cur_col)
        }
        "u" {
            set S(cur_row) $S(saved_row)
            set S(cur_col) $S(saved_col)
        }
        "h" - "l" {
            # Non-private mode set/reset — ignored.
        }
        default {
            lappend S(unhandled) "CSI [join $p {;}]$ch"
        }
    }
}

proc ::vt::_erase_display {SVar mode} {
    upvar 1 $SVar S
    set blank [string repeat " " $S(cols)]
    switch -- $mode {
        0 {
            # cursor → end of screen
            _erase_line S 0
            for {set r [expr {$S(cur_row) + 1}]} {$r < $S(rows)} {incr r} {
                set S(row,$r) $blank
            }
        }
        1 {
            # start of screen → cursor (inclusive)
            for {set r 0} {$r < $S(cur_row)} {incr r} {
                set S(row,$r) $blank
            }
            _erase_line S 1
        }
        2 -
        3 {
            for {set r 0} {$r < $S(rows)} {incr r} {
                set S(row,$r) $blank
            }
        }
    }
}

proc ::vt::_erase_line {SVar mode} {
    upvar 1 $SVar S
    set row $S(row,$S(cur_row))
    set cols $S(cols)
    set col $S(cur_col)
    switch -- $mode {
        0 {
            # cursor → end of line
            set fill [string repeat " " [expr {$cols - $col}]]
            set S(row,$S(cur_row)) [string replace $row $col end $fill]
        }
        1 {
            # start of line → cursor (inclusive)
            set fill [string repeat " " [expr {$col + 1}]]
            set S(row,$S(cur_row)) [string replace $row 0 $col $fill]
        }
        2 {
            set S(row,$S(cur_row)) [string repeat " " $cols]
        }
    }
}

proc ::vt::_osc {SVar ch} {
    upvar 1 $SVar S
    set b [scan $ch %c]
    if {$b == 0x07 || $b == 0x9c} {
        set S(state) ground
        return
    }
    if {$b == 0x1b} {
        set S(state) esc_st
        return
    }
    # Otherwise consume the OSC string body.  We don't yet capture it.
}

# Catch-all for ESC \ string-terminator sequences that close DCS/OSC
# without us having to track which one we're in.
proc ::vt::_esc_st {SVar ch} {
    upvar 1 $SVar S
    # Whether or not it's the expected '\' we just go back to ground;
    # any stray byte after ESC inside a string terminates it.
    set S(state) ground
}

proc ::vt::_charset {SVar ch} {
    upvar 1 $SVar S
    # Only G0 (ESC ( ...) controls our line-drawing flag.  We treat
    # designate-and-activate semantics: ESC ( 0 turns line drawing on,
    # ESC ( B turns it off.  Real terminals also have G1/G2/G3 plus
    # SI/SO/LS2/LS3 to switch active charset; tests don't need that
    # fidelity.
    switch -- $ch {
        "0" { set S(line_drawing) 1 }
        "B" -
        "A" -
        "U" { set S(line_drawing) 0 }
    }
    set S(state) ground
}
