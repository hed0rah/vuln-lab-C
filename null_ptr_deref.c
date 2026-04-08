#include <stdio.h>

/* null pointer dereference: dereferencing a null pointer is undefined behavior.
   on most systems the kernel maps no page at address 0, so the access triggers
   a segfault. if an attacker can map page zero (older kernels, embedded, or
   mmap_min_addr = 0), they control the data the program reads. */

int main(void)
{
    char *ptr = NULL;
    printf("dereferencing null: %c\n", *ptr);  /* segfaults */
    return 0;
}
