#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* uninitialized variable (heap reuse): sensitive data is written into a heap
   allocation that is then freed. a subsequent stack buffer is never initialized.
   because the allocator may reuse the same pages, the stack buffer can contain
   the sensitive heap data verbatim.

   this models a broader class of info leaks where freed memory content is
   accessible via a different variable before the pages are zeroed or reused. */
void uninitialized_variable(void)
{
    char *sensitive = malloc(64);
    if (!sensitive) return;

    strcpy(sensitive, "api_key=deadbeefcafe");
    free(sensitive);                /* freed but not zeroed */

    char leak[64];                  /* uninitialized; may land on same pages */
    printf("leak contains: %s\n", leak);
}

int main(void)
{
    uninitialized_variable();
    return 0;
}
