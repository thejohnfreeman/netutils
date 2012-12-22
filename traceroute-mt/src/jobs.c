#include "jobs.h"

void
jobsq_ctor(struct jobsq* jq, int n) {
  pthread_mutex_init(&jq->mutex, /*attr=*/NULL);
  jq->next = 1;
  jq->done = false;
  proconq_ctor(&jq->pcq, n);
}

struct job
jobsq_next(struct jobsq* jq) {
  pthread_mutex_lock(&jq->mutex);
  struct job rv = { jq->next++, proconq_claim(&jq->pcq) };
  pthread_mutex_unlock(&jq->mutex);
  return rv;
}

