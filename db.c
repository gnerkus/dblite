// required for stdin
#include <stdio.h>

// required for EXIT_FAILURE (1), EXIT_SUCCESS (0), malloc, free & exit
// exit(1) can signal successfull termination on VMS
#include <stdlib.h>

// required for the `strcmp` method
#include <string.h>

#include <errno.h>

// required for `open`, O_RDWR, O_CREAT
#include <fcntl.h>

// required for S_IRUSR, S_IWUSR
#include <sys/stat.h>

// required for `close`, `lseek`
#include <unistd.h>

// required for `bool`
#include <stdbool.h>

/* Forward declarations of structures */
typedef struct InputBuffer InputBuffer;

/*
InputBuffer represents the an input object for the DBLite repl
InputBuffer is defined as a struct type
char* is used for the buffer because it represents a string of input
*/
struct InputBuffer
{
  char *buffer;         // the input buffer (input from an IO device, in this case, the shell)
  size_t buffer_length; // size_t is an unsigned long (at least 32 bits)
  ssize_t input_length; // ssize_t is a long
};

// MetaCommandResult defines all possible results of running a meta command
// If a meta command is recognized, use meta_command_success
// meta commands in SQlite include .exit, .help e.t.c commands that are run in the shell and start with '.'
typedef enum
{
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

// PrepareResult defines all possible outcomes of running a prepared statement
// If a prepares statement is run successfully, use prepare_success
typedef enum
{
  PREPARE_SUCCESS,
  PREPARE_STRING_TOO_LONG,
  PREPARE_NEGATIVE_ID,
  PREPARE_UNRECOGNIZED_STATEMENT,
  PREPARE_SYNTAX_ERROR
} PrepareResult;

// StatementType defines all possible SQL statements allowed by this compiler
typedef enum
{
  STATEMENT_INSERT,
  STATEMENT_SELECT,
  STATEMENT_UPDATE,
  STATEMENT_DELETE
} StatementType;

// ExecuteResult defines all possible results after executing an SQL statement
typedef enum
{
  EXECUTE_SUCCESS,
  EXECUTE_TABLE_FULL,
  EXECUTE_DUPLICATE_KEY,
} ExecuteResult;

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

// Row defines the arguments for an insert operation
// username is an array of characters that has COLUMN_USERNAME_SIZE allocated
// in this case, we allocate only 32 characters
// email is an array of characters; a string
typedef struct
{
  uint32_t id;
  // '+1' allocates an extra position for the null character
  // we use a char array here because we want to edit it
  char username[COLUMN_USERNAME_SIZE + 1];
  char email[COLUMN_EMAIL_SIZE + 1];
} Row;

// Statement defines a statement to be processed by the compiler
typedef struct
{
  StatementType type;
  Row row_to_insert; // only used by insert statement
} Statement;

// returns the size of an attribute of a struct
// e.g size_of_attribute(Row, id); -> uint32_t <some_value>
#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

// serialized representation of a row in the table
/*
A serialized row looks like this:
COLUMN    SIZE(bytes)   OFFSET
id        4             0
username  32            4
email     255           36
total     291
*/
const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

void print_row(Row *row)
{
  printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

// store the row in a memory location
void serialize_row(Row *source, void *destination)
{
  // destination is a memory address (pointer)
  // get the address of the id, copy into the destination from position ID_OFFSET (usually the beginning of destination)
  memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);

  // get the address of the username, copy into the destination from position USERNAME_OFFSET
  memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
  memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void *source, Row *destination)
{
  // get all the content from memory block position ID_OFFSET, of size ID_SIZE, and copy into destination->id
  memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
  memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

const uint32_t PAGE_SIZE = 4096;
const uint32_t TABLE_MAX_PAGES = 100;

/*
The Pager accesses the page cache and the file.
The Table object makes requests for pages through the pager

Data is saved to a file via multiple page-sized memory blocks.
Reads are made via pages.

file_length -> the size of each page
*/
typedef struct
{
  int file_descriptor;
  uint32_t file_length;
  uint32_t num_pages;
  void *pages[TABLE_MAX_PAGES];
} Pager;

/*
The Table replaces the B-Tree in the real SQLite implementation.
This is temporary.
*/
typedef struct
{
  Pager *pager;
  uint32_t root_page_num;
} Table;

// a cursor represents a location in a table
typedef struct
{
  Table *table;
  uint32_t page_num; // pointer to the current page
  uint32_t cell_num; // pointer to the current cell (row)
  bool end_of_table; // Indicates a position one post the last element
} Cursor;

// 'constructor' for InputBuffer
// struct properties are accessed via ->
InputBuffer *new_input_buffer()
{
  // allocate memory of the size of the InputBuffer then cast the result to a
  // pointer to the InputBuffer
  InputBuffer *input_buffer = (InputBuffer *)malloc(sizeof(InputBuffer));
  input_buffer->buffer = NULL;     // initial buffer is empty
  input_buffer->buffer_length = 0; // limit on size of buffer
  input_buffer->input_length = 0;

  return input_buffer;
}

// <TREE DEFINITIONS>
// Node types for the B-Tree implementation of a table
typedef enum
{
  NODE_INTERNAL,
  NODE_LEAF
} NodeType;

/*
Common Node Header Layout

This is defined for all nodes; leaves (pages) and internal
*/
// how many bits do we need to store the node type size?
const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
// we store the node type at the beginning of the memory block
const uint32_t NODE_TYPE_OFFSET = 0;
// if this node is the root node
const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
// the 'is root' check is stored right after the node type
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_SIZE + IS_ROOT_OFFSET;
// size of the header
const uint8_t COMMON_NODE_HEADER_SIZE = NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

/*
Leaf Node format

A leaf node is a page so it needs to keep track of how many rows (cells)
it contains
*/
// how many cells are in a page
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
// add header to point to the next leaf node
const uint32_t LEAF_NODE_NEXT_LEAF_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NEXT_LEAF_OFFSET =
    LEAF_NODE_NUM_CELLS_OFFSET + LEAF_NODE_NUM_CELLS_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE + LEAF_NODE_NEXT_LEAF_SIZE;

/*
Leaf Node Body Layout
*/
const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0;
// The size of a row (cell)
const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET = LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
// a page is a leaf node; it has multiple cells
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS = LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;

const uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) / 2;
const uint32_t LEAF_NODE_LEFT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) - LEAF_NODE_RIGHT_SPLIT_COUNT;

