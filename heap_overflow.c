#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* heap overflow: a fixed-size buffer is allocated on the heap, but strcpy
   writes however much the caller provides with no length check.
   overflows corrupt adjacent heap metadata or neighboring allocations,
   which can lead to arbitrary code execution via heap exploitation. */
void heap_overflow(char *input)
{
    char *buffer = malloc(16);
    if (!buffer) return;

    strcpy(buffer, input); /* no bounds check -- overflows when strlen(input) >= 16 */
    printf("buffer: %s\n", buffer);

    free(buffer);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <input>\n", argv[0]);
        return 1;
    }
    heap_overflow(argv[1]);
    return 0;
}
