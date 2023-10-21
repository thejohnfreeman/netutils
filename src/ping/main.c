#include <assert.h>     // assert
#include <stdio.h>      // printf
#include <string.h>     // memset
#include <float.h>      // FLT_MAX
#include <math.h>       // sqrt
#include <signal.h>     // sigaction
#include <unistd.h>     // close
#include <err.h>        // errno
#include <errno.h>      // errno
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <jfnet/ip.h>
#include <jfnet/icmp.h>
#include <jfnet/inet.h>

struct jfsock sock;
const char* destname;
u_int nsent   = 0;
u_int nrecv   = 0;
float min     = FLT_MAX;
float max     = 0.0;
float total   = 0.0;
float total_2 = 0.0;

void report() {
  printf("\n--- %s ping statistics ---\n", destname);
  printf("%d packets transmitted, %d packets received, %.1f%% packet loss\n",
      nsent, nrecv, (nsent - nrecv) / (float)nsent);
  float mean   = total / nrecv;
  float stddev = sqrt((total_2 / nrecv) - (mean * mean));
  printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
      min, mean, max, stddev);
}

void onsig(int sig) {
  jfsock_dtor(&sock);
  if (sig == SIGINT) {
    report();
    fflush(/*stream=*/NULL);
    errno = 0;
  }
  _exit(errno);
}

const char* usage =
"usage: ping host";

void ping(const struct sockaddr_in* dest) {
  /* Prepare to receive. We want minimal processing between send and
   * receive. */
  struct icmp* req = (struct icmp*)sock.buffer;
#define MAX_PACKET_SIZE 64
  u8_t buffer[MAX_PACKET_SIZE] = { 0 };
  struct sockaddr_in src;

  /* Start timer. */
  struct timeval start, finish;
  gettimeofday(&start, /*timezone=*/NULL);

  /* Send. */
  ssize_t sendbytes = jficmp_send(&sock, dest);
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
  ssize_t recvbytes = jficmp_recv(&sock, buffer, MAX_PACKET_SIZE, &src);
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
  struct ip* resp_ip;
  struct icmp* resp_icmp;
  jficmp_open(buffer, &resp_ip, &resp_icmp);
  assert(resp_icmp->icmp_type == ICMP_ECHOREPLY);
  assert(resp_icmp->icmp_code == 0);
  assert(resp_icmp->icmp_id   == req->icmp_id);
  assert(resp_icmp->icmp_seq  == req->icmp_seq);

  /* Print. */
  printf("%ld bytes from %d.%d.%d.%d: icmp_seq=%d ttl=%d time=%.3f ms\n",
      recvbytes,
      buffer[12], buffer[13], buffer[14], buffer[15],
      ntohs(resp_icmp->icmp_seq), resp_ip->ip_ttl, ms);
}

int main(int argc, const char** argv) {

  /* Parse command line. */
  if (argc != 2) {
    err(EXIT_FAILURE, usage);
  }

  destname = argv[1];

  struct sockaddr_in dest;
  jf_resolve(destname, &dest);

  /* Construct socket. */
  struct icmp* req = jficmp_ctor(&sock, /*reserve=*/ICMP_MINLEN,
      /*type=*/ICMP_ECHO, /*code=*/0);

  jfsock_setttl(&sock, 255);

  /* Schedule reporting. */
  struct sigaction act;
  act.sa_handler = &onsig;
  if (-1 == sigaction(SIGINT, &act, /*oact=*/NULL)) {
    jfsock_dtor(&sock);
    err(errno, "could not schedule reporting");
  }

  /* Construct ICMP header. */
  req->icmp_id  = getpid();
  req->icmp_seq = 0;

  while (1) {
    ping(&dest);
    sleep(1);
    u16_t seq     = ntohs(req->icmp_seq) + 1;
    req->icmp_seq = htons(seq);
  }

  return EXIT_SUCCESS;
}

