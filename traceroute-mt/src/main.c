#include <stdio.h>
#include <err.h>

#include "qmap.h"
#include "countit.h"

#define NUM_JOBS    10

bool trace1(void* in, void* out) {
  int ttl = *((int*)in);
  *((int*)out) = ttl;
  return (ttl > NUM_JOBS);
}

#define BUFFER_SIZE 4
#define NUM_THREADS 8

int main() {
  struct qmap qm;
  qmap_ctor(&qm, BUFFER_SIZE, sizeof(int));

  struct countit     it;
  struct iter_vtable vt;
  countit_ctor(&it, &vt, 0, NUM_JOBS);

  qmap_map(&qm, &it, &vt, NUM_THREADS, &trace1);

  int ttl;
  while (!qmap_empty(&qm)) {
    qm_pop(&qm, &ttl);
    printf("ttl = %d\n", ttl);
  }

  qmap_dtor(&qm);

  fflush(stdout);
  return 0;
}

