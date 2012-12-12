#ifndef JFNET_IP_H
#define JFNET_IP_H

#include <stdlib.h>
#include <netinet/ip.h>

#include <jfnet/byteorder.h>

u16_t ip_cksum(void* hdr, size_t size);
void fixip_osx(struct ip* ip);

#endif

