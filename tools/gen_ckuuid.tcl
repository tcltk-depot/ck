#!/usr/bin/env tclsh
# Usage: gen_ckuuid.tcl <source-dir> <output-file>
# Writes a small C header with the build's CK_VERSION_UUID, derived from
# git rev-parse HEAD if available, otherwise from manifest.uuid, otherwise
# a generic placeholder.

set srcdir [lindex $argv 0]
set out    [lindex $argv 1]

set uuid "unknown"
if {[catch {
    set uuid "git-[exec git -C $srcdir rev-parse HEAD]"
}]} {
    set f [file join $srcdir manifest.uuid]
    if {[file readable $f]} {
        set fp [open $f r]
        set uuid [string trim [read $fp]]
        close $fp
    }
}

set fp [open $out w]
puts $fp "#define CK_VERSION_UUID \\"
puts $fp $uuid
close $fp
