#include <string.h>
#include <stdlib.h> // exit
#include <errno.h>  // errno

#include "io.h"

#define SEEK_STRIDE 16

static long min(long a, long b) {
  return (a < b) ? a : b;
}

void frseekln(FILE* file) {
  char buffer[SEEK_STRIDE + 1];
  long pos;
  char* begin;

  while ((pos = ftell(file)) > 0) {
    long stride = min(pos, SEEK_STRIDE);
    fseek(file, -stride, SEEK_CUR);
    fread(buffer, sizeof(char)/*=1*/, stride, file);
    buffer[stride] = '\0';
    fseek(file, -stride, SEEK_CUR);
    if ((begin = strrchr(buffer, '\n'))) {
      fseek(file, (begin - buffer) + 1, SEEK_CUR);
      break;
    }
  }
}

long fseekln(FILE* file) {
  long nread = 0;

  int c;
  while ((c = fgetc(file)) != '\n' && c != EOF) {
    ++nread;
  }

  if (ferror(file)) {
    perror("fgetc");
    exit(errno);
  }

  return nread;
}

char* nextof(const char* haystack, const char* needles) {
  while (!strchr(needles, *haystack) && (*haystack != '\0') && ++haystack);
  return (char*)haystack;
}

