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
};

struct pcq_queue {
  int             num;
  int             size;
  char*           buffer;
  /* Slot for next token. Always ahead. May not lap icon. */
  int             ipro;
  /* Slot for next data. Always behind. May not pass ipro. */
  int             icon;
  bool            is_closed;
  pthread_mutex_t mutex;
  pthread_cond_t  is_not_full;
  pthread_cond_t  is_not_empty;
};

void
pcq_ctor(struct pcq_queue* q, int num, int size);
int
pcq_sizeoft(struct pcq_queue* q);
bool
pcq_empty(struct pcq_queue* q);
struct pcq_token
pcq_claim(struct pcq_queue* q);
int
pcq_push(struct pcq_queue* q, struct pcq_token* tok, bool is_last, void* in);
int
pcq_pop(struct pcq_queue* q, void* out);
void
pcq_dtor(struct pcq_queue* q);

#endif

