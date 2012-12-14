#include <stdlib.h>     // malloc, realloc, free
#include <string.h>     // memset
#include <unistd.h>     // close
#include <err.h>        // err
#include <errno.h>      // errno
#include <sys/socket.h>

#include <jfnet/sock.h>

void jfsock_ctor(struct jfsock* sock, int type, int proto, size_t reserve) {
  memset(sock, 0, sizeof(*sock));

  sock->fd = socket(AF_INET, type, proto);
  if (-1 == sock->fd) {
    err(errno, "could not acquire socket");
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;

  if (-1 == bind(sock->fd, (struct sockaddr*)&addr, sizeof(addr))) {
    jfsock_dtor(sock);
    err(errno, "could not bind socket");
  }

  sock->buffer = (u8_t*)malloc(reserve);
  if (!sock->buffer) {
    jfsock_dtor(sock);
    err(errno, "could not allocate packet");
  }

  memset(sock->buffer, 0, reserve);
  sock->capacity = reserve;
  sock->length   = reserve;
}

void jfsock_setttl(struct jfsock* sock, int ttl) {
  if (-1 == setsockopt(sock->fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl))) {
    jfsock_dtor(sock);
    err(errno, "could not set time-to-live");
  }
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
  sock->buffer = NULL;
}

