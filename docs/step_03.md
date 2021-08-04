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

## TERMs
`sscanf`
*Definition*
```c
int sscanf ( const char * s, const char * format, ...);
```
Read formatted data from string.
Reads data from `s` and stored them according to parameter `format` into the locations given by the additional arguments.
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

