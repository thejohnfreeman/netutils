#include <stdlib.h>    // malloc
#include <netdb.h>     // NI_MAXHOST

#include "path.h"
#include "opts.h"

void path_ctor(struct path* path, int max_ttl, int nqueries) {
  path->hops = (struct hop*)malloc(max_ttl * nqueries * sizeof(struct hop));
}

struct hop* path_gethop(struct path* path, int ttl, int seq) {
  return path->hops + (ttl * path->nqueries * sizeof(struct hop)) + seq;
}

void path_print(struct path* path, int nhops, FILE* out) {
  struct hop* hop = path->hops;
  char hostname[NI_MAXHOST + 1] = { '\0' };

  for (int ihop = 0; ihop < nhops; ++ihop) {
    in_addr_t ipaddr_prev = 0;

    printf("%2d ", ihop);

    for (int iseq = 0; iseq < path->nqueries; ++iseq) {
      if (hop->ipaddr == 0) {
        printf(" *");
        continue;
      }

      if (hop->ipaddr != ipaddr_prev) {
        ipaddr_prev = hop->ipaddr;

        if (options.reverse_dns) {
          jf_unresolve4(hop->ipaddr, hostname);
        }

        if (iseq != 0) {
          printf("\n   ");
        }

        printf(" %s (%d.%d.%d.%d)",
            hostname,
            buffer[12], buffer[13], buffer[14], buffer[15]);
      }

      printf("  %.3f ms", hop->ms);
    }

    puts("");
  }
}

void path_dtor(struct path* path) {
  free(path->hops);
  path->hops = NULL;
}

