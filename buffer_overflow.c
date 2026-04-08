#include <stdio.h>
#include <string.h>

/* stack buffer overflow: strcpy copies argv[1] into a 16-byte stack buffer
   with no length check. inputs longer than 15 bytes overwrite the saved frame
   pointer and return address, allowing control-flow hijacking.

   try: ./buffer_overflow $(python3 -c "print('A'*32)")  */

void vulnerable_function(char *input)
{
    char buffer[16];
    strcpy(buffer, input); /* no bounds check */
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <input>\n", argv[0]);
        return 1;
    }
    vulnerable_function(argv[1]);
    printf("input processed\n");
    return 0;
}
