#ifndef JFNET_SOCK_H
#define JFNET_SOCK_H

#include <sys/types.h>
#include <netinet/in.h>

#include <jfnet/byteorder.h>

struct jfsock {
  int fd;
  u8_t* buffer;
  size_t capacity;
  size_t length;
};

void jfsock_ctor(struct jfsock* sock, int type, int proto, size_t reserve);
void jfsock_resize(struct jfsock* sock, size_t length);
void jfsock_dtor(struct jfsock* sock);

#endif

