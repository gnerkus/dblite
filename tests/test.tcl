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

set baseCommand ""
set insertCommand "insert 1 foo a@b.c\n"

# Bulk insert test
set description2 "prints error message when table is full"
set expected2 "db > Error: Table full."

for { set a 0} {$a < 1401} {incr a} {
  append baseCommand $insertCommand
}

append baseCommand ".exit\n"

set result2 [exec $dbliteFileName << $baseCommand]
set result2_list [split $result2 "\n"]

set lastOutput [lindex $result2_list 1400]

if {[string compare $lastOutput $expected2] != 0} {
  puts "Test failure"
  puts "Description:\n  $description2"
  puts "Expected result:\n  $expected2"
  puts "Received result:\n  $lastOutput"
} else {
  puts "Test passed"
}
