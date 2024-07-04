#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void vulnerable_function(size_t length) {
  char buffer[10];
  if (length > sizeof(buffer)) {
    printf("Length too large.\n");
    return;
  }

  // overflow could cause the buffer size check to fail
  char *data = (char *)malloc(length);
  if (data == NULL) {
    printf("Memory allocation failed.\n");
    return;
  }

  memset(data, 'A', length);    // fill the allocated memory with 'A's
  memcpy(buffer, data, length); // unsafe copy, potential buffer overflow
  printf("Buffer content: %s\n", buffer);

  free(data);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <length>\n", argv[0]);
    return 1;
  }

  size_t length = atoi(argv[1]);
  vulnerable_function(length);

  return 0;
}
