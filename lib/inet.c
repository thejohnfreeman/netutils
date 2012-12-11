#include <assert.h> // assert
#include <err.h>    // errx
#include <stdlib.h> // EXIT_FAILURE, NULL
#include <string.h> // memcpy
#include <netdb.h>

#include <jfnet/inet.h>

void jf_resolve(const char* hostname, struct sockaddr_in* addr) {
  struct addrinfo* res = NULL;
  int error = getaddrinfo(hostname, /*servname=*/NULL, /*hints=*/NULL, &res);
  if (error) {
    errx(EXIT_FAILURE, "could not parse address: %s", gai_strerror(error));
  }
  assert(res);
  memcpy(addr, res->ai_addr, sizeof(*addr));
  freeaddrinfo(res);
}

