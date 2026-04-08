#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* double free: calling free() twice on the same pointer corrupts the
   allocator's internal free-list. depending on the allocator, this can cause
   crashes, heap corruption, or controllable write-what-where conditions.

   glibc detects some double-free patterns and aborts with a diagnostic;
   other allocators may not. */

int main(void)
{
    char *ptr = malloc(32);
    if (!ptr) return 1;

    strcpy(ptr, "testdata");
    printf("allocated: %s\n", ptr);

    free(ptr);
    free(ptr);  /* double free -- undefined behavior */

    return 0;
}
