# Step 03
Database limitations:
-- supports two operations
- insert a row
- print all rows
-- reside only in memory (no persistence to disk)
-- support a single, hard-coded table

Hard-coded table will store users via `id`, `username` and `email`.

Insert statements will look like:
```bash
insert 1 ify foo@bar.com
```

The data read in will be copied to a data structure representing the table.
SQLite uses a B-tree for fast lookups, inserts and deletes. We’ll start with something simpler. Like a B-tree, it will group rows into pages, but instead of arranging those pages as a tree it will arrange them as an array.

The plan:

- Store rows in blocks of memory called pages
- Each page stores as many rows as it can fit
- Rows are serialized into a compact representation with each page
- Pages are only allocated as needed
- Keep a fixed-size array of pointers to pages

## TERMs
`sscanf`
*Definition*
```c
int sscanf ( const char * s, const char * format, ...);
```
Read formatted data from string.
Reads data from `s` and stored them according to parameter `format` into the locations given by the additional arguments `...`.
On success, returns the number of items in the argument list successfully filled.

```c
char sentence []="Rudolph is 12 years old";
char str [20];
int i;

sscanf (sentence,"%s %*s %d",str,&i);
printf ("%s -> %d\n",str,i); // Rudolph -> 12
```
puts "Rudolph" in the first %s
puts "is" in the next %s
puts 12 in the %d

`stuff`
*Definition*
```c

```
stuff
```c

```

