#include <stdio.h>

int main() {
	int i = 5;
	int* i_ad = &(i);
	char random[50];
	int scanned = sscanf("name 1", "%s %d", random, &i);
  printf("%s %d", random, i);
  
  return 0;
}