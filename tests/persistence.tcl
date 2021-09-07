puts "spec: PERSISTENCE"

# This file implements persistence tests for DBLite library.
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

set singleInsertDesc "keeps data after closing the connection"
set singleInsertInitialExpected "db > Executed.
db > "
set singleInsertFinalExpected "db > (1, foo, a@b.c)
Executed.
db > "

set singleInsertInitialResult [exec $dbliteFileName $dbFile << "insert 1 foo a@b.c\n.exit\n"]
puts [testOutput $singleInsertDesc $singleInsertInitialExpected $singleInsertInitialResult]

set singleInsertFinalResult [exec $dbliteFileName $dbFile << "select\n.exit\n"]
puts [testOutput $singleInsertDesc $singleInsertFinalExpected $singleInsertFinalResult]
