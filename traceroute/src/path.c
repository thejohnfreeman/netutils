#include <stdlib.h>    // malloc
#include <string.h>

#include "path.h"

void path_ctor(struct path* path, int max_ttl, int nprobes) {
  path->max_ttl = max_ttl;
  path->nprobes = nprobes;
  size_t size = max_ttl * nprobes * sizeof(struct probe);
  path->probes = (struct probe*)malloc(size);
  memset(path->probes, 0, size);
}

struct probe* path_getprobe(struct path* path, int ttl, int seq) {
  return path->probes + (ttl * path->nprobes * sizeof(struct probe)) + seq;
}

void path_dtor(struct path* path) {
  free(path->probes);
  path->probes = NULL;
}

