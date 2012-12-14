#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>
#include <err.h>    // err
#include <errno.h>  // errno

#include "io.h"
#include "csv.h"

#include <jfnet/inet.h>

int comp_block(const void* key, const char* row) {
  /* Parse row. */
  union addr first, last;
  int error = sscanf(row, "\"%u\",\"%u\"",
      &first.in_addr, &last.in_addr);
  if (error == EOF || error < 2) {
    /* Assume we landed in the header. */
    return 1;
  }

  /* Cast key. */
  in_addr_t addr = *(in_addr_t*)key;

  /* Branch. */
  if (addr < first.in_addr) {
    return -1;
  } 
  if (addr > last.in_addr) {
    return 1;
  }
  return 0;
}

int main(int argc, char** argv) {
  union addr addr;
  int error = sscanf(argv[1], "%hhu.%hhu.%hhu.%hhu",
      &addr.octs[0], &addr.octs[1], &addr.octs[2], &addr.octs[3]);
  if (error == EOF || error < 4) {
    errx(EX_USAGE, "error: not an IP address: %s\n", argv[1]);
  }
  addr.in_addr = ntohl(addr.in_addr);

  printf("looking for %u\n", addr.in_addr);

  FILE* db = fopen("database/GeoLiteCity-Blocks.csv", "rb");
  if (!db) {
    err(errno, "error: cannot open database");
  }

  struct csv csv;
  csv_ctor(&csv);

  const char* row = csv_bsearch(&csv, db, &addr.in_addr, &comp_block);
  u_int id;
  error = sscanf(row, "\"%*u\",\"%*u\",\"%u\"", &id);

  printf("block %u\n", id);

  fclose(db);
  return 0;
}

