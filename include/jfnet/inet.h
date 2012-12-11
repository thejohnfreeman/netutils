#ifndef JFNET_INET_H
#define JFNET_INET_H

#include <netinet/in.h>

void jf_resolve(const char* hostname, struct sockaddr_in* addr);

#endif

