#ifndef TRACEROUTE_MT_JOBS_H
#define TRACEROUTE_MT_JOBS_H

#include <pthread.h>
#include <pcq.h>

#include "iter.h"

struct qmap {
  pthread_t*        threads;
  int               nthreads;
  struct pcq_queue* q;
  pthread_mutex_t   mutex;
};

void
qmap_ctor(struct qmap* qm, int num, int size);
void
qmap_map(struct qmap* qm, void* it, struct iter_vtable* vt, int nworkers,
    bool (*f)(void* in, void* out));
bool
qmap_empty(struct qmap* qm);
void
qmap_pop(struct qmap* qm, void* out);
void
qmap_dtor(struct qmap* qm);

#endif

