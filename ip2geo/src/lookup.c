#include <stdlib.h> // EXIT_FAILURE

#include <jfnet/inet.h>

#include "lookup.h"
#include "compare.h"

int ip2geo(const char* host, FILE* blk_db, FILE* loc_db, struct csv* csv,
    float* lat, float* lng)
{
  union addr addr;
  int error = sscanf(host, "%hhu.%hhu.%hhu.%hhu",
      &addr.octs[0], &addr.octs[1], &addr.octs[2], &addr.octs[3]);
  if (error == EOF || error < 4) {
    fprintf(stderr, "error: not an IP address: %s\n", host);
    return EXIT_FAILURE;
  }
  addr.in_addr = ntohl(addr.in_addr);

  //printf("looking for %u\n", addr.in_addr);

  const char* row = csv_bsearch(csv, blk_db, &addr.in_addr, &comp_blk);
  if (!row) {
    return EXIT_FAILURE;
  }

  u_int id;
  error = sscanf(row, "\"%*u\",\"%*u\",\"%u\"", &id);
  if (error == EOF || error < 1) {
    fprintf(stderr, "abort: could not parse block id for %s\n", host);
    return EXIT_FAILURE;
  }

  //printf("block %u\n", id);

  row = csv_bsearch(csv, loc_db, &id, &comp_loc);
  if (!row) {
    fprintf(stderr, "abort: block %u not found in database\n", id);
    return EXIT_FAILURE;
  }

  //printf("found location data: %s", row);

  /* Can't use sscanf for this row because of parsing sensitivities. */
  /* Latitude and longitude are in fields 6 and 7, respectively, when the
   * first field is numbered 1. */
  const char* field;
  int i = 6;
  while (i--) {
    field = csv_next_field(csv);
  }
  *lat = atof(field);
  field = csv_next_field(csv);
  *lng = atof(field);

  return EXIT_SUCCESS;
}

