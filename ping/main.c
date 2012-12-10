#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <float.h>
#include <math.h>
#include <signal.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#include <jfnet/ip.h>
#include <jfnet/io.h>

int sock;
const char* destname;
u_int nsent   = 0;
u_int nrecv   = 0;
float min     = FLT_MAX;
float max     = 0.0;
float total   = 0.0;
float total_2 = 0.0;

void cleanup() {
  close(sock);
}

void report() {
  printf("\n--- %s ping statistics ---\n", destname);
  printf("%d packets transmitted, %d packets received, %.1f%% packet loss\n",
      nsent, nrecv, (nsent - nrecv) / (float)nsent);
  float mean   = total / nrecv;
  float stddev = sqrt((total_2 / nrecv) - (mean * mean));
  printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
      min, mean, max, stddev);
}

void onint(int sig) {
  cleanup();
  if (sig == SIGINT) {
    report();
    fflush(/*stream=*/NULL);
    errno = 0;
  }
  _exit(errno);
}

void onexit() {
  cleanup();
}

const char* usage =
"usage: ping host\n";

void ping(const struct sockaddr_in* dest, struct icmp* req) {
  /* Prepare to receive. We want minimal processing between send and
   * receive. */
#define MAX_PACKET_SIZE 64
  u8_t buffer[MAX_PACKET_SIZE] = { 0 };

  struct sockaddr_in respr;
  socklen_t respr_len = sizeof(respr);

  /* Start timer. */
  struct timeval start, finish;
  gettimeofday(&start, /*timezone=*/NULL);

  /* Send. */
  ssize_t sendbytes = sendto(sock, req, ICMP_MINLEN, /*flags=*/0,
      (struct sockaddr*)dest, sizeof(*dest));
  if (-1 == sendbytes) {
    perror("could not send echo request");
    exit(errno);
  }
  ++nsent;

  /* Print. */
  if (req->icmp_seq == 0) {
    u8_t* destocts = (u8_t*)&dest->sin_addr.s_addr;
    printf("PING %s (%d.%d.%d.%d): %ld data bytes\n",
        destname,
        destocts[0], destocts[1], destocts[2], destocts[3],
        sendbytes);
  }

  /* Receive. */
  ssize_t recvbytes = recvfrom(sock, buffer, MAX_PACKET_SIZE, /*flags=*/0,
      (struct sockaddr*)&respr, &respr_len);
  if (-1 == recvbytes) {
    perror("could not read response");
    exit(errno);
  }
  ++nrecv;

  /* Stop timer. */
  gettimeofday(&finish, /*timezone=*/NULL);
  float ms = (((finish.tv_sec - start.tv_sec) * 1000000.0) +
      (finish.tv_usec - start.tv_usec)) / 1000.0;
  if (ms < min) min = ms;
  if (ms > max) max = ms;
  total   += ms;
  total_2 += ms * ms;

  /* Verify. */
  struct ip* resp_ip = (struct ip*)buffer;
  size_t resp_ip_len = resp_ip->ip_hl << 2;
  /* It appears Mac OS changes `ip_len` to just the data length in host
   * representation. */
  resp_ip->ip_len = htons(resp_ip->ip_len + resp_ip_len);
  assert(0 == ip_cksum(resp_ip, resp_ip_len));

  struct icmp* resp_icmp = (struct icmp*)(buffer + resp_ip_len);
  size_t resp_icmp_len   = ntohs(resp_ip->ip_len) - resp_ip_len;
  assert(0 == ip_cksum(resp_icmp, resp_icmp_len));
  assert(resp_icmp->icmp_type  == ICMP_ECHOREPLY);
  assert(resp_icmp->icmp_code  == 0);
  assert(resp_icmp->icmp_id    == req->icmp_id);
  assert(resp_icmp->icmp_seq   == req->icmp_seq);

  /* Print. */
  printf("%ld bytes from %d.%d.%d.%d: icmp_seq=%d ttl=%d time=%.3f ms\n",
      recvbytes,
      buffer[12], buffer[13], buffer[14], buffer[15],
      ntohs(resp_icmp->icmp_seq), resp_ip->ip_ttl, ms);
}

int main(int argc, const char** argv) {

  /* Parse command line. */
  if (argc != 2) {
    fputs(usage, stderr);
    exit(EXIT_FAILURE);
  }

  destname = argv[1];

  struct sockaddr_in dest;
  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  if (0 == inet_aton(destname, &dest.sin_addr)) {
    puts("could not parse address\n");
    exit(EXIT_FAILURE);
  }

  /* Construct socket. */
  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
  if (-1 == sock) {
    perror("could not acquire socket");
    exit(errno);
  }

  /* Register function to close socket and report statistics when the process
   * dies. */
  atexit(&onexit);

  struct sigaction act;
  act.sa_handler = &onint;
  if (-1 == sigaction(SIGINT, &act, /*oact=*/NULL)) {
    perror("could not assign cleanup");
    exit(errno);
  }

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

  while (true) {
    req.icmp_cksum = 0;
    req.icmp_cksum = ip_cksum(&req, ICMP_MINLEN);
    ping(&dest, &req);
    sleep(1);
    u16_t seq    = ntohs(req.icmp_seq) + 1;
    req.icmp_seq = htons(seq);
  }

  exit(EXIT_SUCCESS);
}

