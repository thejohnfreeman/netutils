#ifndef IP_TO_GEO_IO_H
#define IP_TO_GEO_IO_H

#include <stdio.h>

void frseekln(FILE* file);
long fseekln(FILE* file);
char* nextof(const char* haystack, const char* needles);

#endif

