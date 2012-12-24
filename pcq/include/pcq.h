#ifndef PRODUCER_CONSUMER_QUEUE_H
#define PRODUCER_CONSUMER_QUEUE_H

#include <stdbool.h>
#include <pthread.h>

struct pcq_token {
  int  index;
  bool is_valid;
};

struct pcq_slot {
  bool  is_full;
  void* data;
};

struct pcq_queue {
  int              size;
  struct pcq_slot* buffer;
  /* Slot for next token. Always ahead. May not lap icon. */
  int              ipro;
  /* Slot for next data. Always behind. May not pass ipro. */
  int              icon;
  pthread_mutex_t  mutex;
  pthread_cond_t   is_not_full;
  pthread_cond_t   is_not_empty;
};

void
pcq_ctor(struct pcq_queue* q, int n);
bool
pcq_empty(struct pcq_queue* q);
struct pcq_token
pcq_claim(struct pcq_queue* q);
void
pcq_push(struct pcq_queue* q, struct pcq_token* tok, void* data);
void*
pcq_pop(struct pcq_queue* q);

#endif

