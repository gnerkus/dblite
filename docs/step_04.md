# Step 04

The plan:
1. Write spec to test single insert
2. Write spec to insert bulk insert to maximum table row count
3. Write spec to insert long strings
4. Throw an error if the string inputs are too long

[3] C strings are supposed to end with a null character and space should be allocated for that.

[4] If the string being read by `scanf` is larger than the buffer it's reading into, it will cause buffer overflow and `scanf` will start writing to unexpected places.

## TERMS
**`llength`** [TCL]

*Definition*
```tcl
llength list
```
Count the number of elements in a list

*Examples*
```tcl
llength {a b c d e}
# 5
```
```tcl
llength {}
# 0
```
```tcl
llength {a b {c d} e}
```

**`lindex`** [TCL]

*Definition*
```tcl
lindex list ?index...?
```
Retrieve an element from a list

*Examples*
```tcl
lindex {a b c}
# a b c
```
```tcl
lindex {a b c} 0
# a
```

**`string index`** [TCL]

*Definition*
```tcl
string index string charIndex
```
Returns the charIndex 'th character of the string argument. A charIndex of 0 corresponds to the first character of the string.

*Examples*
```tcl
string index abcde 3
# d
```

```tcl
string index abcde 10
# ""
```

**`uplevel`** [TCL]

*Definition*
```tcl
uplevel ?level? arg ?arg ...?
```
If level is an integer then it gives a distance (up the procedure calling stack) to move before executing the command. If level consists of # followed by a number then the number gives an absolute level number. If level is omitted then it defaults to 1. Level cannot be defaulted if the first command argument starts with a digit or #.

For example, suppose that procedure a was invoked from top-level, and that it called b, and that b called c. Suppose that c invokes the uplevel command. If level is 1 or #2 or omitted, then the command will be executed in the variable context of b. If level is 2 or #1 then the command will be executed in the variable context of a. If level is 3 or #0 then the command will be executed at top-level (only global variables will be visible).

**`file`** [TCL]

*Definition*
```tcl
file option name ?arg arg ...?
```

Manipulate file names and attributes

*Examples*
```tcl
file executable $filePath
# 1
```
Check if a file located in filePath is executable.

**`exec`** [TCL]

*Definition*
```tcl
exec ?switches? arg ?arg ...? ?&?
```

Invoke subprocesses

*Examples*
```tcl
exec $filePath << "arg0 arg1"
```
"arg0 arg1" is passed to the first command as its standard input; filePath is the path to an executable file

```tcl
exec $filePath <@ $openFile
```
openFile is passed to the first command as its standard input;$openFile is a file that has been opened by `open`.

**`string compare`** [TCL]

*Definition*
```tcl
string compare ?-nocase? ?-length int? string1 string2
```
Perform a character-by-character comparison of strings string1 and string2. Returns -1, 0, or 1, depending on whether string1 is lexicographically less than, equal to, or greater than string2

*Examples*
```tcl
string compare "hello" "hello"
# 0
```

**`for loop`** [TCL]

*Definition*
```tcl
for {set i 0} {$i < count} {incr i} {}
```

**`strok`**

*Definition*
```c
char *strtok( char *str1, const char *str2 );
```
Returns a pointer to the next token in str1, where str2 contains the delimiters that determine the token. strtok() returns NULL if no token is found.

All calls after the first call should have str1 be NULL.

*Examples*
```c
char str[] = "now # is the time for all # good men to come to the # aid of their country";
  char delims[] = "#";
  char *result = NULL;
  result = strtok( str, delims );
  while( result != NULL ) {
      printf( "result is \"%s\"\n", result );
      result = strtok( NULL, delims );
  }
// "now "
// " is the time for all "
// ...
```

**`atoi`**

*Definition*
```c
int atoi(const char *str);
```
Converts a string to an integer.

*Examples*
```c
atoi("512");
// 512
```
