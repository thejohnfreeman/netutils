#include <stdio.h>

int main(int argc, char** argv) {
  FILE* db = fopen("database/GeoLiteCity-Blocks.csv", "rb");

  fclose(db);
  return 0;
}

