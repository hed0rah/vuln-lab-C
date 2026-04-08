#include <stdio.h>
#include <stdlib.h>

/* command injection: unsanitized user input is interpolated into a shell
   command string and passed to system(). an attacker can append shell
   metacharacters to execute arbitrary commands.

   try: ./cmd_injection "hello; id"
   try: ./cmd_injection "$(cat /etc/passwd)"  */

void command_injection(char *input)
{
    char command[256];
    snprintf(command, sizeof(command), "echo %s", input);
    system(command); /* input is not sanitized */
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <input>\n", argv[0]);
        return 1;
    }
    command_injection(argv[1]);
    return 0;
}
