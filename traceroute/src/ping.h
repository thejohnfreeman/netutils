#ifndef TRACEROUTE_PING_H
#define TRACEROUTE_PING_H

#include <stdbool.h>

#include <jfnet/sock.h>

#include "path.h"

#define MAX_PACKET_SIZE 64

bool pinghop_icmp(struct jfsock* sock, struct sockaddr_in* dest, int ttl,
    struct path* path);

#endif

