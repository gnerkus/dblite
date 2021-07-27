# STEP 01
Sqlite starts a read-execute-print loop when you start it from the command line

So the first task is to build a REPL (`db.c`)

## Compiling and running a C program on MacOS
```bash
clang program.c -o program
```
```bash
./program
```

## TERMS
`malloc`
// You can also dynamically allocate contiguous blocks of memory with the
// standard library function malloc, which takes one argument of type size_t
// representing the number of bytes to allocate (usually from the heap, although this
```c
int *my_ptr = malloc(sizeof(*my_ptr) * 20);
  for (xx = 0; xx < 20; xx++) {
    *(my_ptr + xx) = 20 - xx; // my_ptr[xx] = 20-xx
  } // Initialize memory to 20, 19, 18, 17... 2, 1 (as ints)
```

`sizeof`
// sizeof(T) gives you the size of a variable with type T in bytes
// sizeof(obj) yields the size of the expression (variable, literal, etc.).
```c
printf("%zu\n", sizeof(int)); // => 4 (on most machines with 4-byte words)
```

`getline`
/*
getline() reads an entire line from stream, storing the address
of the buffer containing the text into *lineptr.  The buffer is
null-terminated and includes the newline character, if one was found.

returns the size of the buffer read
 */
definition
```c
ssize_t getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream);
```

`strcmp`
/*
Compares two strings (char*). If both are equal, returns 0
*/
```c
strcmp('hello', 'hello'); // 0
```


