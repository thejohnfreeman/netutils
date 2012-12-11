#include <assert.h>
#include <stdio.h>
#include <sys/errno.h>

#include <jfnet/icmp.h>
#include <jfnet/ip.h>
#include <jfnet/io.h>
#include <jfnet/byteorder.h>

struct icmp* jficmp_ctor(struct jfsock* sock, size_t reserve,
    u8_t type, u8_t code)
{
  jfsock_ctor(sock, SOCK_DGRAM, IPPROTO_ICMP, reserve);
  struct icmp* hdr = (struct icmp*)sock->buffer;
  hdr->icmp_type = type;
  hdr->icmp_code = code;
  return hdr;
}

ssize_t jficmp_send(struct jfsock* sock, const struct sockaddr_in* dest) {
  struct icmp* hdr = (struct icmp*)sock->buffer;
  hdr->icmp_cksum = 0;
  hdr->icmp_cksum = ip_cksum(hdr, sock->length);
  ssize_t sendbytes = sendto(sock->fd, hdr, sock->length, /*flags=*/0,
      (struct sockaddr*)dest, sizeof(*dest));
  if (-1 == sendbytes) {
    perror("could not send ICMP message");
    jfsock_dtor(sock);
    exit(errno);
  }
  return sendbytes;
}

ssize_t jficmp_recv(struct jfsock* sock, void* buffer, size_t size,
    struct sockaddr_in* src)
{
  socklen_t src_len = sizeof(*src);
  ssize_t recvbytes = recvfrom(sock->fd, buffer, size, /*flags=*/0,
      (struct sockaddr*)src, &src_len);
  if (-1 == recvbytes) {
    perror("could not read response");
    jfsock_dtor(sock);
    exit(errno);
  }
  return recvbytes;
}

void jficmp_open(void* buffer, struct ip** oip, struct icmp** oicmp) {
  struct ip* ip   = (struct ip*)buffer;
  u16_t ip_hdrlen = ip->ip_hl << 2;
  /* It appears Mac OS changes `ip_len` to just the data length in host
   * representation. */
  u16_t ip_totlen = ip->ip_len + ip_hdrlen;
  ip->ip_len = htons(ip_totlen);
  /* Mac OS fucks with the data too much to get a correct checksum. */
  //assert(0 == ip_cksum(ip, ip_hdrlen));
  *oip = ip;

  struct icmp* icmp = (struct icmp*)((u8_t*)buffer + ip_hdrlen);
  u16_t icmp_len    = ip_totlen - ip_hdrlen;
  //assert(0 == ip_cksum(icmp, icmp_len));
  *oicmp = icmp;
}

