#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* signedness confusion: the length parameter is signed, so a negative value
   passes the "too large" guard (negative < 64 is true). but memcpy takes
   size_t, so the negative value wraps to a huge unsigned count, reading far
   past the source buffer and writing past the destination.

   try: ./int_to_buffer_overflow -1    */
void copy_data(int length, char *src)
{
    char buf[64];

    if (length > 64) {              /* signed comparison: -1 passes this check */
        printf("too long\n");
        return;
    }

    memcpy(buf, src, length);       /* cast to size_t: -1 becomes ~0ULL */
    printf("copied %d bytes\n", length);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <length>\n", argv[0]);
        return 1;
    }

    char src[64] = "source data";
    int length = atoi(argv[1]);
    copy_data(length, src);
    return 0;
}
