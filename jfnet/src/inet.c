#include <assert.h> // assert
#include <err.h>    // errx
#include <string.h> // memcpy
#include <sysexits.h>
#include <netdb.h>

#include <jfnet/inet.h>

void jf_resolve(const char* hostname, struct sockaddr_in* addr) {
  struct addrinfo* res = NULL;
  int error = getaddrinfo(hostname, /*serv=*/NULL, /*hints=*/NULL, &res);
  if (error) {
    errx(EX_NOHOST, "could not parse hostname: %s", gai_strerror(error));
  }
  assert(res);
  memcpy(addr, res->ai_addr, sizeof(*addr));
  freeaddrinfo(res);
}

void jf_unresolve(const struct sockaddr_in* addr, char* hostname) {
  int error = getnameinfo((const struct sockaddr*)addr, sizeof(*addr),
      hostname, NI_MAXHOST + 1, /*serv=*/NULL, /*servlen=*/0, /*flags=*/0);
  if (error) {
    errx(EX_NOHOST, "could not get hostname: %s", gai_strerror(error));
  }
}

void jf_unresolve4(in_addr_t addr, char* hostname) {
  struct sockaddr_in wrapper;
  wrapper.sin_len         = sizeof(wrapper);
  wrapper.sin_family      = AF_INET;
  wrapper.sin_addr.s_addr = addr;
  jf_unresolve(&wrapper, hostname);
}

