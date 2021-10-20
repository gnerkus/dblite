# Step 11

## History:
An error is thrown when attemping to input the 15th item in a table. This occurs
because the `PAGE_SIZE` for `4096` resolves `LEAF_NODE_MAX_CELLS` to `14` (because
each cell resolves to `292` in size).

## The plan:
- Implement searching an internal node to fix a bug when inputing
  the 15th item in a table.
- 

### References

## TERMS
