#include <stdio.h>
#include <string.h>

void vulnerable_function(char *input) {
  char buffer[16];
  strcpy(buffer, input); // vulnerable to buffer overflow
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <input>\n", argv[0]);
    return 1;
  }
  vulnerable_function(argv[1]);
  printf("Input processed\n");
  return 0;
}
