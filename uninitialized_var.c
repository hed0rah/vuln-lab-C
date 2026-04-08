#include <stdio.h>
#include <string.h>

/* uninitialized variable (stack): sensitive data is stored in one stack buffer,
   then a second buffer is printed without being initialized. depending on stack
   layout and compiler, the second buffer may contain bytes from the first,
   leaking data to the caller. */
void uninitialized_variable(void)
{
    char sensitive[64];
    char leak[64];          /* never initialized */

    strcpy(sensitive, "password=hunter2");

    /* leak may contain bytes from sensitive if they overlap on the stack */
    printf("leak contains: %s\n", leak);
}

int main(void)
{
    uninitialized_variable();
    return 0;
}
