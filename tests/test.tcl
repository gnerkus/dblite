# This file implements regression tests for DBLite library.
set workingDir [pwd]
set dbliteFileName "$workingDir/.bin/dblite"

# Check if file is executable
set isExecutable [file executable $dbliteFileName]
if {!$isExecutable} {
  puts "error"
}

# Basic insert test
set description "inserts and retrieves a row"
set expected "db > Executed.
db > Executed.
db > (1, foo, a@b.c)
(2, bar, d@e.f)
Executed.
db > "
set result [exec $dbliteFileName << "insert 1 foo a@b.c\ninsert 2 bar d@e.f\nselect\n.exit\n"]

if {[string compare $result $expected] != 0} {
  puts "Test failure"
  puts "Description:\n  $description"
  puts "Expected result:\n  $expected"
  puts "Received result:\n  $result"
} else {
  puts "Test passed"
}
