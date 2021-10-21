# Step 12

## History
- Printing the table's contents after inserting 15 elements returns a result similar to this: `(2, user1, a1@b.com)`
  - Only one result is returned
  - The id does not match the content

This occurs because the data selected is from the root, an internal node, which shouldn't contain data. The data is a left over from when the root was a leaf node.

## Plan

### References

## TERMS
