#include <stdio.h>
#include <err.h>

#include "jobs.h"

#define BUFFER_SIZE 4
#define NUM_JOBS    10

void* trace1(void* args) {
  struct jobsq* jq = (struct jobsq*)args;

  while (1) {
    /* Job start. */
    struct job j = jobsq_next(jq);

    /* What to do if no jobs? */
    if (j.ttl > NUM_JOBS) {
      return NULL;
    }

    /* Job finish. */
    proconq_push(&jq->pcq, &j.tok, (void*)j.ttl);
    if (j.ttl == NUM_JOBS) {
      jq->done = true;
    }
  }
}

#define NUM_THREADS 8

int main() {
  struct jobsq jq;
  jobsq_ctor(&jq, BUFFER_SIZE);

  pthread_t threads[NUM_THREADS] = { NULL };

  int error;
  for (int i = 0; i < NUM_THREADS; ++i) {
    if ((error = pthread_create(threads + i, /*attr=*/NULL, &trace1, &jq))) {
      errc(error, error, "could not create thread");
    }
  }

  while (!jq.done || !proconq_empty(&jq.pcq)) {
    int ttl = (int)proconq_pop(&jq.pcq);
    printf("ttl = %d\n", ttl);
  }

  fflush(stdout);
  return 0;
}

