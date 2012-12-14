#ifndef IP_TO_GEO_CSV_H
#define IP_TO_GEO_CSV_H

#include <stdio.h>
#include <sys/types.h>

/* Something akin to an iterator for CSV files. */

struct csv {
  char*  buffer;
  size_t size;
  char*  it;
};

void csv_ctor(struct csv* csv);
const char* csv_next_row(struct csv* csv, FILE* file);
/* Returns a null-terminated, unquoted string. */
const char* csv_next_field(struct csv* csv);
const char* csv_bsearch(struct csv* csv, FILE* file, const void* key,
    int (*comp)(const void* key, const char* row));
void csv_dtor(struct csv* csv);

#endif

