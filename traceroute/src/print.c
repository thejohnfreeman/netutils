#include <netdb.h>     // NI_MAXHOST

#include <jfnet/inet.h>

#include "path.h"
#include "opts.h"

static void path_println(struct probe* probes, int nprobes, FILE* out) {
  struct probe* probe           = probes;
  union addr    src             = { .in_addr = 0 };
  char hostname[NI_MAXHOST + 1] = { '\0' };

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
}

void path_print(struct path* path, int nhops, FILE* out) {
  struct probe* probes = path->probes;
  for (int i = 0; i < nhops; ++i) {
    printf("%2d ", i);
    path_println(probes, path->nprobes, out);
    puts("");
    probes += path->nprobes;
  }
}

