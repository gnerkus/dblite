# Step 12

## History
- Printing the table's contents after inserting 15 elements returns a result similar to this: `(2, user1, a1@b.com)`
  - Only one result is returned
  - The id does not match the content

This occurs because the data selected is from the root, an internal node, which shouldn't contain data. The data is a left over from when the root was a leaf node.

## Plan
- On `table_start` (called by `execute_select`), find the leftmost cell node via `table_find`, which finds nodes based on their type. First it finds the root node (via `internal_node_find`) then it finds the leftmost leaf node.
- Move to the next leaf node once all the cells in the leftmost leaf node has been exhausted.
- 

### References

## TERMS
