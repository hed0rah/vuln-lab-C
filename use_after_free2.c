#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* use-after-free (reuse): sensitive data is written into a heap allocation,
   which is then freed. a new allocation of the same size commonly receives the
   same address from the allocator, and the old pointer still points there.

   reading through the stale pointer returns the new object's data; writing
   through it silently corrupts the new object. this variant focuses on showing
   that the allocator reuses the freed address. */

int main(void)
{
    char *ptr = malloc(32);
    if (!ptr) return 1;

    strcpy(ptr, "secret_token=abcd1234");
    printf("original: %s  (at %p)\n", ptr, (void *)ptr);

    free(ptr);  /* freed but ptr still holds the address */

    /* allocate the same size -- allocator commonly hands back the same block */
    char *new_ptr = malloc(32);
    if (!new_ptr) return 1;

    strcpy(new_ptr, "replaced_data");
    printf("new alloc: %s  (at %p)\n", new_ptr, (void *)new_ptr);

    /* stale pointer reads whatever is at the old address now */
    printf("stale ptr: %s  (at %p -- same address, different data)\n",
           ptr, (void *)ptr);

    free(new_ptr);
    return 0;
}
