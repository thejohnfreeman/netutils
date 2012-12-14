#ifndef JFNET_INET_H
#define JFNET_INET_H

#include <netinet/in.h>

#include <jfnet/byteorder.h>

union addr {
  u8_t      octs[4];
  in_addr_t in_addr;
};

void jf_resolve(const char* hostname, struct sockaddr_in* addr);
void jf_unresolve(const struct sockaddr_in* addr, char* hostname);
void jf_unresolve4(in_addr_t addr, char* hostname);

#endif

