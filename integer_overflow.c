#include <stdio.h>

void integer_overflow() {
  unsigned int max = 0xFFFFFFFF;
  unsigned int result = max + 1; // causes integer overflow
  printf("Result of overflow: %u\n", result);
}

int main() {
  integer_overflow();
  return 0;
}
