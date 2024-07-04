#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int shared_resource = 0;

void *increment(void *arg) {
  for (int i = 0; i < 100; ++i) {
    int temp = shared_resource; // read shared resource
    usleep(rand() % 100);   // sleep for a random time to increase the chance of
                            // context switch
    temp = temp + 1;        // increment the value
    shared_resource = temp; // write back to shared resource
    usleep(rand() % 100);   // random sleep
  }
  return NULL;
}

int main() {
  pthread_t thread1, thread2;

  srand(time(NULL)); // seed random number generator for usleep

  pthread_create(&thread1, NULL, increment, NULL);
  pthread_create(&thread2, NULL, increment, NULL);

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  printf("Final value of shared resource: %d\n", shared_resource);
  return 0;
}
