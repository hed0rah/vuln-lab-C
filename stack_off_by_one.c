#include <stdio.h>
#include <string.h>

/* off-by-one (stack): the length check uses <= instead of <, allowing a
   string of exactly 10 characters (plus null terminator = 11 bytes) to be
   written into a 10-byte buffer. the extra byte overwrites whatever is
   adjacent on the stack -- typically a saved frame pointer byte, which can
   redirect control flow on return.

   try: ./stack_off_by_one AAAAAAAAAA   (exactly 10 A's)  */

void off_by_one(char *input)
{
    char buffer[10];

    if (strlen(input) <= 10) {
        strcpy(buffer, input); /* writes 11 bytes when strlen == 10 */
        printf("buffer: %s\n", buffer);
    } else {
        printf("input too long\n");
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <input>\n", argv[0]);
        return 1;
    }
    off_by_one(argv[1]);
    return 0;
}
