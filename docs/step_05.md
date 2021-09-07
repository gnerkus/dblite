# Step 05

The plan:
Write the blocks of memory to a file, and read them back into memory the next time the program starts up.

1. Develop an abstraction called the pager.
2. We ask the pager for page number x, and the pager gives us back a block of memory. It first looks in its cache. On a cache miss, it copies data from disk into memory (by reading the database file).

The pager communicates between the table and the device's memory. The pager writes data from memory to permanent storage.

A crucial point about pages is they have to be atomically read and written to storage, so they have to be contiguous blocks of RAM when in memory and contiguous chunks of storage when written to the persistent store.

## TERMS
`open`
*Definition*
```c
int open(const char *path, int oflag, ... );
```
Open a file; returns a file descriptor as an int.
The file status flags and file access modes of the open file description will be set according to the value of oflag.

*Examples*
```c
int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

if (fd == -1) {
  printf("Unable to open file");
}
```

`lseek`
*Definition*
```c
off_t lseek(int fildes, off_t offset, int whence);
```

Move the read/write file offset. If whence is SEEK_END, the file offset is
set to the size of the file plus offset.

SEEK_SET moves the file pointer position to the beginning of the file.

*Examples*
```c
int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
off_t file_length = lseek(fd, 0, SEEK_END);
```

`uint32_t`
*Definition*
Unsigned integer with width of exactly 32 bits; provided only if the implementation supports the type.

This is similar to the INT(11) in MySQL, which is 4 bytes in size.

When not using WITHOUT ROWID in SQLite, all tables have a rowId column that is 64 bits in size. The largest size for an INTEGER in SQLite is 64 bits.

### C strings
```c
char *s = "Hello world";
```
places "Helo world" in the read-only parts of the memory, making `s` a pointer to that. Any writing operation is illegal.

```c
char s[] = "Hello world";
```

puts the literal string in read-only memory and copies the string to newly allocated memory on the stack. So 
```c
s[0] = 'J'
```
is legal.

A char in C is usually 8 bits in size (1 byte).

### Pointers in C
Arguments cannot be passed by reference in C so pointers are used instead.

As an analogy, a page number in a book's index could be considered a pointer to the corresponding page; dereferencing such a pointer would be done by flipping to the page with the given page number and reading the text found on that page. The actual format and content of a pointer variable is dependent on the underlying computer architecture.

Using pointers significantly improves performance for repetitive operations, like traversing iterable data structures (e.g. strings, lookup tables, control tables and tree structures). In particular, it is often much cheaper in time and space to copy and dereference pointers than it is to copy and access the data to which the pointers point.

Pointers are also used to hold the addresses of entry points for called subroutines in procedural programming and for run-time linking to dynamic link libraries (DLLs). In object-oriented programming, pointers to functions are used for binding methods, often using virtual method tables.

*Uses of pointers*
- passing a large structure without reference would create a copy of said structure (hence wastage of space)
- 

*Void pointers*
Void pointers point to any data type; like `any` in Typescript.

## ADDITIONAL
-- [System Programming with C](http://www.cs.cmu.edu/~ab/15-123S10/AnnotatedNotes/Lecture24-9AM.pdf)
-- [Reading and Writing Files with C](https://stackoverflow.com/questions/14003466/how-can-i-read-and-write-from-files-using-the-headers-fcntl-h-and-unistd-h)