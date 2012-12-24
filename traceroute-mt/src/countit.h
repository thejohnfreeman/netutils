#ifndef COUNTING_ITERATOR_H
#define COUNTING_ITERATOR_H

#include "iter.h"

struct countit {
  int i;
  int end;
};

void countit_ctor(struct countit* it, struct iter_vtable* vt,
    int begin, int end);
void* countit_next(void* it);
bool countit_is_not_end(void* it);
void* countit_get(void* it);

#endif

