#ifndef IP_TO_GEO_ARGS_H
#define IP_TO_GEO_ARGS_H

struct args_vtab {
};

struct args_cli {
  struct args_vtab vtab;
  int argc;
  char** argv;
};

struct args_stdin {
  struct args_vtab vtab;
};

#endif

