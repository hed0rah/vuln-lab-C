#include <stdio.h>
#include <string.h>

void off_by_one(char *input) {
  char buffer[10]; // buffer of size 10

  // Off-by-one error: The condition allows input of length exactly 10,
  // which can overwrite the memory just beyond the buffer.
  if (strlen(input) <= 10) {
    strcpy(buffer, input); // vulnerable to off-by-one error when input length
                           // is exactly 10
    printf("Buffer content: %s\n", buffer);
  } else {
    printf("Input too long\n");
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <input>\n", argv[0]);
    return 1;
  }
  off_by_one(argv[1]);
  return 0;
}
