#include <stdio.h>

void null_pointer_dereference() {
  char *ptr = NULL;
  printf("Dereferencing null pointer: %c\n", *ptr); // dereference null pointer
}

int main() {
  null_pointer_dereference();
  return 0;
}
