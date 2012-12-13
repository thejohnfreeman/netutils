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

bool pinghop_icmp(struct jfsock* sock, struct sockaddr_in* dest, int ttl,
    struct path* path)
{
  bool           reached_dest  = false;
  struct icmp*   req           = (struct icmp*)sock->buffer;
  struct timeval timeout       = { 0 };
  u8_t buffer[MAX_PACKET_SIZE] = { 0 };
  struct timeval start, finish;
  fd_set readfds;

  timeout.tv_sec = options.recvwait;

  for (int i = 0; i < path->nprobes; ++i) {
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

    /* In case of timeout, don't worry about the rest. */
    if (!isready) {
      continue;
    }

    /* Stop timer. */
    gettimeofday(&finish, /*timezone=*/NULL);
    float ms = (((finish.tv_sec - start.tv_sec) * 1000000.0) +
        (finish.tv_usec - start.tv_usec)) / 1000.0;

    /* Receive. */
    struct sockaddr_in src;
    jficmp_recv(sock, buffer, MAX_PACKET_SIZE, &src);

    struct probe* probe = path_getprobe(path, ttl, i);
    probe->src.in_addr  = src.sin_addr.s_addr;
    probe->ms           = ms;

    /* Check if we found the last hop. */
    struct ip* resp_ip;
    struct icmp* resp_icmp;
    jficmp_open(buffer, &resp_ip, &resp_icmp);
    assert(src.sin_addr.s_addr == *(u32_t*)(buffer + 12));
    if (resp_icmp->icmp_type == ICMP_ECHOREPLY) {
      reached_dest = true;
    }
  }

  return reached_dest;
}

