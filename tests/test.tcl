# This file implements regression tests for DBLite library.
set workingDir [pwd]
set dbliteFileName "$workingDir/.bin/dblite"

# Check if file is executable
set isExecutable [file executable $dbliteFileName]
if {!$isExecutable} {
  puts "error"
}

set result [exec $dbliteFileName << "insert 1 foo a@b.c\ninsert 2 bar d@e.f\nselect\n.exit\n"]
puts $result
