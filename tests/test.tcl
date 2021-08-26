# This file implements regression tests for DBLite library.
set workingDir [pwd]
set dbliteFileName "$workingDir/.bin/dblite"

# Check if file is executable
set isExecutable [file executable $dbliteFileName]
if {!$isExecutable} {
  puts "error"
}

proc testOutput {description expected actual} {
  if {[string compare $actual $expected] != 0} {
    puts "Test failure"
    puts "Description:\n  $description"
    puts "Expected result:\n  $expected"
    puts "Received result:\n  $actual"
  } else {
    puts "Test passed"
  }
}

# Basic insert test
set singleInsertDesc "inserts and retrieves a row"
set singleInsertExpected "db > Executed.
db > Executed.
db > (1, foo, a@b.c)
(2, bar, d@e.f)
Executed.
db > "
set singleInsertResult [exec $dbliteFileName << "insert 1 foo a@b.c\ninsert 2 bar d@e.f\nselect\n.exit\n"]

puts [testOutput $singleInsertDesc $singleInsertExpected $singleInsertResult]

# Bulk insert test
set baseCommand ""
set insertCommand "insert 1 foo a@b.c\n"

set bulkInsertDesc "prints error message when table is full"
set bulkInsertExpected "db > Error: Table full."

for { set a 0} {$a < 1401} {incr a} {
  append baseCommand $insertCommand
}

append baseCommand ".exit\n"

set result [exec $dbliteFileName << $baseCommand]
set resultList [split $result "\n"]

set bulkInsertResult [lindex $resultList 1400]

puts [testOutput $bulkInsertDesc $bulkInsertExpected $bulkInsertResult]
