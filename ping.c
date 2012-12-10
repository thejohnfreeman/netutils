#include <assert.h>
#include <stdbool.h>
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

int sock;

void cleanup() {
  close(sock);
}

const char* usage =
"usage: ping host\n";

u_short ip_cksum(void* hdr, size_t size) {
  u_short* hwords = (u_short*)hdr;
  /* Size comes in bytes (8-bits). We want half-words (16-bits). */
  assert(size % 2 == 0);
  size >>= 1;

  u_int sum = 0;
  while (size--) {
    sum += *hwords++;
  }

  sum = (sum & 0xFFFF) + (sum >> 16);
  sum += sum >> 16;
  return ~sum;
}

int main(int argc, const char** argv) {

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

  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
  if (-1 == sock) {
    perror("could not acquire socket");
    exit(errno);
  }

  atexit(&cleanup);

  struct sockaddr_in src;
  memset(&dest, 0, sizeof(dest));
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

  u_short id  = (u_short)getpid();
  u_short seq = 0;

  struct icmp hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.icmp_type  = ICMP_ECHO;
  hdr.icmp_code  = 0;
  hdr.icmp_id    = id;
  hdr.icmp_seq   = seq;
  hdr.icmp_cksum = ip_cksum(&hdr, ICMP_MINLEN);

  ssize_t nbytes = sendto(sock, &hdr, ICMP_MINLEN, /*flags=*/0,
      (struct sockaddr*)&dest, sizeof(dest));
  if (-1 == nbytes) {
    perror("could not send echo request");
    exit(errno);
  }

  exit(EXIT_SUCCESS);
}

