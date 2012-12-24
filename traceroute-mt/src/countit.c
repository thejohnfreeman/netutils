#include "countit.h"

void countit_ctor(struct countit* it, struct iter_vtable* vt,
    int begin, int end)
{
  it->i          = begin;
  it->end        = end;
  vt->next       = &countit_next;
  vt->is_not_end = &countit_is_not_end;
  vt->get        = &countit_get;
}

void* countit_next(void* it) {
  struct countit* cit = (struct countit*)it;
  ++cit->i;
  return it;
}

bool countit_is_not_end(void* it) {
  struct countit* cit = (struct countit*)it;
  return cit->i != cit->end;
}

void* countit_get(void* it) {
  struct countit* cit = (struct countit*)it;
  return (void*)cit->i;
}

