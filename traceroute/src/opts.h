#ifndef TRACEROUTE_OPTS_H
#define TRACEROUTE_OPTS_H

#include <stdbool.h>
#include <sys/types.h>

#include <jfnet/byteorder.h>

enum method {
  METHOD_ICMP,
  METHOD_TCP,
  METHOD_UDP,
  METHOD_UDP_PORT,
  METHOD_UDPLITE
};

struct options {
  enum method method;
  const char* src;
  const char* dest;
  int         port;
  int         first_ttl;
  int         max_ttl;
  int         nprobes;
  int         sprobes;
  float       sendwait;
  float       recvwait;
  bool        reverse_dns;
};

extern struct options options;

void parse_options(int argc, char** argv);

#endif

