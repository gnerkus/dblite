# Step 07

SQLite represents both tables and indexes with a B-Tree.

SQLite uses B-tree to store indexes and a B+ tree to store tables.

|                              |B-tree         |B+ tree             |
|------------------------------|---------------|--------------------|
|Used to store                 |Indexes        |Tables              |
|Internal nodes store keys     |Yes            |Yes                 |
|Internal nodes store values   |Yes            |No                  |
|Number of children per node   |Less           |More                |
|Internal nodes vs. leaf nodes |Same structure |Different structure |

For the table implementation, we'll begin with the B+ tree.

## B+ Tree

Nodes with children are called “internal” nodes. Internal nodes and leaf nodes are structured differently:

|For an order-m tree|Internal Node                |Leaf Node          |
|-------------------|-----------------------------|-------------------|
|Stores             |Keys and pointers to children|keys and values    |
|Number of keys     |up to m - 1                  |as many as will fit|
|Number of pointers |number of keys + 1           |none               |
|Number of values   |none                         |number of keys     |
|Key purpose        |used for routing             |paired with value  |
|Store values?      |No                           |Yes                |

## B-Tree
A B-tree is a tree data structure that keeps data sorted and allows searches, insertions, and deletions in logarithmic amortized time. Unlike self-balancing binary search trees, it is optimized for systems that read and write large blocks of data. It is most commonly used in database and file systems.

- B-Tree nodes have more than 2 children
- B-tree nodes may contain more than just a single element.

### The set formulation of the B-tree rules: 

Every B-tree depends on a positive constant integer called MINIMUM, which is used to determine how many elements are held in a single node.

- **Rule 1**: The root can have as few as one element (or even no elements if it also has no children); every other node has at least MINIMUM element.

- **Rule 2**: The maximum number of elements in a node is twice the value of MINIMUM.
- **Rule 3**: The elements of each B-tree node are stored in a partially filled array, sorted from the smallest element (at index 0) to the largest element (at the final used position of the array).
- **Rule 4**: The number of subtrees below a nonleaf node is always one more than the number of elements in the node.
Subtree 0, subtree 1, ...
- **Rule 5**: For any nonleaf node:
An element at index i is greater than all the elements in subtree number i of the node, and
An element at index i is less than all the elements in subtree number i + 1 of the node.
- **Rule 6**: Every leaf in a B-tree has the same depth. Thus it ensures that a B-tree avoids  the problem of a unbalanced tree.

- Nodes must have between MINIMUM and 2 * MINIMUM elements.
- Elements in a node are sorted
- If a node has 2 elements, it must have 3 subtrees.
- 

### Why is a B-Tree good for a database
- Searching for a particular value is fast (logarithmic time)
- Inserting / deleting a value you’ve already found is fast (constant-ish time to rebalance)
- Traversing a range of values is fast (unlike a hash map)

### References
[Lecture Notes: B-Trees](https://www.cpp.edu/~ftang/courses/CS241/notes/b-tree.htm)
[How Indexing Works in SQL](https://stackoverflow.com/questions/1108/how-does-database-indexing-work)
[Book Index / DB Index Analogy](https://stackoverflow.com/a/43572540/2259144)

The plan:



## TERMS

