#include <float.h>  // FLT_EPSILON
#include <math.h>   // fabs

#include "loc.h"

int feq(float a, float b) {
  return fabs(b - a) < (10.0 * FLT_EPSILON);
}

int loc_equal(struct loc* a, struct loc* b) {
  return feq(a->lat, b->lat) && feq(a->lng, b->lng);
}

