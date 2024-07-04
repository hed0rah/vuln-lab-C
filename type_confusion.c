#include <stdio.h>

void type_confusion() {
  void *ptr = NULL;
  int int_value = 42;
  float float_value = 3.14;

  ptr = &int_value;
  printf("As int: %d\n", *(int *)ptr);

  ptr = &float_value;
  printf("As float: %f\n", *(float *)ptr); // correct type cast

  ptr = &float_value;
  printf("As int: %d\n", *(int *)ptr); // incorrect type cast
}

int main() {
  type_confusion();
  return 0;
}
