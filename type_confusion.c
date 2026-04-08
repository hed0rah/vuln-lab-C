#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* type confusion: two struct types share the same initial layout (type tag +
   name), but diverge after that. one has a function pointer; the other has
   attacker-controlled bytes at the same offset. if the runtime checks only
   the type tag and then casts without further verification, an attacker can
   substitute a data_blob for a handler and redirect the function call. */

#define TYPE_HANDLER  1
#define TYPE_DATA     2

struct handler {
    int  type;
    char name[16];
    void (*process)(const char *); /* function pointer at offset 20 */
};

struct data_blob {
    int  type;
    char name[16];
    char payload[8];               /* attacker-controlled bytes at same offset */
};

static void dispatch(void *obj)
{
    int type = *(int *)obj;
    if (type == TYPE_HANDLER) {
        /* safe path */
        struct handler *h = (struct handler *)obj;
        h->process(h->name);
    } else {
        /* bug: falls through without a valid cast;
           if an attacker crafts a data_blob with type=TYPE_HANDLER,
           payload is treated as a function pointer and called */
        printf("unknown type -- no dispatch\n");
    }
}

static void legit_handler(const char *name)
{
    printf("handling: %s\n", name);
}

int main(void)
{
    /* normal use */
    struct handler h = { TYPE_HANDLER, "request", legit_handler };
    dispatch(&h);

    /* type confusion: craft a data_blob that claims to be a handler.
       payload bytes will be interpreted as a function pointer by dispatch. */
    struct data_blob blob;
    blob.type = TYPE_HANDLER;      /* lies about its type */
    strcpy(blob.name, "evil");
    memset(blob.payload, 0x41, sizeof(blob.payload)); /* 0x41414141... as fn ptr */

    printf("\nconfused dispatch (likely crashes or redirects control):\n");
    dispatch(&blob);

    return 0;
}
