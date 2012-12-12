#ifndef JFNET_INET_H
#define JFNET_INET_H

#include <netinet/in.h>

void jf_resolve(const char* hostname, struct sockaddr_in* addr);
void jf_unresolve(const struct sockaddr_in* addr, char* hostname);

#endif

