#include <stdio.h>

#include <jfnet/inet.h>

#include "compare.h"

int comp_blk(const void* key, const char* row) {
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

int comp_loc(const void* key, const char* row) {
  /* Parse row. */
  u_int rowid;
  int error = sscanf(row, "%u", &rowid);
  if (error == EOF || error < 1) {
    /* Assume we landed in the header. */
    return 1;
  }

  /* Cast key. */
  u_int keyid = *(u_int*)key;

  /* Branch. */
  if (keyid < rowid) {
    return -1;
  } 
  if (keyid > rowid) {
    return 1;
  }
  return 0;
}

