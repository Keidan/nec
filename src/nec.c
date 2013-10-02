/**
 *******************************************************************************
 * @file nec.c
 * @author Keidan
 * @date 30/09/2013
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tk/sys/syssig.h>
#include <tk/io/net/netiface.h>
#include <tk/io/net/nettools.h>

#define nlog(...) ({				\
    fprintf(stderr, __VA_ARGS__);		\
    logger(LOG_ERR, __VA_ARGS__);		\
  })

htable_t ifaces = NULL;

static void nec_cleanup(void);
static void nec_signals(int sig);

void usage(int err) {
  if(err != EXIT_FAILURE)
    fprintf(stdout, "Network config is a FREE software v%d.%d.\nCopyright 2011-2013 By kei\nLicense GPL.\n\n", NEC_VERSION_MAJOR, NEC_VERSION_MINOR);
  fprintf(stdout, "usage: nec options\n");
  exit(err);
}


int main(int argc, char** argv) {
  syssig_init(log_init_cast_user("nec", LOG_PID|LOG_CONS), nec_cleanup);
  syssig_add_signal(SIGINT, nec_signals);
  syssig_add_signal(SIGTERM, nec_signals);

  int i, si = 1, count;
  _Bool all = 0, up = 0, updown = 0;
  char **keys;
  netiface_ip4_t ip4;
  _Bool first_iface = 0;

  struct netiface_info_s info;
  netiface_t iface;

  bzero(ip4, sizeof(netiface_ip4_t));

  ifaces = netiface_list_new(NETIFACE_LVL_UDP, NETIFACE_KEY_NAME);


  if(argc >= 2 && (iface = htable_lookup(ifaces, argv[1]))) {
     first_iface = 1;
     si = 2;
  }
  if(argc >= 3 && nettools_is_ipv4(argv[2])) {
    strcpy(ip4, argv[2]);
    si = 3;
  } else if(argc >= 3 && (!strcmp(argv[2], "up") || !strcmp(argv[2], "down"))) {
    up = (!strcmp(argv[2], "up"));
    si = 4;
    updown = 1;
    if(argc > 3) {
      nlog("Invalid up/down usage!\n");
      usage(EXIT_FAILURE);
    }
  }
  if(!updown) {
    for(i = si; i < argc; i++) {
      
      if(!strlen(ip4) && !first_iface && (!strcmp(argv[i], "all"))) {
	all = 1;
	break;
      } else if(!strcmp(argv[i], "broadcast")) {
	i++;
	if(i == argc) {
	  nlog("Invalid broadcast usage (no ip)!\n");
	  usage(EXIT_FAILURE);	  
	} else if(!nettools_is_ipv4(argv[i])) {
	  nlog("Invalid broadcast usage (invalid ip v4)!\n");
	  usage(EXIT_FAILURE);	  
	}
	all = 1;
	break;
      } else if(!strcmp(argv[i], "netmask")) {
	i++;
	if(i == argc) {
	  nlog("Invalid broadcast usage (no ip)!\n");
	  usage(EXIT_FAILURE);	  
	} else if(!nettools_is_ipv4(argv[i])) {
	  nlog("Invalid broadcast usage (invalid ip v4)!\n");
	  usage(EXIT_FAILURE);	  
	}
	all = 1;
	break;
      } else {
	nlog("Unknown option: '%s'!\n", argv[i]);
	usage(EXIT_FAILURE);
      }
    }
  }
  argc--; /* remove pp name */
  if(!argc || (argc == 1 && all)) {
    count = htable_get_keys(ifaces, &keys);
    for(i = 0; i < count; i++) {
      iface = netiface_list_get(ifaces, keys[i]);
      netiface_read(iface, &info);
      netiface_print(stdout, &info, !all);
    }
    exit(EXIT_SUCCESS);
  } else if(argc == 1 && iface) {
    netiface_read(iface, &info);
    netiface_print(stdout, &info, 0);
    exit(EXIT_SUCCESS);
  } else if(!first_iface) {
    nlog("Invalide iface name '%s'\n", argv[1]);
    exit(EXIT_FAILURE);
  }
  if(updown) {
    printf("updown, up:%d\n", up);
    exit(EXIT_SUCCESS);
  }
  return EXIT_SUCCESS;
}

static void nec_signals(int sig) {
  if(sig == SIGINT)
    printf("\n"); // for ^C
  exit(0);
}

static void nec_cleanup(void) {
  if(ifaces)
    netiface_list_delete(ifaces), ifaces = NULL;
}
