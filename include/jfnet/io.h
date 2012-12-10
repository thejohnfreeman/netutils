#ifndef JFNET_IO_H
#define JFNET_IO_H

#include <stdio.h>
#include <stdlib.h>

#include <jfnet/byteorder.h>

enum format_t {
  FORMAT_BINARY,
  FORMAT_HEX
};

void fprintu8blks(FILE* out, enum format_t format, u8_t* bytes, size_t size,
    size_t block_size);
void fprintu8s(FILE* out, u8_t* bytes, size_t size);
void fprintu8(FILE* out, u8_t byte);
void printu8blks(u8_t* bytes, enum format_t format, size_t size,
    size_t block_size);
void printu8s(u8_t* bytes, size_t size);
void printu8(u8_t byte);

void fprintu16(FILE* out, u16_t hword);
void printu16(u16_t hword);

void fprintu32(FILE* out, u32_t word);
void printu32(u32_t word);

#endif

