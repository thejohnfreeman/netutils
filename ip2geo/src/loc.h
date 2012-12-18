#ifndef IP_TO_GEO_LOC_H
#define IP_TO_GEO_LOC_H

struct loc {
  float lat;
  float lng;
};

int loc_equal(struct loc* a, struct loc* b);

#endif

