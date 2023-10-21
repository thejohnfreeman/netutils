#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>
#include <stdlib.h> // exit
#include <err.h>    // err
#include <errno.h>  // errno

#include "io.h"
#include "csv.h"
#include "lookup.h"

const char* usage =
"usage: ip2geo ip-address...";

int main(int argc, char** argv) {
  if (argc < 2) {
    fputs(usage, stderr);
    fputs("\n", stderr);
    exit(EX_USAGE);
  }

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

  struct loc* locs = (struct loc*)malloc((argc - 1) * sizeof(struct loc));
  if (!locs) {
    err(errno, "abort: malloc");
  }

  size_t iloc = 0;
  for (size_t iarg = 1; iarg < argc; ++iarg) {
    if (ip2geo(argv[iarg], blk_db, loc_db, &csv, locs + iloc)) {
      /* Error. */
      //printf("%s not found in database\n", argv[iarg]);
      continue;
    }

    /* Only remember this location if it's different from the last one. */
    if (iloc > 0 && loc_equal(locs + iloc, locs + iloc - 1)) {
      //puts("skipping consecutive duplicate location");
      continue;
    }

    ++iloc;
  }

  //printf("found path with %lu hops\n", iloc);

  printf("http://maps.googleapis.com/maps/api/staticmap?"
      "size=600x600&sensor=false&path=");
  for (size_t i = 0; i < iloc; ++i) {
    if (i > 0) {
      printf("|");
    }
    printf("%.6f,%.6f", locs[i].lat, locs[i].lng);
  }
  puts("");

  fclose(blk_db);
  fclose(loc_db);

  return 0;
}

