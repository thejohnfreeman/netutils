#include <jfnet/io.h>

const char* u4str[] = {
  "0000",
  "0001",
  "0010",
  "0011",
  "0100",
  "0101",
  "0110",
  "0111",
  "1000",
  "1001",
  "1010",
  "1011",
  "1100",
  "1101",
  "1110",
  "1111"
};

void fprintu8blks(FILE* out, enum format_t format, u8_t* bytes, size_t size,
    size_t block_size)
{
  for (size_t i = 0; i < size; ++i) {
    if (i > 0) {
      if ((block_size > 0) && (i % block_size == 0)) {
        fprintf(out, "\n");
      } else {
        fprintf(out, " ");
      }
    }
    if (format == FORMAT_BINARY) {
      fprintf(out, "%s %s", u4str[bytes[i] >> 4], u4str[bytes[i] & 0xF]);
    } else {
      fprintf(out, "%02x", bytes[i]);
    }
  }
  fflush(out);
}

void fprintu8s(FILE* out, u8_t* bytes, size_t size) {
  fprintu8blks(out, FORMAT_BINARY, bytes, size, 0);
}

void fprintu8(FILE* out, u8_t byte) {
  fprintu8s(out, &byte, sizeof(u8_t));
}

void printu8blks(enum format_t format, u8_t* bytes, size_t size,
    size_t block_size)
{
  fprintu8blks(stdout, format, bytes, size, block_size);
}

void printu8s(u8_t* bytes, size_t size) {
  fprintu8s(stdout, bytes, size);
}

void printu8(u8_t byte) {
  fprintu8(stdout, byte);
}

void fprintu16(FILE* out, u16_t hword) {
  fprintu8s(out, (u8_t*)&hword, sizeof(u16_t));
}

void printu16(u16_t hword) {
  fprintu16(stdout, hword);
}

void fprintu32(FILE* out, u32_t word) {
  fprintu8s(out, (u8_t*)&word, sizeof(u32_t));
}

void printu32(u32_t word) {
  fprintu32(stdout, word);
}

