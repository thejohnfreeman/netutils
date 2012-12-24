#ifndef ITERATOR_H
#define ITERATOR_H

struct iter_vtable {
  void* (*next)      (void* it);
  bool  (*is_not_end)(void* it);
  void* (*get)       (void* it);
};

#endif

