#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* race condition: two threads read, modify, and write a shared integer without
   synchronization. the read-modify-write is not atomic, so interleaving threads
   lose increments. expected final value is 200; actual value is less.

   fix: protect the critical section with a mutex. */

int shared = 0;

void *increment(void *arg)
{
    (void)arg;
    for (int i = 0; i < 100; i++) {
        int tmp = shared;          /* read */
        usleep(rand() % 50);       /* artificial delay to force interleaving */
        shared = tmp + 1;          /* write -- another thread may have written since read */
    }
    return NULL;
}

int main(void)
{
    srand((unsigned)time(NULL));

    pthread_t t1, t2;
    pthread_create(&t1, NULL, increment, NULL);
    pthread_create(&t2, NULL, increment, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("expected: 200\n");
    printf("actual:   %d\n", shared);
    return 0;
}
