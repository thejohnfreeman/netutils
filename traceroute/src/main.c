#include <assert.h>    // assert
#include <stdio.h>     // printf
#include <stdlib.h>    // EXIT_SUCCESS

#include <jfnet/icmp.h>
#include <jfnet/inet.h>

#include "opts.h"
#include "path.h"
#include "ping.h"

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

  struct path path;
  path_ctor(&path, options.max_ttl, options.nprobes);

  int ttl = options.first_ttl;
  bool reached_dest;
  do {
    reached_dest = pinghop_icmp(&sock, &dest, ttl, &path);
    ++ttl;
  } while (!reached_dest && ttl < options.max_ttl);

  path_print(&path, /*nhops=*/ttl, stdout);

  return EXIT_SUCCESS;
}

