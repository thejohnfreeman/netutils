#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <float.h>  // FLT_EPSILON
#include <sysexits.h>
#include <stdlib.h> // atof
#include <err.h>    // err
#include <errno.h>  // errno

#include "io.h"
#include "csv.h"
#include "lookup.h"

const char* usage =
"ip2geo ip-address...";

int feq(float a, float b) {
  return (b - a) < (10.0 * FLT_EPSILON);
}

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

  float* locs = (float*)malloc(2 * (argc - 1) * sizeof(float));
  if (!locs) {
    err(errno, "abort: malloc");
  }

  size_t iloc = 0;
  for (size_t iarg = 1; iarg < argc; ++iarg) {
    if (ip2geo(argv[iarg], blk_db, loc_db, &csv,
          locs + iloc, locs + iloc + 1))
    {
      /* Error. */
      //printf("%s not found in database\n", argv[iarg]);
      continue;
    }

    /* Only remember this location if it's different from the last one. */
    if (iloc > 0 &&
        feq(locs[iloc], locs[iloc - 2]) &&
        feq(locs[iloc + 1], locs[iloc - 1]))
    {
      continue;
    }

    iloc += 2;
  }

  printf("http://maps.googleapis.com/maps/api/staticmap?"
      "size=600x600&sensor=false&path=");
  for (size_t i = 0; i < iloc; i += 2) {
    if (i > 0) {
      printf("|");
    }
    printf("%.6f,%.6f", locs[i], locs[i + 1]);
  }
  puts("");

  fclose(blk_db);
  fclose(loc_db);

  return 0;
}

