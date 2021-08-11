// required for stdin
#include <stdio.h>
// required for EXIT_FAILURE (1), EXIT_SUCCESS (0), malloc, free & exit
// exit(1) can signal successfull termination on VMS
#include <stdlib.h>
// required for the `strcmp` method
#include <string.h>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

// InputBuffer represents the an input object for the DBLite repl
// InputBuffer is defined as a struct type
typedef struct {
  char* buffer; // the input buffer (input from an IO device, in this case, the shell)
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

// MetaCommandResult defines all possible results of running a meta command
// If a meta command is recognized, use meta_command_success
// meta commands in SQlite include .exit, .help e.t.c commands that are run in the shell and start with '.'
typedef enum {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

// PrepareResult defines all possible outcomes of running a prepared statement
// If a prepares statement is run successfully, use prepare_success
typedef enum {
  PREPARE_SUCCESS,
  PREPARE_UNRECOGNIZED_STATEMENT,
  PREPARE_SYNTAX_ERROR
} PrepareResult;

// StatementType defines all possible SQL statements allowed by this compiler
typedef enum {
  STATEMENT_INSERT,
  STATEMENT_SELECT,
  STATEMENT_UPDATE,
  STATEMENT_DELETE
} StatementType;

// Row defines the arguments for an insert operation
// username is an array of characters that has COLUMN_USERNAME_SIZE allocated
// email is an array of characters; a string
typedef struct {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE];
  char email[COLUMN_EMAIL_SIZE];
} Row;

// Statement defines a statement to be processed by the compiler
typedef struct {
  StatementType type;
  Row row_to_insert; // only used by insert statement
} Statement;

// returns the size of an attribute of a struct
// e.g size_of_attribute(Row, id); -> uint32_t <some_value>
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

// serialized representation of a row in the table
const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

// 'constructor' for InputBuffer
// struct properties are accessed via ->
InputBuffer* new_input_buffer() {
  InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer)); // allocate memory for the input buffer 'instance'
  input_buffer->buffer = NULL; // initial buffer is empty
  input_buffer->buffer_length = 0; // limit on size of buffer
  input_buffer->input_length = 0;

  return input_buffer;
}
// display a prompt requesting input
void print_prompt() { printf("db > "); }

// stuff
void read_input(InputBuffer* input_buffer) {
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

  if (bytes_read <= 0) {
    printf("Error reading input\n");
    exit(EXIT_FAILURE);
  }

  // Ignore trailing newline
  input_buffer->input_length = bytes_read - 1;
  input_buffer->buffer[bytes_read - 1] = 0;
}

void close_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}

// Check if the input buffer holds a meta command
MetaCommandResult do_meta_command(InputBuffer* input_buffer) {
  if (strcmp(input_buffer->buffer, ".exit") == 0) {
    close_input_buffer(input_buffer);
    exit(EXIT_SUCCESS);
  } else if (strcmp(input_buffer->buffer, ".help") == 0) {
    printf(".exit: Exits the REPL\n");
    return META_COMMAND_SUCCESS;
  } else {
    return META_COMMAND_UNRECOGNIZED_COMMAND;
  }
}

// Set the statement type based on the content of the input buffer
// &(statement->row_to_insert.id) stores the value of the digit read into the address
// sscanf requires the arguments be specified by address
PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement) {
  if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
    statement->type = STATEMENT_INSERT;
    int args_assigned = sscanf(
      input_buffer->buffer,
      "insert %d %s %s",
      &(statement->row_to_insert.id),
      statement->row_to_insert.username,
      statement->row_to_insert.email
    );
    if (args_assigned < 3) {
      return PREPARE_SYNTAX_ERROR;
    }
    return PREPARE_SUCCESS;
  }

  if (strncmp(input_buffer->buffer, "select", 6) == 0) {
    statement->type = STATEMENT_SELECT;
    return PREPARE_SUCCESS;
  }

  if (strncmp(input_buffer->buffer, "update", 6) == 0) {
    statement->type = STATEMENT_UPDATE;
    return PREPARE_SUCCESS;
  }

  if (strncmp(input_buffer->buffer, "delete", 6) == 0) {
    statement->type = STATEMENT_DELETE;
    return PREPARE_SUCCESS;
  }

  return PREPARE_UNRECOGNIZED_STATEMENT;
}

void execute_statement(Statement* statement) {
  switch (statement->type)
  {
  case (STATEMENT_INSERT):
    printf("Insert code WIP\n");
    break;
  case (STATEMENT_SELECT):
    printf("Select code WIP\n");
    break;
  case (STATEMENT_UPDATE):
    printf("Update code WIP\n");
    break;
  case (STATEMENT_DELETE):
    printf("Delete code WIP\n");
    break;
  default:
    break;
  }
}

// main function will have an infinite loop that prints the prompt, gets a line of input, then processes that line of input:
int main(int argc, char* argv[]) {
  InputBuffer* input_buffer = new_input_buffer(); // initialize input buffer
  for (;;)
  {
    print_prompt();
    read_input(input_buffer); // input_buffer is passed by reference

    // if the input begins with ".", we process it as a meta command
    if (input_buffer->buffer[0] == '.') {
      switch (do_meta_command(input_buffer))
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
    switch(prepare_statement(input_buffer, &statement)) {
      case (PREPARE_SUCCESS):
        break;
      case (PREPARE_UNRECOGNIZED_STATEMENT):
        printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);
        // request input again
        continue;
    }

    execute_statement(&statement);
    printf("Executed.\n");
  }
}