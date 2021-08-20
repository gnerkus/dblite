# This file implements regression tests for DBLite library.
exec echo ".help" | dblite

proc dblite {args} {
  # If there are two args and the first one is not an option i.e -stuff...
  if {[llength $args]>=2 && [string index [lindex $args 0] 0]!="-"} {
    # pass the arguments to the dblite command
    puts $dbliteExec $args
  }
}

# dblite
