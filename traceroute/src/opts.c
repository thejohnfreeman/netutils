#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>    // exit, atoi, atof
#include <unistd.h>    // getpid
#include <sysexits.h>
#include <err.h>

#include "opts.h"

struct options options = {
  /*method=*/     METHOD_ICMP,
  /*src=*/        NULL,
  /*dest=*/       NULL,
  /*port=*/       0,
  /*first_ttl=*/  1,
  /*max_ttl=*/    30,
  /*nprobes=*/    3,
  /*sprobes=*/    16,
  /*sendwait=*/   0.0,
  /*recvwait=*/   5.0,
  /*reverse_dns=*/true,
  /*format=*/     FORMAT_DEFAULT
};

static struct option long_opts[] = {
  { "help", no_argument, 0, 'h' },
  { 0, 0, 0, 0 }
};

const char* short_opts = "1f:hIm:nN:p:q:s:TUw:z:";

const char* usage =
"usage: traceroute host\n";

void option_assert(char** argv, bool expr, const char* msg) {
  if (!expr) {
    err(EX_USAGE, "%s: %s\n", argv[0], msg);
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
        options.method = METHOD_ICMP;
        options.port   = getpid();
        break;

      case 'T':
        options.method = METHOD_TCP;
        options.port   = 80;
        break;

      case 'U':
        options.method = METHOD_UDP_PORT;
        options.port   = 53;
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
        if (options.format == FORMAT_ONE_PER_LINE) {
          break;
        }
        options.nprobes = atoi(optarg);
        OPTION_ASSERT(options.nprobes > 0,
            "nprobes must be > 0");
        break;

      case 'N':
        options.sprobes = atoi(optarg);
        OPTION_ASSERT(options.sprobes > 0,
            "sprobes must be > 0");
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
        break;

      case '1':
        options.format  = FORMAT_ONE_PER_LINE;
        options.nprobes = 1;
        break;

      default:
        errx(EX_USAGE, "unrecognized option");
    }

  }

  /* Position options. */
  if ((argc - optind) < 1) {
    fputs(usage, stderr);
    exit(EX_USAGE);
  }

  options.dest = argv[optind];
}

