#ifndef TRACEROUTE_PATH_H
#define TRACEROUTE_PATH_H

#include <stdio.h>

#include <jfnet/inet.h>

struct probe {
  union addr src;
  float      ms;
};

struct path {
  struct probe* probes;
  int           max_ttl;
  /* These are probes per hop. The total number of probes allocated is
   *   nprobes * max_ttl
   */
  int           nprobes;
};

void path_ctor(struct path* path, int max_ttl, int nprobes);
struct probe* path_getprobe(struct path* path, int ttl, int seq);
void path_print(struct path* path, int nhops, FILE* out);
void path_println(struct path* path, int ttl,  FILE* out);
void path_print1(struct path* path, int ttl,  FILE* out);
void path_dtor(struct path* path);

#endif

