/**
 *******************************************************************************
 * @file nec.c
 * @author Keidan
 * @date 03/01/2013
 * @par Project
 * nec
 *
 * @par Copyright
 * Copyright 2011-2013 Keidan, all right reserved
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
#include "nec_config.h"
#include <tk/sys/syssig.h>

static void nec_cleanup(void);
static void nec_signals(int sig);

void usage(int err) {
  fprintf(stdout, "usage: nec options\n");
  exit(err);
}


int main(int argc, char** argv) {
  fprintf(stdout, "Network config is a FREE software v%d.%d.\nCopyright 2011-2013 By kei\nLicense GPL.\n\n", NEC_VERSION_MAJOR, NEC_VERSION_MINOR);


  syssig_init(log_init_cast_user("nec", LOG_PID|LOG_CONS|LOG_PERROR), nec_cleanup);
  syssig_add_signal(SIGINT, nec_signals);
  syssig_add_signal(SIGTERM, nec_signals);

  return EXIT_SUCCESS;
}

static void nec_signals(int sig) {
  if(sig == SIGINT)
    printf("\n"); // for ^C
  exit(0);
}

static void nec_cleanup(void) {

}
