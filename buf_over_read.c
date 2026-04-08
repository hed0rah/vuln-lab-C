#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* buffer over-read (Heartbleed-style): the caller supplies a claimed payload
   length that the server echoes back. if the claimed length exceeds the actual
   payload length, the response contains bytes from adjacent memory -- leaking
   heap contents including keys, cookies, or other sensitive data.

   CVE-2014-0160 (Heartbleed) used exactly this pattern in OpenSSL's TLS
   heartbeat handler.

   try: ./buf_over_read hello 200   */

#define MAX_PAYLOAD 256

void heartbeat(const char *payload, unsigned int claimed_len)
{
    char response[MAX_PAYLOAD];

    if (claimed_len > MAX_PAYLOAD) {
        printf("claimed length too large\n");
        return;
    }

    /* bug: copies claimed_len bytes, not strlen(payload).
       if claimed_len > strlen(payload), bytes after the null terminator
       are included in the response. */
    memcpy(response, payload, claimed_len);
    response[claimed_len] = '\0';

    printf("response (%u bytes): ", claimed_len);
    for (unsigned int i = 0; i < claimed_len; i++) {
        unsigned char c = (unsigned char)response[i];
        printf(c >= 0x20 && c < 0x7f ? "%c" : "\\x%02x", c);
    }
    putchar('\n');
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s <payload> <claimed_length>\n", argv[0]);
        return 1;
    }

    /* place something sensitive adjacent in memory */
    char secret[64];
    strcpy(secret, "session_token=deadbeef1234");

    const char *payload = argv[1];
    unsigned int claimed = (unsigned int)atoi(argv[2]);

    heartbeat(payload, claimed);
    return 0;
}
