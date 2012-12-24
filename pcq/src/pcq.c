#include <assert.h>
#include <stdlib.h>
#include <err.h>

#include "pcq.h"

void
pcq_ctor(struct pcq_queue* q, int n) {
  q->size   = n;
  q->buffer = (struct pcq_slot*)malloc(n * sizeof(struct pcq_slot));
  q->ipro   = 0;
  q->icon   = 0;
  int error;
  if ((error = pthread_mutex_init(&q->mutex, /*attr=*/NULL))) {
    errc(error, error, "could not acquire mutex");
  }
  if ((error = pthread_cond_init(&q->is_not_full, /*attr=*/NULL))) {
    errc(error, error, "could not acquire condition variable");
  }
  if ((error = pthread_cond_init(&q->is_not_empty, /*attr=*/NULL))) {
    errc(error, error, "could not acquire condition variable");
  }
}

#define rotate(i, n) (((i) + 1) % (n))

bool
pcq_empty(struct pcq_queue* q) {
  pthread_mutex_lock(&q->mutex);
  bool rv = true;
  for (int i = q->icon; i != q->ipro; i = rotate(i, q->size)) {
    if (q->buffer[i].is_full) {
      rv = false;
      break;
    }
  }
  pthread_mutex_unlock(&q->mutex);
  return rv;
}

struct pcq_token
pcq_claim(struct pcq_queue* q) {
  pthread_mutex_lock(&q->mutex);
  while (rotate(q->ipro, q->size) == q->icon) {
    pthread_cond_wait(&q->is_not_full, &q->mutex);
  }
  struct pcq_token tok = { q->ipro, /*is_valid=*/true };
  q->ipro        = rotate(q->ipro, q->size);
  pthread_mutex_unlock(&q->mutex);
  return tok;
}

void
pcq_push(struct pcq_queue* q, struct pcq_token* tok, void* data) {
  if (!tok->is_valid) {
    /* Token expired. */
    return;
  }
  pthread_mutex_lock(&q->mutex);
  struct pcq_slot* slot = q->buffer + tok->index;
  assert(!slot->is_full);
  slot->data    = data;
  slot->is_full = true;
  tok->is_valid = false;
  pthread_cond_signal(&q->is_not_empty);
  pthread_mutex_unlock(&q->mutex);
}

void*
pcq_pop(struct pcq_queue* q) {
  pthread_mutex_lock(&q->mutex);
  struct pcq_slot* slot;
  while (!(slot = q->buffer + q->icon)->is_full) {
    pthread_cond_wait(&q->is_not_empty, &q->mutex);
  }
  void* data    = slot->data;
  slot->is_full = false;
  q->icon     = rotate(q->icon, q->size);
  pthread_cond_signal(&q->is_not_full);
  pthread_mutex_unlock(&q->mutex);
  return data;
}

