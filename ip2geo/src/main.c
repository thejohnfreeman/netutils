#include <stdio.h>
#include <string.h>
#include <stdlib.h> // exit
#include <errno.h>  // errno

#include "io.h"
#include "csv.h"

int main(int argc, char** argv) {
  FILE* db = fopen("database/GeoLiteCity-Blocks.csv", "rb");
  if (!db) {
    fputs("error: database missing\n", stderr);
    exit(EXIT_FAILURE);
  }

  int error = fseek(db, 0, SEEK_END);
  if (error) {
    perror("SEEK_END unavailable");
    exit(errno);
  }

  long fsize = ftell(db);
  if (fsize < 0) {
    perror("could not determine file size");
    exit(errno);
  }

  printf("database size: %ld\n", fsize);

  long mid = fsize >> 2;
  error = fseek(db, mid, SEEK_SET);
  if (error) {
    perror("fseek");
    exit(errno);
  }

  frseekln(db);

  struct csv csv;
  csv_ctor(&csv);

  for (int i = 0; i < 3; ++i) {
    csv_next_row(&csv, db);
    const char* field;
    while ((field = csv_next_field(&csv))) {
      printf("field [%2ld] = |%s|\n", strlen(field), field);
    }
  }

  fclose(db);
  return 0;
}

