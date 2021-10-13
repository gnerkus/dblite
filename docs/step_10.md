# Step 10

## The plan:
- Add more nodes to the b-tree by allowing a leaf node to be split
- Define internal nodes

### Splitting algorithm for leaf nodes
> If there is no space on the leaf node, we would split the existing entries residing there and the new one (being inserted) into two equal halves: lower and upper halves. (Keys on the upper half are strictly greater than those on the lower half.) We allocate a new leaf node, and move the upper half into the new node.

From _SQLite Database System: Design and Implementation_



### References

## TERMS
