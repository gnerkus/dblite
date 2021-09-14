# Step 08

The main plan:
_Change the format of the table from unsorted array of rows to a B-Tree._

The plan:
- Define the layout of a leaf node and support inserting key/value pairs into a single-node tree.
  - Every node will store what type of node it is, whether or not it is the root node, and a pointer to its parent.
  - Leaf nodes will store how many cells they contain
  - The body of a leaf node is an array of cells. Each cell is a key followed by a value (serialized row).
  Each leaf of the tree is a page in the table; it has a pointer to its parent. The parent is used to find the row's sibling pages.
- Every node is going to take up exactly one page, even if not full.



## Why change the table format?
- Current format allows search only by scanning the entire table
- Deletion involves moving several rows to fill the 'hole'
- If implemented as sorted array, search would be faster but insertion would be slow because lots of rows need to move.
- While the tree is larger, insertion, deletion and lookup are faster

|             |unsorted |sorted   |tree                        |
|-------------|---------|---------|----------------------------|
|Pages contain|only data|only data|metadata, primary keys, data|
|Rows per page|more     |more     |fewer                       |
|Insertion    |O(1)     |O(n)     |O(log(n))                   |
|Deletion     |O(n)     |O(n)     |O(log(n))                   |
|Lookup by id |O(n)     |O(log(n))|O(log(n))                   |

### References

## TERMS