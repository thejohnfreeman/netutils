#ifndef TRACEROUTE_OPTS_H
#define TRACEROUTE_OPTS_H

#include <stdbool.h>
#include <sys/types.h>

#include <jfnet/byteorder.h>

enum probe {
  PROBE_ICMP,
  PROBE_TCP,
  PROBE_UDP,
  PROBE_UDP_PORT,
  PROBE_UDPLITE
};

struct options {
  enum probe  probe;
  const char* src;
  const char* dest;
  int         port;
  int         first_ttl;
  int         max_ttl;
  int         nqueries;
  int         squeries;
  float       sendwait;
  float       recvwait;
  bool        reverse_dns;
};

extern struct options options;

void parse_options(int argc, char** argv);

#endif

