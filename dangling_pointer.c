#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* dangling pointer: a stored pointer is used after the object it referenced
   has been freed. the allocator may give the same address to a new allocation,
   so the old pointer now silently aliases the new object.

   reads through the dangling pointer return data from the new allocation;
   writes corrupt it. no crash -- this is what makes it dangerous. */

struct record {
    int  id;
    char name[16];
};

int main(void)
{
    struct record *a = malloc(sizeof *a);
    a->id = 100;
    strcpy(a->name, "original");

    struct record *stale = a;   /* stale will outlive a */

    free(a);                    /* a is released back to the allocator */

    /* allocate the same size -- allocator commonly returns the same address */
    struct record *b = malloc(sizeof *b);
    b->id = 999;
    strcpy(b->name, "newrecord");

    printf("b->id:     %d\n",    b->id);
    printf("stale->id: %d  (was 100; now shows b's data -- same address)\n",
           stale->id);         /* dangling: stale == b on most allocators */

    stale->id = 0;             /* silently corrupts b */
    printf("b->id after write through stale: %d\n", b->id);

    free(b);
    return 0;
}
