#ifndef JFNET_ICMP_H
#define JFNET_ICMP_H

#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include <jfnet/sock.h>

struct icmp* jficmp_ctor(struct jfsock* sock, size_t reserve,
    u8_t type, u8_t code);
ssize_t jficmp_send(struct jfsock* sock, const struct sockaddr_in* dest);
ssize_t jficmp_recv(struct jfsock* sock, void* buffer, size_t size,
    struct sockaddr_in* src);
void jficmp_open(void* buffer, struct ip** oip, struct icmp** oicmp);

#endif

