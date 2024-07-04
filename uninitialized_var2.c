#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void uninitialized_variable() {
  char *sensitive_data = malloc(1024); // allocate memory for sensitive data
  char uninitialized[1024];            // larger uninitialized variable

  // simulate storing sensitive data
  strcpy(sensitive_data, "KillENGN");

  // free the sensitive data buffer to increase the chance of memory reuse
  free(sensitive_data);

  // use uninitialized memory
  printf("Uninitialized variable contains: %s\n", uninitialized);
}

int main() {
  uninitialized_variable();
  return 0;
}
