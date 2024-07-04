#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void double_free() {
  char *ptr = (char *)malloc(10);
  strcpy(ptr, "test");
  printf("Allocated memory contains: %s\n", ptr);
  free(ptr);
  free(ptr); // double free
}

int main() {
  double_free();
  return 0;
}
