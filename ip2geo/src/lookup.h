#ifndef IP_TO_GEO_LOOKUP_H
#define IP_TO_GEO_LOOKUP_H

#include <stdio.h>

#include "csv.h"
#include "loc.h"

int ip2geo(const char* host, FILE* blk_db, FILE* loc_db, struct csv* csv,
    struct loc* loc);

#endif

