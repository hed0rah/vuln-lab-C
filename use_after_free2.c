#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void use_after_free() {
  char *ptr = (char *)malloc(10); // allocate memory
  if (ptr == NULL) {
    printf("Memory allocation failed.\n");
    return;
  }

  strcpy(ptr, "KillENGN"); // copy data into allocated memory
  printf("Allocated memory contains: %s\n", ptr);

  free(ptr); // free the allocated memory
  printf("Freed memory contains: %s\n",
         ptr); // attempt to access freed memory (use after free)

  // allocate memory again and show that the same address may be reused
  char *new_ptr = (char *)malloc(10);
  if (new_ptr == NULL) {
    printf("Memory allocation failed.\n");
    return;
  }
  strcpy(new_ptr, "newdata");
  printf("Newly allocated memory contains: %s\n", new_ptr);

  // access the old pointer after new allocation
  printf("After reallocation, old pointer contains: %s\n", ptr);

  free(new_ptr); // free the new allocation
}

int main() {
  use_after_free();
  return 0;
}
