#include <string.h>
#include <stdio.h>

int main(void) {
  char source[] = "once upon a time", dest[4];
  memcpy(dest, source, sizeof dest);
  for(size_t n = 0; n < sizeof dest; ++n)
    putchar(dest[n]);
}