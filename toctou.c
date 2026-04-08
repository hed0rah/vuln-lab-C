#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

/* TOCTOU (time-of-check time-of-use): access(2) checks whether the calling
   process can reach a file, but the check and the subsequent open(2) are two
   separate syscalls with a window between them. an attacker who can modify the
   filesystem (e.g., replace a regular file with a symlink) during that window
   can redirect the open to a file the check would have rejected.

   classic attack:
     1. program calls access("/tmp/userfile", R_OK) -- passes
     2. attacker swaps /tmp/userfile -> symlink to /etc/shadow
     3. program calls open("/tmp/userfile") -- opens /etc/shadow

   fix: open the file first, then use fstat() on the fd to verify attributes.
   never make a security decision based on a path after separately checking it. */

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <path>\n", argv[0]);
        return 1;
    }

    const char *path = argv[1];

    /* check */
    if (access(path, R_OK) != 0) {
        perror("access denied");
        return 1;
    }

    /* window: attacker can swap the file here */
    sleep(2); /* simulates the delay; remove to see the race disappear */

    /* use */
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open failed");
        return 1;
    }

    char buf[128];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("read: %s\n", buf);
    }
    close(fd);
    return 0;
}
