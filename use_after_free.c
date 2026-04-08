#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* use-after-free: memory is accessed after it has been freed. the contents
   are no longer valid and may have been overwritten by the allocator's internal
   bookkeeping or by a subsequent allocation. reading stale data or writing
   through a freed pointer leads to corruption, info leaks, or code execution. */

int main(void)
{
    char *ptr = malloc(32);
    if (!ptr) return 1;

    strcpy(ptr, "sensitive");
    printf("before free: %s\n", ptr);

    free(ptr);

    /* accessing freed memory -- undefined behavior */
    printf("after free:  %s\n", ptr);

    return 0;
}
