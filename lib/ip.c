#include <jfnet/ip.h>

u_short ip_cksum(void* hdr, size_t size) {
  u_short* hwords = (u_short*)hdr;
  /* Size comes in bytes (8-bits). We want half-words (16-bits). */
  assert(size % 2 == 0);
  size >>= 1;

  u_int sum = 0;
  while (size--) {
    sum += *hwords++;
  }

  sum = (sum & 0xFFFF) + (sum >> 16);
  sum += sum >> 16;
  return ~sum;
}

