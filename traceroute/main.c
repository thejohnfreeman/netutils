#include <assert.h>    // assert
#include <stdio.h>     // printf
#include <stdlib.h>    // exit, EXIT_FAILURE
#include <unistd.h>    // getpid
#include <netdb.h>     // NI_MAXHOST
#include <sys/errno.h> // errno
#include <sys/time.h>

#include <jfnet/icmp.h>
#include <jfnet/inet.h>

#define MAX_PACKET_SIZE 64

const char* usage =
"usage: traceroute host\n";

int tracehop(struct jfsock* sock, struct sockaddr_in* dest, int ttl,
    int nqueries)
{
  struct icmp* req = (struct icmp*)sock->buffer;
  u8_t buffer[MAX_PACKET_SIZE] = { 0 };
  char hostname[NI_MAXHOST + 1];
  struct timeval start, finish, timeout = { 0 };
  int notlast = 0;
  fd_set readfds;
  int wait = 5;

  timeout.tv_sec = wait;

  printf("%2d ", ttl);

  for (int i = 0; i < nqueries; ++i) {
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

    if (i == 0) {
      assert(src.sin_addr.s_addr == *(u32_t*)(buffer + 12));
      jf_unresolve(&src, hostname);
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

int main(int argc, const char** argv) {

  /* Parse command line. */
  if (argc != 2) {
    fputs(usage, stderr);
    exit(EXIT_FAILURE);
  }

  const char* destname = argv[1];

  struct sockaddr_in dest;
  jf_resolve(destname, &dest);

  /* Construct socket. */
  struct jfsock sock;
  struct icmp* req = jficmp_ctor(&sock, /*reserve=*/ICMP_MINLEN,
      /*type=*/ICMP_ECHO, /*code=*/0);

  /* Ping with max TTL just to confirm the destination is reachable. */
  int max_ttl = 64;
  jfsock_setttl(&sock, max_ttl);
  req->icmp_id  = getpid();
  req->icmp_seq = 0;
  ssize_t sendbytes = jficmp_send(&sock, &dest);

  {
    u8_t* destocts = (u8_t*)&dest.sin_addr.s_addr;
    printf("traceroute to %s (%d.%d.%d.%d), %d hops max, %ld byte packets\n",
        destname,
        destocts[0], destocts[1], destocts[2], destocts[3],
        max_ttl, sendbytes);
  }

  /* Receive. */
  u8_t buffer[MAX_PACKET_SIZE] = { 0 };
  struct sockaddr_in src;
  jficmp_recv(&sock, buffer, MAX_PACKET_SIZE, &src);

  /* Verify. */
  struct ip* resp_ip;
  struct icmp* resp_icmp;
  jficmp_open(buffer, &resp_ip, &resp_icmp);
  assert(resp_icmp->icmp_type == ICMP_ECHOREPLY);
  assert(resp_icmp->icmp_code == 0);
  assert(resp_icmp->icmp_id   == req->icmp_id);
  assert(resp_icmp->icmp_seq  == req->icmp_seq);

  int ttl      = 0;
  int nqueries = 3;
  int notlast  = 0;
  do {
    ++ttl;
    notlast = tracehop(&sock, &dest, ttl, nqueries);
  } while (notlast);

  return EXIT_SUCCESS;
}

