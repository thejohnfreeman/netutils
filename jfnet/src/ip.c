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

void fixip_osx(struct ip* ip) {
  /* Something on my Mac subtracts the header length from `ip_len` and stores
   * it in host order (little endian). */
  u16_t ip_hdrlen = ip->ip_hl << 2;
  u16_t ip_totlen = ip->ip_len + ip_hdrlen;
  ip->ip_len = htons(ip_totlen);
  /* Also reverses the byte order of the fragment offset / flags field. */
  ip->ip_off = htons(ip->ip_off);
}

