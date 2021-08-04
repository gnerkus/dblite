# Step 02
DBLite is a clone of sqlite. The front-end of sqlite is a SQL compiler that parses a string and outputs an internal representation called bytecode.
This bytecode is passed to a virtual machine, which executes it.

We'll build a simple compiler and virtual machine in this step.

## TERMSs
`strncmp`
*Definition*
```c
int strncmp ( const char * str1, const char * str2, size_t num );
```
Compares up to num characters of the C string str1 to those of the C string str2.
This function starts comparing the first character of each string. If they are equal to each other, it continues with the following pairs until the characters differ, until a terminating null-character is reached, or until num characters match in both strings, whichever happens first.
```c
strncmp("R2D2", "R2xx", 2) == 0; // true
```

`free`
*Definition*
```c
void free(void* ptr);
```
Deallocates the space previously allocated by `malloc()`, `calloc()` or `realloc()`.
If `ptr` is a null pointer, the function does nothing.
```c
int *p1 = malloc(10*sizeof *p1);
free(p1); // every allocated pointer must be freed
```

**Pointer**
-- A pointer in C is an address, which is a numeric value.
-- Accepted operations: ++, --, +, -
```c
int var1 = NULL;
&var1; // address of var1

int *ip; // pointer to an integer
ip = &var1; // store address of var1 in pointer variable 
char *ch; // pointer to a character
```
https://www.eskimo.com/~scs/cclass/notes/sx10b.html

