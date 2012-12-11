#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/errno.h>

#include <jfnet/sock.h>

void jfsock_ctor(struct jfsock* sock, int type, int proto, size_t reserve) {
  memset(sock, 0, sizeof(*sock));

  sock->fd = socket(AF_INET, type, proto);
  if (-1 == sock->fd) {
    perror("could not acquire socket");
    exit(errno);
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;

  if (-1 == bind(sock->fd, (struct sockaddr*)&addr, sizeof(addr))) {
    perror("could not bind socket");
    jfsock_dtor(sock);
    exit(errno);
  }

  sock->buffer = (u8_t*)malloc(reserve);
  if (!sock->buffer) {
    perror("could not allocate packet");
    jfsock_dtor(sock);
    exit(errno);
  }

  memset(sock->buffer, 0, reserve);
  sock->capacity = reserve;
  sock->length   = reserve;
}

void jfsock_resize(struct jfsock* sock, size_t length) {
  if (length > sock->capacity) {
    sock->buffer   = (u8_t*)realloc(sock->buffer, length);
    sock->capacity = length;
  }
  sock->length = length;
}

void jfsock_dtor(struct jfsock* sock) {
  if (sock->fd) close(sock->fd);
  free(sock->buffer);
}

