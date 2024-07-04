#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void win() {
  printf("congrats.\n");
  system("/bin/sh");
}

void vulnerable_function(char *input) {
  char buffer[10];                  // buffer of size 10
  volatile int target = 0x41414141; // target value to overwrite

  // off-by-one error: The condition allows input of length exactly 10,
  // which can overwrite the memory just beyond the buffer.
  if (strlen(input) <= 10) {
    strcpy(buffer, input); // vulnerable to off-by-one error when input length
                           // is exactly 10
    printf("Buffer content: %s\n", buffer);
  } else {
    printf("Input too long\n");
  }

  // check if the target value has been modified
  if (target != 0x41414141) {
    printf("Target overwritten: 0x%x\n", target);
    win(); // call the win function if target is overwritten
  } else {
    printf("Target not overwritten: 0x%x\n", target);
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <input>\n", argv[0]);
    return 1;
  }
  vulnerable_function(argv[1]);
  return 0;
}
