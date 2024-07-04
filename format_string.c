#include <stdio.h>

void vulnerable_function(char *input) {
  printf(input); // vulnerable to format string exploit
  printf("\n");
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <input>\n", argv[0]);
    return 1;
  }
  vulnerable_function(argv[1]);
  return 0;
}
