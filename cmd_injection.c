#include <stdio.h>
#include <stdlib.h>

void command_injection(char *input) {
  char command[256];
  // snprintf used improperly can lead to command injection
  snprintf(command, sizeof(command), "echo %s", input);
  // executing the command, input is not sanitized
  system(command);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <input>\n", argv[0]);
    return 1;
  }
  command_injection(argv[1]);
  return 0;
}
