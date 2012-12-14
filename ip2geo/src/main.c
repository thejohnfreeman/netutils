#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <err.h>    // err
#include <errno.h>  // errno

#include "io.h"
#include "csv.h"

int main(int argc, char** argv) {
  FILE* db = fopen("database/GeoLiteCity-Blocks.csv", "rb");
  if (!db) {
    err(errno, "error: cannot open database");
  }

  int error = fseek(db, 0, SEEK_END);
  if (error) {
    err(errno, "abort: fseek with SEEK_END");
  }

  long fsize = ftell(db);
  if (fsize < 0) {
    err(errno, "abort: ftell for file size");
  }

  printf("database size: %ld\n", fsize);

  struct csv csv;
  csv_ctor(&csv);

  long pos = fsize >> 2;
  while (1) {
    error = fseek(db, pos, SEEK_SET);
    if (error) {
      err(errno, "abort: fseek with SEEK_SET");
    }

    frseekln(db);
    csv_next_row(&csv, db);
    const char* lows = csv_next_field(&csv);
    assert(lows && isdigit(*lows));
    const char* highs = csv_next_field(&csv);
    assert(highs && isdigit(*highs));
    break;
  }

  fclose(db);
  return 0;
}

