/**
 *******************************************************************************
 * @file nec.c
 * @author Keidan
 * @date 30/09/2013
 * @par Project
 * nec
 *
 * @par Copyright
 * Copyright 2011-2014 Keidan, all right reserved
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY.
 *
 * Licence summary : 
 *    You can modify and redistribute the sources code and binaries.
 *    You can send me the bug-fix
 *
 * Term of the licence in in the file licence.txt.
 *
 *******************************************************************************
 */
#include "nec_utils.h"
#include <tk/sys/syssig.h>
#include <unistd.h>

htable_t ifaces = NULL;
ping_t ping = NULL;
_Bool end_loop = 0;

static void nec_cleanup(void);
static void nec_signals(int sig);


void usage(int err) {
  if(err != EXIT_FAILURE)
    fprintf(stdout, "Network config is a FREE software v%d.%d.\nCopyright 2011-2013 By kei\nLicense GPL.\n\n", NEC_VERSION_MAJOR, NEC_VERSION_MINOR);
  fprintf(stdout, "usage: nec options\n");
  fprintf(stdout, "  - h or help: print this help\n");
  usage_base();
  usage_tun();
  usage_route();
  usage_ping();
  exit(err);
}


void manage_parser(result_et rs) {
  switch(rs) {
    case result_consume:
      exit(EXIT_SUCCESS);
      break;
    case result_block:
      while(!end_loop) sleep(1);
      exit(EXIT_SUCCESS);
      break;
    case result_next: break;
  }
}

int main(int argc, char** argv) {
  syssig_init(log_init_cast_user("nec", LOG_PID|LOG_CONS), nec_cleanup);
  syssig_add_signal(SIGINT, nec_signals);
  syssig_add_signal(SIGTERM, nec_signals);

  if(argc == 2 && (!strcmp(argv[1], "h") || !strcmp(argv[1], "help")))
     usage(EXIT_SUCCESS);
  printf("%d\n", __LINE__);
  manage_parser(parse_tun(argc, argv));
  printf("%d\n", __LINE__);
  manage_parser(parse_route(argc, argv));
  printf("%d\n", __LINE__);
  manage_parser(parse_ping(argc, argv));
  printf("%d\n", __LINE__);
  manage_parser(parse_base(argc, argv));
  printf("%d\n", __LINE__);

  return EXIT_FAILURE;
}

static void nec_signals(int sig) {
  if(sig == SIGINT)
    printf("\n"); // for ^C
  exit(0);
}

static void nec_cleanup(void) {
  end_loop = 1;
  if(ifaces)
    netiface_list_delete(ifaces), ifaces = NULL;
  if(ping) ping_delete(ping), ping = NULL;
}