/*
 * Internal Node Header Layout
 *
 * 1. byte 0: node_type [common] 8 bits
 * 2. byte 1: is_root [common] 8 bits
 * 3. byte 2 - 5: parent pointer [common] 32 bits
 * 4. byte 6 - 9: num keys [internal] 32 bits
 * 5. byte 10 - 13: right child pointer [internal] 32 bits
 * 6. byte 14 - 17: child pointer 0 [internal] 32 bits
 * 7. byte 18 - 21: key 0 [internal] 32 bits
 * ...
 * byte 4086 - 4089: child pointer 509 [internal] <always 1 more child pointer than keys>
 * byte 4090 - 4093: key 509
 */
// the keys in an internal node are references to pages (leaf nodes)
const uint32_t INTERNAL_NODE_NUM_KEYS_SIZE = sizeof(uint32_t);
// the number of keys start where the common header ends
const uint32_t INTERNAL_NODE_NUM_KEYS_OFFSET = COMMON_NODE_HEADER_SIZE;
// Reference to the page number of the rightmost child of the internal node
const uint32_t INTERNAL_NODE_RIGHT_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_RIGHT_CHILD_OFFSET =
    INTERNAL_NODE_NUM_KEYS_OFFSET + INTERNAL_NODE_NUM_KEYS_SIZE;
const uint32_t INTERNAL_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE + INTERNAL_NODE_NUM_KEYS_SIZE + INTERNAL_NODE_RIGHT_CHILD_SIZE;

/*
* Internal Node Body Layout

* The body is an array of cells where each cell contains a child pointer and a key.
* Every key should be the maximum key contained in the child to its left.
*/
const uint32_t INTERNAL_NODE_CELL_KEY_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CELL_SIZE =
    INTERNAL_NODE_CHILD_SIZE + INTERNAL_NODE_CELL_KEY_SIZE;

/**
 * LEAF NODE FUNCTIONS
 */
// returns a pointer to the start of the leaf node's cells
uint32_t *leaf_node_num_cells(void *node)
{
  return node + LEAF_NODE_NUM_CELLS_OFFSET;
}

