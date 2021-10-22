puts "spec: BASIC INSERT"

# This file implements regression tests for DBLite library.
set workingDir [pwd]
set dbliteFileName "$workingDir/.bin/dblite"

# Check if file is executable
set isExecutable [file executable $dbliteFileName]
if {!$isExecutable} {
  puts "error"
}

# Get directory of the test database file
set dbFile "test.db"
set dbFileDirectory "$workingDir/$dbFile"

proc testOutput {description expected actual} {
  set TEST_FAIL_COLOR "\033\[37;41m"
  set TEST_PASS_COLOR "\033\[37;42m"
  set TEST_FAIL_DESC_COLOR "\033\[1;31m"
  set TEST_PASS_DESC_COLOR "\033\[1;32m"
  set RESET_COLOR "\033\[0m"

  if {[string compare $actual $expected] != 0} {
    puts "$TEST_FAIL_COLOR FAIL:$RESET_COLOR $description"
    puts "Description:\n  $description"
    puts "Expected result:\n  $TEST_PASS_DESC_COLOR $expected $RESET_COLOR"
    puts "Received result:\n  $TEST_FAIL_DESC_COLOR $actual $RESET_COLOR"
  } else {
    puts "$TEST_PASS_COLOR PASS:$RESET_COLOR $description"
  }
}

# Basic insert test

# Remove the test database
file delete $dbFileDirectory

set singleInsertDesc "inserts and retrieves a row"
set singleInsertExpected "db > Executed.
db > Executed.
db > (1, foo, a@b.c)
(2, bar, d@e.f)
Executed.
db > "
set singleInsertResult [exec $dbliteFileName $dbFile << "insert 1 foo a@b.c\ninsert 2 bar d@e.f\nselect\n.exit\n"]

puts [testOutput $singleInsertDesc $singleInsertExpected $singleInsertResult]

# Insert long length strings

# Remove the test database
file delete $dbFileDirectory

set longUsername ""
for {set a 0} {$a < 32} {incr a} {
  append longUsername "a"
}

set longEmail ""
for {set a 0} {$a < 255} {incr a} {
  append longEmail "a"
}

set longStringInsertDesc "allows inserting strings that are the maximum length"
set longStringInsertExpected "db > Executed.
db > (1, $longUsername, $longEmail)
Executed.
db > "
set longStringInsertResult [exec $dbliteFileName $dbFile << "insert 1 $longUsername $longEmail\nselect\n.exit\n"]

puts [testOutput $longStringInsertDesc $longStringInsertExpected $longStringInsertResult]

# Print message if string inputs are too long

# Remove the test database
file delete $dbFileDirectory

set longUsername2 ""
for {set a 0} {$a < 33} {incr a} {
  append longUsername2 "a"
}

set longEmail2 ""
for {set a 0} {$a < 256} {incr a} {
  append longEmail2 "a"
}

set longStringInsertErrorDesc "prints error message if strings are too long"
set longStringInsertErrorExpected "db > String is too long.
db > Executed.
db > "
set longStringInsertErrorResult [exec $dbliteFileName $dbFile << "insert 1 $longUsername2 $longEmail2\nselect\n.exit\n"]

puts [testOutput $longStringInsertErrorDesc $longStringInsertErrorExpected $longStringInsertErrorResult]

# Print error if id is negative

# Remove the test database
file delete $dbFileDirectory

set negativeInsertId -1

set negativeInsertDesc "prints an error message if id is negative"
set negativeInsertExpected "db > ID must be positive.
db > Executed.
db > "

set negativeIdInsertResult [exec $dbliteFileName $dbFile << "insert $negativeInsertId foo foo@bar.com\nselect\n.exit\n"]

puts [testOutput $negativeInsertDesc $negativeInsertExpected $negativeIdInsertResult]

# Print error if duplicate index

# Remove the test database
file delete $dbFileDirectory

set duplicateIdInsertDesc "prints an error message if there is a duplicate id"
set duplicateIdInsertExpected "db > Executed.
db > Error: Duplicate key.
db > (1, foo, a@b.c)
Executed.
db > "

set duplicateIdInsertResult [exec $dbliteFileName $dbFile << "insert 1 foo a@b.c\ninsert 1 bar d@e.f\nselect\n.exit\n"]

puts [testOutput $duplicateIdInsertDesc $duplicateIdInsertExpected $duplicateIdInsertResult]

# Bulk insert test

# Remove the test database
file delete $dbFileDirectory

set bulkInsertDesc "prints all rows in a multi-level tree"

set baseCommand ""
for { set a 1} {$a < 16} {incr a} {
  append baseCommand "insert $a user$a a$a@b.com\n"
}
append baseCommand "select\n.exit\n"

set bulkInsertExpected "db > (1, user1, a1@b.com)\n"
for { set a 2} {$a < 16} {incr a} {
  append bulkInsertExpected "($a, user$a, a$a@b.com)\n"
}
append bulkInsertExpected "Executed.\ndb > "

set result [exec $dbliteFileName $dbFile << $baseCommand]
set resultList [split $result "\n"]
set resultSubset [lrange $resultList 15 end]

set bulkInsertResult [join $resultSubset "\n"]

puts [testOutput $bulkInsertDesc $bulkInsertExpected $bulkInsertResult]

# Full table test

# Remove the test database
file delete $dbFileDirectory

set baseCommand ""

set fullTableInsertDesc "prints error message when table is full"
set fullTableInsertExpected "db > Error: Table full."

for { set a 0} {$a < 1401} {incr a} {
  append baseCommand "insert $a foo a@b.c\n"
}

append baseCommand ".exit\n"

set result [exec $dbliteFileName $dbFile << $baseCommand]
set resultList [split $result "\n"]

set fullTableInsertResult [lindex $resultList 1400]

puts [testOutput $fullTableInsertDesc $fullTableInsertExpected $fullTableInsertResult]
