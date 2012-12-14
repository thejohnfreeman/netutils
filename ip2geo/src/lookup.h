#ifndef IP_TO_GEO_LOOKUP_H
#define IP_TO_GEO_LOOKUP_H

#include <stdio.h>

#include "csv.h"

int ip2geo(const char* host, FILE* blk_db, FILE* loc_db, struct csv* csv,
    float* lat, float* lng);

#endif

