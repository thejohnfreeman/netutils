#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "pcq.h"

void
pcq_ctor(struct pcq_queue* q, int num, int size) {
  q->num       = num;
  q->size      = size;
  q->buffer    = (char*)calloc(num, sizeof(struct pcq_slot) + size);
  q->ipro      = 0;
  q->icon      = 0;
  q->is_closed = false;
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

int
pcq_sizeoft(struct pcq_queue* q) {
  return q->size;
}

#define rotate(i, n) (((i) + 1) % (n))
#define get_slot(buffer, size, i) \
  ((struct pcq_slot*)((buffer) + ((sizeof(struct pcq_slot) + (size)) * (i))))
#define get_data(buffer, size, i) \
  ((buffer) + ((sizeof(struct pcq_slot) + (size)) * (i)) + \
   sizeof(struct pcq_slot))

bool
pcq_empty(struct pcq_queue* q) {
  pthread_mutex_lock(&q->mutex);
  bool rv = q->is_closed && (q->ipro == q->icon);
  pthread_mutex_unlock(&q->mutex);
  return rv;
}

struct pcq_token
pcq_claim(struct pcq_queue* q) {
  pthread_mutex_lock(&q->mutex);
  while (!q->is_closed && (rotate(q->ipro, q->num) == q->icon)) {
    pthread_cond_wait(&q->is_not_full, &q->mutex);
  }
  struct pcq_token tok = { q->ipro, /*is_valid=*/false };
  if (!q->is_closed) {
    tok.is_valid = true;
    q->ipro      = rotate(q->ipro, q->num);
  }
  pthread_mutex_unlock(&q->mutex);
  return tok;
}

int
pcq_push(struct pcq_queue* q, struct pcq_token* tok, bool is_last,  void* in)
{
  if (!tok->is_valid) {
    /* Token expired. */
    return EXIT_FAILURE;
  }
  pthread_mutex_lock(&q->mutex);
  struct pcq_slot* slot = get_slot(q->buffer, q->size, tok->index);
  assert(!slot->is_full);
  slot->is_full = true;
  memcpy(get_data(q->buffer, q->size, tok->index), in, q->size);
  tok->is_valid = false;
  if (is_last) {
    q->ipro      = rotate(tok->index, q->num);
    q->is_closed = true;
  }
  pthread_cond_signal(&q->is_not_empty);
  pthread_mutex_unlock(&q->mutex);
  return EXIT_SUCCESS;
}

int
pcq_pop(struct pcq_queue* q, void* out) {
  pthread_mutex_lock(&q->mutex);
  struct pcq_slot* slot;
  while (!(slot = get_slot(q->buffer, q->size, q->icon))->is_full) {
    pthread_cond_wait(&q->is_not_empty, &q->mutex);
  }
  memcpy(out, get_data(q->buffer, q->size, q->icon), q->size);
  slot->is_full = false;
  q->icon       = rotate(q->icon, q->num);
  pthread_cond_signal(&q->is_not_full);
  pthread_mutex_unlock(&q->mutex);
  return EXIT_SUCCESS;
}

void
pcq_dtor(struct pcq_queue* q) {
  free(q->buffer);
}

