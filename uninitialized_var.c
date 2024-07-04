#include <stdio.h>
#include <string.h>

void uninitialized_variable() {
  char sensitive_data[100];
  char uninitialized[100]; // uninitialized variable

  // simulate storing sensitive data
  strcpy(sensitive_data, "KillENGN");

  printf("Uninitialized variable contains: %s\n", uninitialized);
}

int main() {
  uninitialized_variable();
  return 0;
}
