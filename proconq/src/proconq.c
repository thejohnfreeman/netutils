#include <assert.h>
#include <stdlib.h>
#include <err.h>

#include "proconq.h"

void
proconq_ctor(struct proconq* pcq, int n) {
  pcq->size  = n;
  pcq->queue = (struct slot*)malloc(n * sizeof(struct slot));
  pcq->ipro  = 0;
  pcq->icon  = 0;
  int error;
  if ((error = pthread_mutex_init(&pcq->mutex, /*attr=*/NULL))) {
    errc(error, error, "could not acquire mutex");
  }
  if ((error = pthread_cond_init(&pcq->is_not_full, /*attr=*/NULL))) {
    errc(error, error, "could not acquire condition variable");
  }
  if ((error = pthread_cond_init(&pcq->is_not_empty, /*attr=*/NULL))) {
    errc(error, error, "could not acquire condition variable");
  }
}

#define rotate(i, n) (((i) + 1) % (n))

bool
proconq_empty(struct proconq* pcq) {
  pthread_mutex_lock(&pcq->mutex);
  bool rv = true;
  for (int i = pcq->icon; i != pcq->ipro; i = rotate(i, pcq->size)) {
    if (pcq->queue[i].is_full) {
      rv = false;
      break;
    }
  }
  pthread_mutex_unlock(&pcq->mutex);
  return rv;
}

struct token
proconq_claim(struct proconq* pcq) {
  pthread_mutex_lock(&pcq->mutex);
  while (rotate(pcq->ipro, pcq->size) == pcq->icon) {
    pthread_cond_wait(&pcq->is_not_full, &pcq->mutex);
  }
  struct token tok = { pcq->ipro, /*is_valid=*/true };
  pcq->ipro        = rotate(pcq->ipro, pcq->size);
  pthread_mutex_unlock(&pcq->mutex);
  return tok;
}

void
proconq_push(struct proconq* pcq, struct token* tok, void* data) {
  if (!tok->is_valid) {
    /* Token expired. */
    return;
  }
  pthread_mutex_lock(&pcq->mutex);
  struct slot* slot = pcq->queue + tok->index;
  assert(!slot->is_full);
  slot->data    = data;
  slot->is_full = true;
  tok->is_valid = false;
  pthread_cond_signal(&pcq->is_not_empty);
  pthread_mutex_unlock(&pcq->mutex);
}

void*
proconq_pop(struct proconq* pcq) {
  pthread_mutex_lock(&pcq->mutex);
  struct slot* slot;
  while (!(slot = pcq->queue + pcq->icon)->is_full) {
    pthread_cond_wait(&pcq->is_not_empty, &pcq->mutex);
  }
  void* data    = slot->data;
  slot->is_full = false;
  pcq->icon     = rotate(pcq->icon, pcq->size);
  pthread_cond_signal(&pcq->is_not_full);
  pthread_mutex_unlock(&pcq->mutex);
  return data;
}

