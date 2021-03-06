# ERRORS

### Using `strcmp` without including <string.h>
```bash
db.c:72:9: error: implicitly declaring library function 'strcmp' with type 'int (const char *, const char *)'
      [-Werror,-Wimplicit-function-declaration]
    if (strcmp(input_buffer->buffer, ".exit") == 0) {
        ^
db.c:72:9: note: include the header <string.h> or explicitly provide a declaration for 'strcmp'
1 error generated.
```
- Points to line where the first error occurs
- Explains the error
- Defines possible solution
-- `note: include the header <string.h> or explicitly provide a declaration for 'strcmp'`

### Missing import of <stdlib.h>
```bash
db.c:20:45: error: implicitly declaring library function 'malloc' with type 'void *(unsigned long)'
      [-Werror,-Wimplicit-function-declaration]
  InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer)); // allocate memory for the inpu...
                                            ^
db.c:20:45: note: include the header <stdlib.h> or explicitly provide a declaration for 'malloc'
db.c:51:5: error: implicitly declaring library function 'exit' with type 'void (int) __attribute__((noreturn))'
      [-Werror,-Wimplicit-function-declaration]
    exit(EXIT_FAILURE);
    ^
db.c:51:5: note: include the header <stdlib.h> or explicitly provide a declaration for 'exit'
db.c:51:10: error: use of undeclared identifier 'EXIT_FAILURE'
    exit(EXIT_FAILURE);
         ^
db.c:60:5: error: implicit declaration of function 'free' is invalid in C99
      [-Werror,-Wimplicit-function-declaration]
    free(input_buffer->buffer);
    ^
db.c:74:12: error: use of undeclared identifier 'EXIT_SUCCESS'
      exit(EXIT_SUCCESS);
           ^
5 errors generated.
```

### Missing import of <stdin.h>
```bash
db.c:28:23: error: implicitly declaring library function 'printf' with type 'int (const char *, ...)'
      [-Werror,-Wimplicit-function-declaration]
void print_prompt() { printf("db > "); }
                      ^
db.c:28:23: note: include the header <stdio.h> or explicitly provide a declaration for 'printf'
db.c:33:7: error: implicit declaration of function 'getline' is invalid in C99
      [-Werror,-Wimplicit-function-declaration]
      getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
      ^
db.c:33:72: error: use of undeclared identifier 'stdin'
      getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
                                                                       ^
3 errors generated.
```

### Not all paths return
```bash
db.c:103:1: warning: non-void function does not return a value in all control paths [-Wreturn-type]
}
^
1 warning generated.
```
Source of the error:
```c
MetaCommandResult do_meta_command(InputBuffer* input_buffer) {
  if (strcmp(input_buffer->buffer, ".exit") == 0) {
    close_input_buffer(input_buffer);
    exit(EXIT_SUCCESS);
  } else if (strcmp(input_buffer->buffer, ".help") == 0) {
    printf(".exit: Exits the REPL\n");
    //return META_COMMAND_SUCCESS;
  } else {
    return META_COMMAND_UNRECOGNIZED_COMMAND;
  }
}
```
The `else if` does not return a value;

### Missing import of <stdio.h> for putchar
```bash
memcpy_test.c:7:5: error: implicit declaration of function 'putchar' is invalid in C99
      [-Werror,-Wimplicit-function-declaration]
    putchar(dest[n]);
    ^
1 error generated.
```

### Missing import of <string.h> for memcpy
```bash
memcpy_test.c:6:3: error: implicitly declaring library function 'memcpy' with type 'void *(void *, const void *,
      unsigned long)' [-Werror,-Wimplicit-function-declaration]
  memcpy(dest, source, sizeof dest);
  ^
memcpy_test.c:6:3: note: include the header <string.h> or explicitly provide a declaration for 'memcpy'
1 error generated.
```

### Incompatible pointer type
```bash
db.c:319:43: warning: incompatible pointer types passing 'Table **' to parameter of type 'Table *'; remove &
      [-Wincompatible-pointer-types]
    switch (execute_statement(&statement, &table)) {
                                          ^~~~~~
db.c:247:62: note: passing argument to parameter 'table' here
ExecuteResult execute_statement(Statement* statement, Table* table) {
```
*def*
```c
...
ExecuteResult execute_statement(Statement* stmt, Table* table) {
...
```
*cause*
```c
Statement statement;
Table* table = new_table();
execute_statement(&statement, &table);
```