#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#include <jfnet/ip.h>
#include <jfnet/io.h>

int sock;

void cleanup() {
  close(sock);
}

const char* usage =
"usage: ping host\n";

void ping(const char* destname, const struct sockaddr_in* dest,
    struct icmp* req)
{
  /* Send. */
  ssize_t nbytes = sendto(sock, req, ICMP_MINLEN, /*flags=*/0,
      (struct sockaddr*)dest, sizeof(*dest));
  if (-1 == nbytes) {
    perror("could not send echo request");
    exit(errno);
  }

  if (req->icmp_seq == 0) {
    u8_t* destocts = (u8_t*)&dest->sin_addr.s_addr;
    printf("PING %s (%d.%d.%d.%d): %ld data bytes\n",
        destname,
        destocts[0], destocts[1], destocts[2], destocts[3],
        nbytes);
  }

  /* Receive. */
#define MAX_PACKET_SIZE 64
  u8_t buffer[MAX_PACKET_SIZE];

  struct sockaddr_in respr;
  socklen_t respr_len = sizeof(respr);
  nbytes = recvfrom(sock, buffer, MAX_PACKET_SIZE, /*flags=*/0,
      (struct sockaddr*)&respr, &respr_len);
  if (-1 == nbytes) {
    perror("could not read response");
    exit(errno);
  }

  /* Verify. */
  struct ip* resp_ip = (struct ip*)buffer;
  size_t resp_ip_len = resp_ip->ip_hl << 2;
  //assert(resp_ip->ip_sum == htons(ip_cksum(resp_ip, resp_ip_len)));

  struct icmp* resp_icmp = (struct icmp*)(buffer + resp_ip_len);
  size_t resp_icmp_len   = ntohs(resp_ip->ip_len) - resp_ip_len;
  //assert(resp_icmp->icmp_cksum == htons(ip_cksum(resp_icmp, resp_icmp_len)));
  assert(resp_icmp->icmp_type  == ICMP_ECHOREPLY);
  assert(resp_icmp->icmp_code  == 0);
  assert(resp_icmp->icmp_id    == req->icmp_id);
  assert(resp_icmp->icmp_seq   == req->icmp_seq);

  /* Print. */
  printf("%ld bytes from %d.%d.%d.%d: icmp_seq=%d ttl=%d time=%.3f ms\n",
      nbytes,
      buffer[12], buffer[13], buffer[14], buffer[15],
      resp_icmp->icmp_seq, resp_ip->ip_ttl, 0.0);
}

int main(int argc, const char** argv) {

  /* Parse command line. */
  if (argc != 2) {
    fputs(usage, stderr);
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in dest;
  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  if (0 == inet_aton(argv[1], &dest.sin_addr)) {
    puts("could not parse address\n");
    exit(EXIT_FAILURE);
  }

  /* Construct socket. */
  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
  if (-1 == sock) {
    perror("could not acquire socket");
    exit(errno);
  }

  atexit(&cleanup);

  struct sockaddr_in src;
  memset(&src, 0, sizeof(src));
  src.sin_family      = AF_INET;
  src.sin_addr.s_addr = INADDR_ANY;

  if (-1 == bind(sock, (struct sockaddr*)&src, sizeof(src))) {
    perror("could not bind socket");
    exit(errno);
  }

  int ttl = 255;
  if (-1 == setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl))) {
    perror("could not set time-to-live");
    exit(errno);
  }

  /* Construct ICMP header. */
  struct icmp req;
  memset(&req, 0, sizeof(req));
  req.icmp_type  = ICMP_ECHO;
  req.icmp_code  = 0;
  req.icmp_id    = getpid();
  req.icmp_seq   = 0;
  req.icmp_cksum = ip_cksum(&req, ICMP_MINLEN);

  while (true) {
    ping(argv[1], &dest, &req);
    sleep(1);
    ++req.icmp_seq;
  }

  exit(EXIT_SUCCESS);
}

