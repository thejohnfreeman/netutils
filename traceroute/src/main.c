#include <assert.h>    // assert
#include <stdio.h>     // printf
#include <stdlib.h>    // exit, EXIT_FAILURE
#include <netdb.h>     // NI_MAXHOST
#include <sys/errno.h> // errno
#include <sys/time.h>

#include <jfnet/icmp.h>
#include <jfnet/inet.h>

#include "opts.h"

#define MAX_PACKET_SIZE 64

int tracehop(struct jfsock* sock, struct sockaddr_in* dest, int ttl) {
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

int main(int argc, char** argv) {
  /* Parse command line. */
  parse_options(argc, argv);

  struct sockaddr_in dest;
  jf_resolve(options.dest, &dest);

  /* Construct socket. */
  struct jfsock sock;
  struct icmp* req = jficmp_ctor(&sock, /*reserve=*/ICMP_MINLEN,
      /*type=*/ICMP_ECHO, /*code=*/0);

  /* Ping with max TTL just to confirm the destination is reachable. */
  jfsock_setttl(&sock, options.max_ttl);
  req->icmp_id  = options.port;
  req->icmp_seq = 0;
  ssize_t sendbytes = jficmp_send(&sock, &dest);

  {
    u8_t* destocts = (u8_t*)&dest.sin_addr.s_addr;
    printf("traceroute to %s (%d.%d.%d.%d), %d hops max, %ld byte packets\n",
        options.dest,
        destocts[0], destocts[1], destocts[2], destocts[3],
        options.max_ttl, sendbytes);
  }

  /* Receive. */
  u8_t buffer[MAX_PACKET_SIZE] = { 0 };
  jficmp_recv(&sock, buffer, MAX_PACKET_SIZE, /*src=*/NULL);

  /* Verify. */
  struct ip* resp_ip;
  struct icmp* resp_icmp;
  jficmp_open(buffer, &resp_ip, &resp_icmp);
  assert(resp_icmp->icmp_type == ICMP_ECHOREPLY);
  assert(resp_icmp->icmp_code == 0);
  assert(resp_icmp->icmp_id   == req->icmp_id);
  assert(resp_icmp->icmp_seq  == req->icmp_seq);

  int ttl     = options.first_ttl - 1;
  int notlast = 0;
  do {
    ++ttl;
    notlast = tracehop(&sock, &dest, ttl);
  } while (notlast);

  return EXIT_SUCCESS;
}

