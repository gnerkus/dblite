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

# Tree view

# Remove the test database
file delete $dbFileDirectory

set treeViewDesc "displays a tree view of the rows"
set treeViewExpected "db > Executed.
db > Executed.
db > Executed.
db > Tree:
- leaf (size 3)
  - 1
  - 2
  - 3
db > "
set treeViewResult [
  exec $dbliteFileName $dbFile << "insert 3 foo a@b.c\ninsert 1 baa d@e.f\ninsert 2 buz g@h.i\n.btree\n.exit\n"
]

puts [testOutput $treeViewDesc $treeViewExpected $treeViewResult]

# Tree view: B-tree

# Remove the test database
file delete $dbFileDirectory

set treeViewBTreeDesc "allows printing out the structure of a 3-leaf-node btree"
set treeViewBTreeExpected "db > Tree:
- internal (size 1)
  - leaf (size 7)
    - 1
    - 2
    - 3
    - 4
    - 5
    - 6
    - 7
  - key 7
  - leaf (size 7)
    - 8
    - 9
    - 10
    - 11
    - 12
    - 13
    - 14
db > Need to implement searching an internal node "

set baseCommand ""

for { set a 1} {$a < 15} {incr a} {
  append baseCommand "insert $a foo a@b.c\n"
}

append baseCommand ".btree\ninsert 15 foo a@b.c\n.exit\n"

set result [exec $dbliteFileName $dbFile << $baseCommand]

set resultList [split $result "\n"]

set treeViewBTreeResult [lrange $resultList 14 end]

puts [testOutput $treeViewBTreeDesc $treeViewBTreeExpected $treeViewBTreeResult]