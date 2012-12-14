#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>
#include <stdlib.h> // atof
#include <err.h>    // err
#include <errno.h>  // errno

#include "io.h"
#include "csv.h"
#include "lookup.h"

int main(int argc, char** argv) {
  FILE* blk_db = fopen("database/GeoLiteCity-Blocks.csv", "rb");
  if (!blk_db) {
    err(errno, "error: cannot open database");
  }

  FILE* loc_db = fopen("database/GeoLiteCity-Location.csv", "rb");
  if (!loc_db) {
    err(errno, "error: cannot open database");
  }

  struct csv csv;
  csv_ctor(&csv);

  float lat, lng;
  if (ip2geo(argv[1], blk_db, loc_db, &csv, &lat, &lng)) {
    /* Error. */
  }
  printf("latitude: %.4f, longitude: %.4f\n", lat, lng);

  fclose(blk_db);
  fclose(loc_db);

  return 0;
}

