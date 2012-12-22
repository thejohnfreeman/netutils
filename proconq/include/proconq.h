#ifndef PROCONQ_PROCONQ_H
#define PROCONQ_PROCONQ_H

#include <stdbool.h>
#include <pthread.h>

struct token {
  int  index;
  bool is_valid;
};

struct slot {
  bool  is_full;
  void* data;
};

struct proconq {
  int             size;
  struct slot*    queue;
  /* Slot for next token. Always ahead. May not lap icon. */
  int             ipro;
  /* Slot for next data. Always behind. May not pass ipro. */
  int             icon;
  pthread_mutex_t mutex;
  pthread_cond_t  is_not_full;
  pthread_cond_t  is_not_empty;
};

void
proconq_ctor(struct proconq* pcq, int n);
bool
proconq_empty(struct proconq* pcq);
struct token
proconq_claim(struct proconq* pcq);
void
proconq_push(struct proconq* pcq, struct token* tok, void* data);
void*
proconq_pop(struct proconq* pcq);

#endif

