#ifndef TRACEROUTE_PATH_H
#define TRACEROUTE_PATH_H

#include <stdio.h>

#include <jfnet/byteorder.h>

struct hop {
  u32_t ipaddr;
  float ms;
};

struct path {
  struct hop* hops;
  int         max_ttl;
  int         nqueries;
};

void path_ctor(struct path* path, int max_ttl, int nqueries);
struct hop* path_gethop(struct path* path, int ttl, int seq);
void path_print(struct path* path, FILE* out);
void path_dtor(struct path* path);

#endif

