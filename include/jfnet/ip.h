#ifndef JFNET_IP_H
#define JFNET_IP_H

#include <assert.h>
#include <sys/types.h>

u_short ip_cksum(void* hdr, size_t size);

#endif

