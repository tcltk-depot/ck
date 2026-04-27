# Ensure the version in this path is used rather than a system-wide installation
set dir [file normalize [file join [file dirname [file normalize [info script]]] ..]]
#source [file join $dir pkgIndex.tcl]

package require ck
button .b -text "hello, world" -command exit
pack .b
after 1000 {focus .b}
puts "hello ck"
puts stderr "hello ck stderr"
vwait exit
