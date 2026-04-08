#include <stdio.h>

/* format string vulnerability: user input is passed directly as the format
   argument to printf. an attacker can supply format specifiers (%x, %s, %n)
   to read stack memory, leak pointers, or write to arbitrary addresses.

   try: ./format_string "%x.%x.%x.%x"
   try: ./format_string "%s"           */

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <input>\n", argv[0]);
        return 1;
    }
    printf(argv[1]); /* user input as format string */
    printf("\n");
    return 0;
}
