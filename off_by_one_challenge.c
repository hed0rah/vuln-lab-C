#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* off-by-one (CTF-style): same <= bug as stack_off_by_one, but here the
   extra byte lands on a volatile guard variable. if the off-by-one write
   changes the guard from its initial value, the program calls win().

   depending on stack layout and compiler, the null terminator from a
   10-byte input can flip the low byte of `target`.

   try: ./off_by_one_challenge AAAAAAAAAA  */

void win(void)
{
    printf("target overwritten -- you win.\n");
    system("/bin/sh");
}

void vulnerable_function(char *input)
{
    char buffer[10];
    volatile int target = 0x41414141;

    if (strlen(input) <= 10) {
        strcpy(buffer, input); /* off-by-one when strlen == 10 */
        printf("buffer: %s\n", buffer);
    } else {
        printf("input too long\n");
    }

    if (target != 0x41414141) {
        printf("target overwritten: 0x%x\n", target);
        win();
    } else {
        printf("target intact: 0x%x\n", target);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <input>\n", argv[0]);
        return 1;
    }
    vulnerable_function(argv[1]);
    return 0;
}
