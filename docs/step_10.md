# Step 10

## The plan:
- Fetch an empty page for use in sharing the data
- Add more nodes to the b-tree by allowing a leaf node to be split between two pages
- Define internal nodes (parent nodes)

### Splitting algorithm for leaf nodes
> If there is no space on the leaf node, we would split the existing entries residing there and the new one (being inserted) into two equal halves: lower and upper halves. (Keys on the upper half are strictly greater than those on the lower half.) We allocate a new leaf node, and move the upper half into the new node.

From _SQLite Database System: Design and Implementation_

### Fetching new pages
For now, we’re assuming that in a database with N pages, page numbers 0 through N-1 are allocated. Therefore we can always allocate page number N for new pages. Eventually after we implement deletion, some pages may become empty and their page numbers unused. To be more efficient, we could re-allocate those free pages.

### Creating a new root node
> Let N be the root node. First allocate two nodes, say L and R. Move lower half of N into L and the upper half into R. Now N is empty. Add 〈L, K,R〉 in N, where K is the max key in L. Page N remains the root. Note that the depth of the tree has increased by one, but the new tree remains height balanced without violating any B+-tree property.

From _SQLite Database System: Design and Implementation_



### References

## TERMS
