puts "spec: META COMMANDS"

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

# Defined constants

set constantsDesc "displays the system's constants"
set constantsExpected "db > Constants:
ROW_SIZE: 293
COMMON_NODE_HEADER_SIZE: 6
LEAF_NODE_HEADER_SIZE: 10
LEAF_NODE_CELL_SIZE: 297
LEAF_NODE_SPACE_FOR_CELLS: 4086
LEAF_NODE_MAX_CELLS: 13
db > "
set constantsResult [exec $dbliteFileName $dbFile << ".constants\n.exit\n"]

puts [testOutput $constantsDesc $constantsExpected $constantsResult]
