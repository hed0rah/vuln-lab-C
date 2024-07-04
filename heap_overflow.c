#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void heap_overflow(char *input) {
  char *buffer = (char *)malloc(16); // allocate a small buffer on the heap
  if (buffer == NULL) {
    printf("Memory allocation failed!\n");
    return;
  }

  // ensure that the input length can indeed cause an overflow
  if (strlen(input) >= 16) {
    printf("Input too long! Potential overflow detected.\n");
  } else {
    strcpy(buffer, input); // copy input to buffer, vulnerable to overflow if
                           // input is too long
    printf("Buffer content: %s\n", buffer);
  }

  free(buffer); // free the allocated memory
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <input>\n", argv[0]);
    return 1;
  }
  heap_overflow(argv[1]);
  return 0;
}
