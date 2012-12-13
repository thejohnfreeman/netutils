#include <netdb.h>     // NI_MAXHOST

#include <jfnet/inet.h>

#include "path.h"
#include "opts.h"

void path_print1(struct path* path, int ttl,  FILE* out) {
  struct probe* probe = path_getprobe(path, ttl, 0);
  union addr src      = probe->src;
  if (src.in_addr) {
    printf("%d.%d.%d.%d\n",
        src.octs[0], src.octs[1], src.octs[2], src.octs[3]);
  }
}

void path_println(struct path* path, int ttl,  FILE* out) {
  struct probe* probes          = path_getprobe(path, ttl, 0);
  struct probe* probe           = probes;
  int           nprobes         = path->nprobes;
  union addr    src             = { .in_addr = 0 };
  char hostname[NI_MAXHOST + 1] = { '\0' };

  printf("%2d ", ttl);

  for (; probe < probes + nprobes; ++probe) {

    if (probe->src.in_addr == 0) {
      printf(" *");
      continue;
    }

    if (probe->src.in_addr != src.in_addr) {
      src.in_addr = probe->src.in_addr;

      if (probe != probes) {
        printf("\n   ");
      }

      if (options.reverse_dns) {
        jf_unresolve4(probe->src.in_addr, hostname);
        printf(" %s (", hostname);
      }

      printf("%d.%d.%d.%d",
          src.octs[0], src.octs[1], src.octs[2], src.octs[3]);

      if (options.reverse_dns) {
        printf(")");
      }
    }

    printf("  %.3f ms", probe->ms);
  }

  puts("");
}

void path_print(struct path* path, int nhops, FILE* out) {
  for (int i = 0; i < nhops; ++i) {
    path_println(path, /*ttl=*/i + 1, out);
  }
}

