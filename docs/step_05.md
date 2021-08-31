# Step 05

The plan:
Write the blocks of memory to a file, and read them back into memory the next time the program starts up.

1. Develop an abstraction called the pager.
2. We ask the pager for page number x, and the pager gives us back a block of memory. It first looks in its cache. On a cache miss, it copies data from disk into memory (by reading the database file).

The pager communicates between the table and the device's memory.

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

*Examples*
```c
int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
off_t file_length = lseek(fd, 0, SEEK_END);
```


## ADDITIONAL
-- [System Programming with C](http://www.cs.cmu.edu/~ab/15-123S10/AnnotatedNotes/Lecture24-9AM.pdf)
-- [Reading and Writing Files with C](https://stackoverflow.com/questions/14003466/how-can-i-read-and-write-from-files-using-the-headers-fcntl-h-and-unistd-h)