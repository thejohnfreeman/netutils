#include <string.h>
#include <stdlib.h> // exit
#include <errno.h>  // errno

#include "io.h"

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

