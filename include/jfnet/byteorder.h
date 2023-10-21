#ifndef JFNET_BYTE_ORDER_H
#define JFNET_BYTE_ORDER_H

#include <endian.h>

typedef unsigned char  u8_t;
typedef unsigned short u16_t;
typedef unsigned int   u32_t;

/* Assumptions. */
_Static_assert((u8_t)(1 << 8) == 0, "expected 1-byte u8_t");
_Static_assert(sizeof(u16_t) == 2, "expected 2-byte u16_t");
_Static_assert(sizeof(u32_t) == 4, "expected 4-byte u32_t");

#endif

