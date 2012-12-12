#include <assert.h>    // assert
#include <stdio.h>     // printf
#include <stdlib.h>    // exit, EXIT_FAILURE
#include <netdb.h>     // NI_MAXHOST
#include <sys/errno.h> // errno
#include <sys/time.h>

#include <jfnet/icmp.h>
#include <jfnet/inet.h>

#include "ping.h"
#include "path.h"
#include "opts.h"

int pinghop_icmp(struct jfsock* sock, struct sockaddr_in* dest, int ttl,
    struct path* path)
{
  struct icmp* req = (struct icmp*)sock->buffer;
  u8_t buffer[MAX_PACKET_SIZE] = { 0 };
  char hostname[NI_MAXHOST + 1];
  struct timeval start, finish, timeout = { 0 };
  int notlast = 0;
  fd_set readfds;
  in_addr_t src_prev = 0;

  timeout.tv_sec = options.recvwait;

  printf("%2d ", ttl);

  for (int i = 0; i < options.nqueries; ++i) {
    /* Start timer. */
    gettimeofday(&start, /*timezone=*/NULL);

    /* Send. */
    jfsock_setttl(sock, ttl);
    req->icmp_seq = htons(i);
    jficmp_send(sock, dest);

    /* Wait for response. */
    FD_ZERO(&readfds);
    FD_SET(sock->fd, &readfds);
    int isready = select(/*nfds=*/sock->fd + 1, &readfds,
        /*writefds=*/NULL, /*exceptfds=*/NULL, &timeout);
    if (isready < 0) {
      perror("unexpected error waiting for response");
      jfsock_dtor(sock);
      exit(errno);
    }

    /* In case of timeout, show progress. */
    if (!isready) {
      printf(" *");
      fflush(NULL);
      notlast = 1;
      continue;
    }

    /* Stop timer. */
    gettimeofday(&finish, /*timezone=*/NULL);
    float ms = (((finish.tv_sec - start.tv_sec) * 1000000.0) +
        (finish.tv_usec - start.tv_usec)) / 1000.0;

    /* Receive. */
    struct sockaddr_in src;
    jficmp_recv(sock, buffer, MAX_PACKET_SIZE, &src);

    /* Print. */
    struct ip* resp_ip;
    struct icmp* resp_icmp;
    jficmp_open(buffer, &resp_ip, &resp_icmp);

    assert(src.sin_addr.s_addr == *(u32_t*)(buffer + 12));
    if (src.sin_addr.s_addr != src_prev) {
      src_prev = src.sin_addr.s_addr;
      jf_unresolve(&src, hostname);
      if (i != 0) {
        printf("\n   ");
      }
      printf(" %s (%d.%d.%d.%d)",
          hostname,
          buffer[12], buffer[13], buffer[14], buffer[15]);
    }

    printf("  %.3f ms", ms);

    /* Check if we need to continue. */
    if (resp_icmp->icmp_type == ICMP_TIMXCEED) {
      notlast = 1;
    }
  }

  puts("");

  return notlast;
}

