#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void use_after_free() {
  char *ptr = (char *)malloc(10);
  strcpy(ptr, "test");
  printf("Allocated memory contains: %s\n", ptr);
  free(ptr);                                  // freeing memory
  printf("Freed memory contains: %s\n", ptr); // use after free
}

int main() {
  use_after_free();
  return 0;
}
