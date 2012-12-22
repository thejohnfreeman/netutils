#ifndef TRACEROUTE_MT_JOBS_H
#define TRACEROUTE_MT_JOBS_H

#include <pthread.h>
#include <proconq.h>

struct job {
  int          ttl;
  struct token tok;
};

struct jobsq {
  pthread_mutex_t mutex;
  /* Job creator. */
  int             next;
  bool            done;
  struct proconq  pcq;
};

void
jobsq_ctor(struct jobsq* jq, int n);
struct job
jobsq_next(struct jobsq* jq);

#endif

