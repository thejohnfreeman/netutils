#include <assert.h>
#include <sys/types.h>

#include <jfnet/ip.h>

u16_t ip_cksum(void* hdr, size_t size) {
  u16_t* hwords = (u16_t*)hdr;
  /* Size comes in bytes (8-bits). We want half-words (16-bits). */
  assert(size % 2 == 0);
  size >>= 1;

  u_int sum = 0;
  while (size--) {
    sum += *hwords++;
  }

  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  sum = ~sum;
  return (u16_t)sum;
}