// returns a pointer to the start of a cell's key based on
// the cell's number
void *leaf_node_cell(void *node, uint32_t cell_num)
{
  return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

// returns a pointer to the start of a cell's (row) key
uint32_t *leaf_node_key(void *node, uint32_t cell_num)
{
  return leaf_node_cell(node, cell_num);
}

// returns a pointer to the start of a cell's value
void *leaf_node_value(void *node, uint32_t cell_num)
{
  return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

// fetch the next leaf for a leaf node
uint32_t *leaf_node_next_leaf(void *node)
{
  return node + LEAF_NODE_NEXT_LEAF_OFFSET;
}

/**
 * INTERNAL NODE FUNCTIONS
 */
// return a pointer to the number of keys for a node
uint32_t *internal_node_num_keys(void *node)
{
  return node + INTERNAL_NODE_NUM_KEYS_OFFSET;
}

// return a pointer to the right child of an internal node
uint32_t *internal_node_right_child(void *node)
{
  return node + INTERNAL_NODE_RIGHT_CHILD_OFFSET;
}

// return a pointer to a child at cell_num of an internal node
uint32_t *internal_node_cell(void *node, uint32_t cell_num)
{
  return node + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE;
}

/**
 * The key of an internal node immediately follows the cell (child)
 * This method is a getter and setter
*/
uint32_t *internal_node_key(void *node, uint32_t key_num)
{
  return (void *)internal_node_cell(node, key_num) + INTERNAL_NODE_CHILD_SIZE;
}

/**
 * Find a child node of a internal node via its child_num ref
*/
uint32_t *internal_node_child(void *node, uint32_t child_num)
{
  uint32_t num_keys = *internal_node_num_keys(node);
  if (child_num > num_keys)
  {
    printf("Tried to access child_num %d > num_keys %d\n", child_num, num_keys);
    exit(EXIT_FAILURE);
  }
  else if (child_num == num_keys)
  {
    return internal_node_right_child(node);
  }
  else
  {
    return internal_node_cell(node, child_num);
  }
}

NodeType get_node_type(void *node)
{
  uint8_t value = *((uint8_t *)(node + NODE_TYPE_OFFSET));
  return (NodeType)value;
}

/**
 * get the last key of a node
 *
 * For internal nodes:
 * 1. Get the key number for the last key by subtracting 1 from num of keys
 * 2. Get the cell at the key number from 1
 * 3. Shift forward by cell size
 */
uint32_t get_node_max_key(void *node)
{
  switch (get_node_type(node))
  {
  case NODE_INTERNAL:
    return *internal_node_key(node, *internal_node_num_keys(node) - 1);
  case NODE_LEAF:
    return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
  }
}

void set_node_type(void *node, NodeType type)
{
  uint8_t value = type;
  /**
   * store the sum of the node and NODE_TYPE_OFFSET in a uint8_t
   * if the value is greater than a uint8_t, it will be truncated to a uint8_t
   *
   * Since the node type is constrained to a uint8_t size, then the result
   * of this truncation will be the node type
   */
  *((uint8_t *)(node + NODE_TYPE_OFFSET)) = value;
}

void set_node_root(void *node, bool is_root)
{
  uint8_t value = is_root;
  *((uint8_t *)(node + IS_ROOT_OFFSET)) = value;
}

void initialize_leaf_node(void *node)
{
  set_node_type(node, NODE_LEAF);
  set_node_root(node, false);
  *leaf_node_num_cells(node) = 0;
  *leaf_node_next_leaf(node) = 0; // 0 represents no sibling
}

void initialize_internal_node(void *node)
{
  set_node_type(node, NODE_INTERNAL);
  set_node_root(node, false);
  *internal_node_num_keys(node) = 0;
}

// </TREE DEFINITIONS>

// display a prompt requesting input
void print_prompt() { printf("db > "); }

void read_input(InputBuffer *input_buffer)
{
  ssize_t bytes_read =
      getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

  /*
  &(input_buffer->buffer)

  a pointer to the variable we use to point to the buffer containing the read line.
  If it set to NULL it is mallocatted by getline and should thus be freed by the user, even if the command fails.

  Use & to retrieve the address of a variable. the address of a variable is a pointer
  input_buffer points to the buffer; it's a pointer
  */

  /*
 getline returns a ssize_t, the size of the buffer read from input
 */

  if (bytes_read <= 0)
  {
    printf("Error reading input\n");
    exit(EXIT_FAILURE);
  }

  // Ignore trailing newline
  input_buffer->input_length = bytes_read - 1;
  input_buffer->buffer[bytes_read - 1] = 0;
}

void close_input_buffer(InputBuffer *input_buffer)
{
  free(input_buffer->buffer);
  free(input_buffer);
}

/*
Write the content of a page into memory
*/
void pager_flush(Pager *pager, uint32_t page_num)
{
  if (pager->pages[page_num] == NULL)
  {
    printf("Tried to flush null page\n");
    exit(EXIT_FAILURE);
  }

  // page_num = 1 && PAGE_SIZE = 4096
  // results in the pointer moving to the beginning of the second page
  off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);

  if (offset == -1)
  {
    printf("Error seeking: %d\n", errno);
    exit(EXIT_FAILURE);
  }

  // write the content of a page, at <size> size, into the file
  // the file is identified by the descriptor (0 for stdin, 1 for stdout)
  ssize_t bytes_written = write(pager->file_descriptor, pager->pages[page_num], PAGE_SIZE);

  if (bytes_written == -1)
  {
    printf("Error writing: %d\n", errno);
    exit(EXIT_FAILURE);
  }
}

/*
db_close();

flushes the page cache to disk
closes the database file
frees the memory for the Pager and Table data structures

*/
void db_close(Table *table)
{
  Pager *pager = table->pager;

  for (uint32_t i = 0; i < pager->num_pages; i++)
  {
    if (pager->pages[i] == NULL)
    {
      continue;
    }
    pager_flush(pager, i);
    free(pager->pages[i]);
    pager->pages[i] = NULL;
  }

  int result = close(pager->file_descriptor);
  if (result == -1)
  {
    printf("Error closing db file.\n");
    exit(EXIT_FAILURE);
  }
  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
  {
    void *page = pager->pages[i];
    if (page)
    {
      free(page);
      pager->pages[i] = NULL;
    }
  }
  free(pager);
  free(table);
}

void print_constants()
{
  printf("ROW_SIZE: %d\n", ROW_SIZE);
  printf("COMMON_NODE_HEADER_SIZE: %d\n", COMMON_NODE_HEADER_SIZE);
  printf("LEAF_NODE_HEADER_SIZE: %d\n", LEAF_NODE_HEADER_SIZE);
  printf("LEAF_NODE_CELL_SIZE: %d\n", LEAF_NODE_CELL_SIZE);
  printf("LEAF_NODE_SPACE_FOR_CELLS: %d\n", LEAF_NODE_SPACE_FOR_CELLS);
  printf("LEAF_NODE_MAX_CELLS: %d\n", LEAF_NODE_MAX_CELLS);
}

// tokenize the insert statement using strtok
// then validate input tokens before performing an insert operation
// check for input length and throw error if too long
PrepareResult prepare_insert(InputBuffer *input_buffer, Statement *statement)
{
  statement->type = STATEMENT_INSERT;

  char *keyword = strtok(input_buffer->buffer, " ");
  char *id_string = strtok(NULL, " ");
  char *username = strtok(NULL, " ");
  char *email = strtok(NULL, " ");

  if (id_string == NULL || username == NULL || email == NULL)
  {
    return PREPARE_SYNTAX_ERROR;
  }

  int id = atoi(id_string);
  if (id < 0)
  {
    return PREPARE_NEGATIVE_ID;
  }

  if (strlen(username) > COLUMN_USERNAME_SIZE)
  {
    return PREPARE_STRING_TOO_LONG;
  }
  if (strlen(email) > COLUMN_EMAIL_SIZE)
  {
    return PREPARE_STRING_TOO_LONG;
  }

  statement->row_to_insert.id = id;
  strcpy(statement->row_to_insert.username, username);
  strcpy(statement->row_to_insert.email, email);

  return PREPARE_SUCCESS;
}

// Set the statement type based on the content of the input buffer
// &(statement->row_to_insert.id) stores the value of the digit read into the address
PrepareResult prepare_statement(InputBuffer *input_buffer, Statement *statement)
{
  if (strncmp(input_buffer->buffer, "insert", 6) == 0)
  {
    return prepare_insert(input_buffer, statement);
  }

  if (strncmp(input_buffer->buffer, "select", 6) == 0)
  {
    statement->type = STATEMENT_SELECT;
    return PREPARE_SUCCESS;
  }

  if (strncmp(input_buffer->buffer, "update", 6) == 0)
  {
    statement->type = STATEMENT_UPDATE;
    return PREPARE_SUCCESS;
  }

  if (strncmp(input_buffer->buffer, "delete", 6) == 0)
  {
    statement->type = STATEMENT_DELETE;
    return PREPARE_SUCCESS;
  }

  return PREPARE_UNRECOGNIZED_STATEMENT;
}

void *get_page(Pager *pager, uint32_t page_num)
{
  if (page_num > TABLE_MAX_PAGES)
  {
    printf("Tried to fetch page number out of bounds. %d > %d\n", page_num, TABLE_MAX_PAGES);
    exit(EXIT_FAILURE);
  }

  if (pager->pages[page_num] == NULL)
  {
    // Cache miss. Allocate memory and load from file.
    void *page = malloc(PAGE_SIZE);
    uint32_t num_pages = pager->file_length / PAGE_SIZE;

    // We might save a partial page at the end of the file
    if (pager->file_length % PAGE_SIZE)
    {
      num_pages += 1;
    }

    if (page_num <= num_pages)
    {
      lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
      ssize_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
      if (bytes_read == -1)
      {
        printf("Error reading file: %d\n", errno);
        exit(EXIT_FAILURE);
      }
    }

    pager->pages[page_num] = page;

    if (page_num >= pager->num_pages)
    {
      pager->num_pages = page_num + 1;
    }
  }
  return pager->pages[page_num];
}

/**
 * Returns a cursor pointing to a page and row on the table.
 * It will return one of three results:
 *
  - the position of the key,
  - the position of another key that we’ll need to move if we want to insert the new key, or
  - the position one past the last key

 *
 * table - The table
 * key - The identifying key for the data object (row)
 * page_num - The page where the data is to be found
 *
*/
Cursor *leaf_node_find(Table *table, uint32_t page_num, uint32_t key)
{
  void *node = get_page(table->pager, page_num);
  // number of cells in the node (page)
  uint32_t num_cells = *leaf_node_num_cells(node);

  Cursor *cursor = malloc(sizeof(Cursor));
  cursor->table = table;
  cursor->page_num = page_num;

  // Binary search for the cell (row) that contains the key
  uint32_t min_index = 0;
  uint32_t one_past_max_index = num_cells;
  while (one_past_max_index != min_index)
  {
    uint32_t mid_index = (min_index + one_past_max_index) / 2;
    uint32_t key_at_index = *leaf_node_key(node, mid_index);
    // when key is found
    if (key == key_at_index)
    {
      cursor->cell_num = mid_index;
      return cursor;
    }
    // if key cannot be found in right sub-array
    if (key < key_at_index)
    {
      one_past_max_index = mid_index;
    }
    else
    {
      // if this is always called until the loop stops, then
      // the cell_num will be one_past_max_index
      min_index = mid_index + 1;
    }
  }

  // min_index is the index of the cell that we'll need to move
  cursor->cell_num = min_index;
  return cursor;
}

/**
 * Recursive function to find a cursor pointing to the key
 * to insert the data.
 *
 * 1. Find which child will contain the key
 * 2. Call the internal_node_find() on the child
 */
Cursor *internal_node_find(Table *table, uint32_t page_num, uint32_t key)
{
  void* node = get_page(table->pager, page_num);

  uint32_t child_index = internal_node_find_child(node, key);
  uint32_t child_num = *internal_node_child(node, child_index); 
  
  void *child = get_page(table->pager, child_num);
  switch (get_node_type(child))
  {
  case NODE_LEAF:
    // find the cell to insert the data into
    return leaf_node_find(table, child_num, key);
  case NODE_INTERNAL:
    // recursive call to find the internal node
    return internal_node_find(table, child_num, key);
  }
}

/**
 * Returns a cursor pointing to a position in the table
 * cursor points to either an internal node or leaf node
 *
 * table - The table
 * key - The identifying key for the data object (row); key
 *       could be an id.
 */
Cursor *table_find(Table *table, uint32_t key)
{
  uint32_t root_page_num = table->root_page_num;
  void *root_node = get_page(table->pager, root_page_num);

  if (get_node_type(root_node) == NODE_LEAF)
  {
    return leaf_node_find(table, root_page_num, key);
  }
  else
  {
    return internal_node_find(table, root_page_num, key);
  }
}

// cursor pointing to the start of the table
Cursor *table_start(Table *table)
{
  // use table_find to get the root node
  Cursor *cursor = table_find(table, 0);

  void *node = get_page(table->pager, cursor->page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);
  cursor->end_of_table = (num_cells == 0);

  return cursor;
}

// figure out where to read/write in memory for a row
// the cursor contains a pointer to the current row
void *cursor_value(Cursor *cursor)
{
  uint32_t page_num = cursor->page_num;

  void *page = get_page(cursor->table->pager, page_num);
  return leaf_node_value(page, cursor->cell_num);
}

// move cursor to the next row
void cursor_advance(Cursor *cursor)
{
  uint32_t page_num = cursor->page_num;
  void *node = get_page(cursor->table->pager, page_num);

  cursor->cell_num += 1;
  // When we reach the end of a leaf node
  if (cursor->cell_num >= (*leaf_node_num_cells(node)))
  {
    /* Advance to next leaf node */
    uint32_t next_page_num = *leaf_node_next_leaf(node);
    if (next_page_num == 0)
    {
      /* This was rightmost leaf */
      cursor->end_of_table = true;
    }
    else
    {
      cursor->page_num = next_page_num;
      cursor->cell_num = 0;
    }
  }
}

/**
 * Pages are not recycled so new pages are at the end of the table
 *
 */
uint32_t get_unused_page_num(Pager *pager)
{
  return pager->num_pages;
}

void create_new_root(Table *table, uint32_t right_child_page_num)
{
  /*
    Handle splitting the root.

    1. Old root copied to new page, becomes left child.
    2. Address of right child passed in.
    3. Re-initialize root page to contain the new root node.
    4. New root node points to two children.
  */

  void *root = get_page(table->pager, table->root_page_num);
  void *right_child = get_page(table->pager, right_child_page_num);

  // get an unallocated page to be used as the left child
  uint32_t left_child_page_num = get_unused_page_num(table->pager);
  void *left_child = get_page(table->pager, left_child_page_num);

  /* Left child has data copied from old root */
  memcpy(left_child, root, PAGE_SIZE);
  // left child is no longer the root
  set_node_root(left_child, false);

  /* Root node is a new internal node with one key and two children */
  initialize_internal_node(root);
  set_node_root(root, true);
  *internal_node_num_keys(root) = 1;
  *internal_node_child(root, 0) = left_child_page_num;
  uint32_t left_child_max_key = get_node_max_key(left_child);
  *internal_node_key(root, 0) = left_child_max_key;
  *internal_node_right_child(root) = right_child_page_num;

  // Point both children to the parent
  *node_parent(left_child) = table->root_page_num;
  *node_parent(right_child) = table->root_page_num;
}

bool is_node_root(void *node)
{
  // the first 8 bits of a node define the node's type
  // + the is_root_offset, we obtain whether the node is root or not
  uint8_t value = *((uint8_t *)(node + IS_ROOT_OFFSET));
  return (bool)value;
}

/**
 * In order to get a reference to the parent,
 * we need to start recording in each node a pointer to its parent node.
 */
uint32_t *node_parent(void *node) { return node + PARENT_POINTER_OFFSET; }

uint32_t internal_node_find_child(void *parent_node, uint32_t key)
{
  uint32_t num_keys = *internal_node_num_keys(parent_node);

  /* Binary search */
  uint32_t min_index = 0;
  uint32_t max_index = num_keys; /* there is one more child than key */

  /**
   * Binary search to find the key
   *
   * The key to insert must be less in value than the rightmost child's key
   * and greater than the leftmost child's key.
   *
   * min_index holds the value of the position to write the key
   */
  while (min_index != max_index)
  {
    uint32_t index = (min_index + max_index) / 2;
    uint32_t key_to_right = *internal_node_key(parent_node, index);
    if (key_to_right >= key)
    {
      max_index = index;
    }
    else
    {
      min_index = index + 1;
    }
  }

  return min_index;
}

/**
 * Example:
 * 
 * Let's say this is the current state of the db:
 * Internal: *, 5, *
 * Left child: 1, 5
 * 
 * Say we add a new cell, 3, and the maximum number of cells is 2.
 * The db state will become:
 * Internal: *, 5, *
 * Left child: 1, 3
 * Right child: 5
 * (the left child was split)
 * 
 * We now want the left key in the parent to point to the left child
 * The left key should be the max of the left child, 3
 * So this method will:
 * i. Find the child at the key 5 (left child)
 * ii. Change the key that points to the left child in the parent to 3
 * (so we change the 5 to 3)
 * 
 * The table now looks like this:
 * Internal: *, 3, *
 * Left child: 1, 3
 * Right child: 5
*/
void update_internal_node_key(void *parent_node, uint32_t old_key, uint32_t new_key)
{
  // find the child at the old_key position
  uint32_t old_child_index = internal_node_find_child(parent_node, old_key);
  *internal_node_key(parent_node, old_child_index) = new_key;
}

/**
 * To split the content of the original page between two pages:
 *
 * 1. Create / fetch the new page
 * 2. Find the middle number
 * 3. For each cell in the old page, choose whether to move it to the
 * new_page or keep in the old page, based on its index (key)
 * if the index is more than a middle number, move to new, if lower, leave in old
 * 4. Get the destination cell for each index from 3 above
 * 5. Write the cells to memory:
 * 5.1. if the index is for the cell to be written, write the data to memory
 * 5.2. if the index is for any of the existing cells (from old_node), copy
 * over to the new destination (new_node / old_node)
 */
void leaf_node_split_and_insert(Cursor *cursor, uint32_t key, Row *value)
{
  // old_node is the page that's full; new_node is the page we want to split with
  void *old_node = get_page(cursor->table->pager, cursor->page_num);
  uint32_t old_max = get_node_max_key(old_node);
  uint32_t new_page_num = get_unused_page_num(cursor->table->pager);
  void *new_node = get_page(cursor->table->pager, new_page_num);
  initialize_leaf_node(new_node);

  *node_parent(new_node) = *node_parent(old_node);
  /**
   * Update the sibling pointers
   */
  // new leaf's sibling is the old leaf's sibling
  *leaf_node_next_leaf(new_node) = *leaf_node_next_leaf(old_node);
  // old leaf's sibling becomes the new leaf itself
  *leaf_node_next_leaf(old_node) = new_page_num;

  /*
    All existing keys plus new key should be divided
    evenly between old (left) and new (right) nodes.
    Starting from the right, move each key to correct position.
  */
  for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--)
  {
    void *destination_node;
    if (i >= LEAF_NODE_LEFT_SPLIT_COUNT)
    {
      destination_node = new_node;
    }
    else
    {
      destination_node = old_node;
    }
    // index_within_node determines where to write the cell within either old or new
    // the values fall between 0 and LEAF_NODE_LEFT_SPLIT_COUNT
    // For example, for LEAF_NODE_MAX_CELLS of 21, index_within_node = 0 - 11
    // So if i is 12, it will be written to index 1 of new_node
    // if i is 1, it will be written to index 1 of old_node
    uint32_t index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;
    void *destination = leaf_node_cell(destination_node, index_within_node);

    if (i == cursor->cell_num)
    {
      serialize_row(value, leaf_node_value(destination_node, index_within_node));
      *leaf_node_key(destination_node, index_within_node) = key;
    }
    else if (i > cursor->cell_num)
    {
      memcpy(destination, leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
    }
    else
    {
      // if i is the key for a cell that was written before the last, copy to its
      // new destination
      memcpy(destination, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
    }
  }

  /* Update cell count on both leaf nodes */
  *(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
  *(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

  if (is_node_root(old_node))
  {
    // if the old_node was a root then we need to create a new root after the split
    // both the old_node and new_node will then be connected to this new root
    return create_new_root(cursor->table, new_page_num);
  }
  else
  {
    printf("Need to implement updating parent after split\n");
    exit(EXIT_FAILURE);
    /**
     * 1. get the parent page [DONE]
     * 2. get the updated max key in the old node [DONE]
     * 3. update the max key for old node in the internal node
     * with the updated max key
     * 4. get the max key of the new node
     * 5. add the max key of the new node to the internal node
     *
     */
    uint32_t parent_page_num = *node_parent(old_node);
    void *parent_page = get_page(cursor->table->pager, parent_page_num);

    uint32_t new_max = get_node_max_key(old_node);

    update_internal_node_key(parent_page, old_max, new_max);
    internal_node_insert(cursor->table, parent_page_num, new_page_num);
  }
}

/*
Inserting a row into the database

The row is inserted as a cell into the leaf node
*/
void leaf_node_insert(Cursor *cursor, uint32_t key, Row *value)
{
  // current page
  void *node = get_page(cursor->table->pager, cursor->page_num);

  uint32_t num_cells = *leaf_node_num_cells(node);
  if (num_cells >= LEAF_NODE_MAX_CELLS)
  {
    // Node full so we split
    leaf_node_split_and_insert(cursor, key, value);
    return;
  }

  if (cursor->cell_num < num_cells)
  {
    // Make room for new cell
    for (uint32_t i = num_cells; i > cursor->cell_num; i--)
    {
      // e.g copy content from [2] to [3]
      // if cell num is 1, copy from [1] to [2] then stop (last copy)
      // new content will be written at [1]
      memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1), LEAF_NODE_CELL_SIZE);
    }
  }

  // increase the number of cells in the page (node)
  *(leaf_node_num_cells(node)) += 1;
  *(leaf_node_key(node, cursor->cell_num)) = key;
  serialize_row(value, leaf_node_value(node, cursor->cell_num));
}

ExecuteResult execute_insert(Statement *statement, Table *table)
{
  void *node = get_page(table->pager, table->root_page_num);
  uint32_t num_cells = (*leaf_node_num_cells(node));

  Row *row_to_insert = &(statement->row_to_insert);
  uint32_t key_to_insert = row_to_insert->id;
  // insert data into a place in the table
  Cursor *cursor = table_find(table, key_to_insert);

  // if the current cell, pointed by the cursor, is not
  // at the end of the table
  if (cursor->cell_num < num_cells)
  {
    // get the key of the current cell
    uint32_t key_at_index = *leaf_node_key(node, cursor->cell_num);
    if (key_at_index == key_to_insert)
    {
      return EXECUTE_DUPLICATE_KEY;
    }
  }

  // insert the row's id as the key to the cell
  leaf_node_insert(cursor, row_to_insert->id, row_to_insert);

  free(cursor);

  return EXIT_SUCCESS;
}

ExecuteResult execute_select(Statement *statement, Table *table)
{
  Cursor *cursor = table_start(table);

  Row row;
  while (!(cursor->end_of_table))
  {
    deserialize_row(cursor_value(cursor), &row);
    print_row(&row);
    cursor_advance(cursor);
  }

  free(cursor);

  return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement *statement, Table *table)
{
  switch (statement->type)
  {
  case (STATEMENT_INSERT):
    return execute_insert(statement, table);
    break;
  case (STATEMENT_SELECT):
    return execute_select(statement, table);
    break;
  case (STATEMENT_UPDATE):
    return EXECUTE_SUCCESS;
  case (STATEMENT_DELETE):
    return EXECUTE_SUCCESS;
    break;
  default:
    return EXECUTE_SUCCESS;
  }
}

/*
The pager communicates directly with memory
and sends the data to the table

O_RDWR -> Open for reading and writing.
O_CREAT -> Create file if it does not exist.
S_IWUSR -> User write permission bit macro (owner permission)
S_IRUSR -> User read permission bit macro (owner permission)
*/
Pager *pager_open(const char *filename)
{
  // open a file for reading or writing O_RDWR
  // if it doesn't exist, create it O_CREAT
  // allow reading S_IRUSR and writing S_IWUSR permissions
  int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

  if (fd == -1)
  {
    printf("Unable to open file\n");
    exit(EXIT_FAILURE);
  }

  // Move the read/write file offset to the beginning of the DB file
  off_t file_length = lseek(fd, 0, SEEK_END);

  Pager *pager = malloc(sizeof(Pager));
  pager->file_descriptor = fd;
  pager->file_length = file_length; // size of the db file
  pager->num_pages = (file_length / PAGE_SIZE);

  // DB file must have an exact number of pages
  if (file_length % PAGE_SIZE != 0)
  {
    printf("Db file is not a whole number of pages. Corrupt file.\n");
    exit(EXIT_FAILURE);
  }

  // initialize pages in the pager
  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
  {
    pager->pages[i] = NULL;
  }

  return pager;
}

/*
db_open():

1. Opens a database file
2. Initializing a pager data structure
3. Initializing a table data structure

The pager is the go between the memory and the table

filename -> The file name for the DB
*/
Table *db_open(const char *filename)
{
  Pager *pager = pager_open(filename);

  Table *table = malloc(sizeof(Table)); // (size_t)808UL (unsigned long)
  table->pager = pager;
  // the root page is indexed 0 (first) when the db is first opened
  table->root_page_num = 0;

  if (pager->num_pages == 0)
  {
    // New database file. Initialize page 0 as leaf node
    void *root_node = get_page(pager, 0);
    initialize_leaf_node(root_node);
    // The first node in the table is the root
    set_node_root(root_node, true);
  }

  return table;
}

// print indentation in output
void indent(uint32_t level)
{
  for (uint32_t i = 0; i < level; i++)
  {
    printf("  ");
  }
}

/**
 * Recursive method to print tree
 *
 * 1. Start at root of tree (may be root of entire db or a specific internal node)
 * 2. If leaf, print size of leaf. Print each key contained in leaf at a higher indentation
 * level.
 * 3. If internal, print size of internal node. For each child, run print_tree on child
 * at a higher indentation level.
 * 4. If internal, finally, run print_tree on right child.
 */
void print_tree(Pager *pager, uint32_t page_num, uint32_t indentation_level)
{
  void *node = get_page(pager, page_num);
  uint32_t num_keys, child;

  switch (get_node_type(node))
  {
  case (NODE_LEAF):
    num_keys = *leaf_node_num_cells(node);
    indent(indentation_level);
    printf("- leaf (size %d)\n", num_keys);
    for (uint32_t i = 0; i < num_keys; i++)
    {
      indent(indentation_level + 1);
      printf("- %d\n", *leaf_node_key(node, i));
    }
    break;

  case (NODE_INTERNAL):
    num_keys = *internal_node_num_keys(node);
    indent(indentation_level);
    printf("- internal (size %d)\n", num_keys);
    for (uint32_t i = 0; i < num_keys; i++)
    {
      child = *internal_node_child(node, i);
      print_tree(pager, child, indentation_level + 1);

      indent(indentation_level + 1);
      printf("- key %d\n", *internal_node_key(node, i));
    }
    child = *internal_node_right_child(node);
    print_tree(pager, child, indentation_level + 1);
    break;
  }
}

// Check if the input buffer holds a meta command
MetaCommandResult do_meta_command(InputBuffer *input_buffer, Table *table)
{
  if (strcmp(input_buffer->buffer, ".exit") == 0)
  {
    db_close(table);
    exit(EXIT_SUCCESS);
  }
  else if (strcmp(input_buffer->buffer, ".btree") == 0)
  {
    printf("Tree:\n");
    print_tree(table->pager, 0, 0);
    return META_COMMAND_SUCCESS;
  }
  else if (strcmp(input_buffer->buffer, ".help") == 0)
  {
    printf(".exit: Exits the REPL\n");
    return META_COMMAND_SUCCESS;
  }
  else if (strcmp(input_buffer->buffer, ".constants") == 0)
  {
    printf("Constants:\n");
    print_constants();
    return META_COMMAND_SUCCESS;
  }
  else
  {
    return META_COMMAND_UNRECOGNIZED_COMMAND;
  }
}

// main function will have an infinite loop that prints the prompt,
// gets a line of input, then processes that line of input:
int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("Must supply a database filename.\n");
    exit(EXIT_FAILURE);
  }

  char *filename = argv[1];
  Table *table = db_open(filename);

  InputBuffer *input_buffer = new_input_buffer(); // initialize input buffer
  for (;;)
  {
    print_prompt();
    read_input(input_buffer); // input_buffer is passed by reference

    // if the input begins with ".", we process it as a meta command (.help, .exit e.t.c)
    if (input_buffer->buffer[0] == '.')
    {
      switch (do_meta_command(input_buffer, table))
      {
      case (META_COMMAND_SUCCESS):
        // request input again
        continue;
      case (META_COMMAND_UNRECOGNIZED_COMMAND):
        printf("Unrecognized command '%s'\n", input_buffer->buffer);
        // request input again
        continue;
      }
    }

    // if the input does not begin with ".", we process is as a statement
    Statement statement;
    switch (prepare_statement(input_buffer, &statement))
    {
    case (PREPARE_SUCCESS):
      break;
    case (PREPARE_NEGATIVE_ID):
      printf("ID must be positive.\n");
      continue;
    case (PREPARE_STRING_TOO_LONG):
      printf("String is too long.\n");
      continue;
    case (PREPARE_SYNTAX_ERROR):
      printf("Syntax error. Could not parse statement.\n");
      continue;
    case (PREPARE_UNRECOGNIZED_STATEMENT):
      printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);
      // request input again
      continue;
    }

    switch (execute_statement(&statement, table))
    {
    case (EXECUTE_SUCCESS):
      printf("Executed.\n");
      break;
    case (EXECUTE_TABLE_FULL):
      printf("Error: Table full.\n");
      break;
    case (EXECUTE_DUPLICATE_KEY):
      printf("Error: Duplicate key.\n");
    default:
      break;
    }
  }
}
