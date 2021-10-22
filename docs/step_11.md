# Step 11

## History:
An error is thrown when attemping to input the 15th item in a table. This occurs
because the `PAGE_SIZE` for `4096` resolves `LEAF_NODE_MAX_CELLS` to `14` (each cell resolves to `292` in size).

## The plan:
- Implement searching an internal node to fix a bug when inputing the 15th item in a table.
- 

### References

## TERMS
**`lrange`** [TCL]

*Definition*
```tcl
lrange list first last
```
Return one or more adjacent elements from a list

*Examples*
```tcl
lrange {a b c d e} 0 1
# a b
```
```tcl
lrange {a b c d e} 1 end-1
# b c d
```

**`join`** [TCL]

*Definition*
```tcl
join list ?joinString?
```
Create a string by joining together list elements

*Examples*
```tcl
set data {1 2 3 4 5}
join $data ", "
# 1, 2, 3, 4, 5
```
flatten a list by a single level:
```tcl
set data {1 {2 3} 4 {5 {6 7} 8}}
join $data
# 1 2 3 4 5 {6 7} 8
```
