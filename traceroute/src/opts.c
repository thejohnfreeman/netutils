#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>    // exit, EXIT_FAILURE
#include <unistd.h>    // getpid

#include "opts.h"

struct options options = {
  /*probe=*/      PROBE_ICMP,
  /*src=*/        NULL,
  /*dest=*/       NULL,
  /*port=*/       0,
  /*first_ttl=*/  1,
  /*max_ttl=*/    30,
  /*nqueries=*/   3,
  /*squeries=*/   16,
  /*sendwait=*/   0.0,
  /*recvwait=*/   5.0,
  /*reverse_dns=*/true
};

static struct option long_opts[] = {
  { "help", no_argument, 0, 'h' },
  { 0, 0, 0, 0 }
};

const char* short_opts = "f:hIm:nN:p:q:s:TUw:z:";

const char* usage =
"usage: traceroute host\n";

void option_assert(char** argv, bool expr, const char* msg) {
  if (!expr) {
    fprintf(stderr, "%s: %s\n", argv[0], msg);
    exit(EXIT_FAILURE);
  }
}

#define OPTION_ASSERT(expr, msg) option_assert(argv, expr, msg)

void parse_options(int argc, char** argv) {
  /* Non-constant defaults. */
  options.port = getpid();

  /* Dash options. */
  int c;
  int long_i;

  while (-1 != (c = getopt_long(argc, argv, short_opts,
          long_opts, &long_i)))
  {

    switch (c) {

      case 'h':
        fputs(usage, stdout);
        exit(EXIT_SUCCESS);

      case 'I':
        options.probe = PROBE_ICMP;
        options.port  = getpid();
        break;

      case 'T':
        options.probe = PROBE_TCP;
        options.port  = 80;
        break;

      case 'U':
        options.probe = PROBE_UDP_PORT;
        options.port  = 53;
        break;

      case 's':
        options.src = optarg;
        break;

      case 'p':
        options.port = atoi(optarg);
        OPTION_ASSERT(options.port > 0,
            "port must be > 0");
        break;

      case 'f':
        options.first_ttl = atoi(optarg);
        OPTION_ASSERT(options.first_ttl > 0,
            "first ttl must be > 0");
        break;

      case 'm':
        options.max_ttl = atoi(optarg);
        OPTION_ASSERT(options.max_ttl > 0,
            "max ttl must be > 0");
        OPTION_ASSERT(options.max_ttl <= 255,
            "max ttl must be <= 255");
        break;

      case 'q':
        options.nqueries = atoi(optarg);
        OPTION_ASSERT(options.nqueries > 0,
            "nqueries must be > 0");
        break;

      case 'N':
        options.squeries = atoi(optarg);
        OPTION_ASSERT(options.squeries > 0,
            "squeries must be > 0");
        break;

      case 'z':
        options.sendwait = atof(optarg);
        if (options.sendwait > 10.0) {
          options.sendwait /= 1000.0;
        }
        break;

      case 'w':
        options.recvwait = atof(optarg);
        break;

      case 'n':
        options.reverse_dns = false;
    }

  }

  /* Position options. */
  if ((argc - optind) < 1) {
    fputs(usage, stderr);
    exit(EXIT_FAILURE);
  }

  options.dest = argv[optind];
}

