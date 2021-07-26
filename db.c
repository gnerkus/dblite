// required for stdin
#include <stdio.h>
// required for EXIT_FAILURE (1) & EXIT_SUCCESS (0)
// exit(1) can signal successfull termination on VMS
#include <stdlib.h>
// required for the `strcmp` method
#include <string.h>

// InputBuffer represents the an input object for the DBLite repl
// InputBuffer is defined as a struct type
typedef struct {
  char* buffer; // the input buffer (input from an IO device, in this case, the shell)
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

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

// main function will have an infinite loop that prints the prompt, gets a line of input, then processes that line of input:
int main(int argc, char* argv[]) {
  InputBuffer* input_buffer = new_input_buffer(); // initialize input buffer
  for (;;)
  {
    print_prompt();
    read_input(input_buffer); // input_buffer is passed by reference

    if (strcmp(input_buffer->buffer, ".exit") == 0) {
      close_input_buffer(input_buffer);
      exit(EXIT_SUCCESS);
    } else if (strcmp(input_buffer->buffer, ".help") == 0) {
      printf(".exit: Exits the REPL\n");
    } else {
      printf("Unrecognized command '%s'.\n", input_buffer->buffer);
    }
  }
}