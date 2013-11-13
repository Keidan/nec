/**
 *******************************************************************************
 * @file nec_utils.c
 * @author Keidan
 * @date 13/11/2013
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
#include "nec_utils.h"

extern htable_t ifaces;
extern void usage(int err);

/**
 * @fn void usage_tun()
 * @brief Print the tun parts of the usage function.
 */
void usage_tun() {
  fprintf(stdout, "TUN/TAP Management:\n");
  fprintf(stdout, "  - Delete tun/tap iface (eg with tap vpn): nec tap del vpn\n");
  fprintf(stdout, "  - Create tun/tap iface (eg with tun vpn): nec tun add vpn\n");
  fprintf(stdout, "    After the create command you can use the base options\n");
}

/**
 * @fn void usage_base()
 * @brief Print the base parts of the usage function.
 */
void usage_base() {
  fprintf(stdout, "Base Management:\n");
  fprintf(stdout, "  - List up ifaces: nec\n");
  fprintf(stdout, "  - List all ifaces (down also): nec a or nec all\n");
  fprintf(stdout, "  - Up a specific iface (eg with eth0): nec eth0 up\n");
  fprintf(stdout, "  - Down a specific iface (eg with eth0): nec eth0 down\n");
  fprintf(stdout, "  - Update a specific iface (eg with eth0): nec eth0 192.168.0.10\n");
  fprintf(stdout, "  - Create iface alias (eg with eth0): nec eth0:1 192.168.0.10\n"); 
  fprintf(stdout, "    With this mode you can specify some attributes\n");
  fprintf(stdout, "    - broadcast addr\n");
  fprintf(stdout, "    - netmask addr\n");
  fprintf(stdout, "    - mac new_mac\n");
  fprintf(stdout, "    - mtu value\n");
  fprintf(stdout, "    - metric value\n");
  fprintf(stdout, "      eg with all attributes:\n");
  fprintf(stdout, "      nec eth0 192.168.0.10 broadcast 192.168.0.255 netmask 255.255.255.0 mac 00:10:01:10:01:00 mtu 1500 metric 1\n");
  fprintf(stdout, "    You can also set or unset some flags (prefixed with + to add and - to remove):\n");
  fprintf(stdout, "    - promisc, notrailers, debug, loopback, broadcast, multicast, master, slave, portsel, automedia, dynamic, running, pointopoint, noarp\n");
  fprintf(stdout, "      eg with some flags:\n");
  fprintf(stdout, "      nec eth0 +broadcast +multicast -debug\n");
}


/**
 * @fn _Bool parse_tun(int argc, char** argv)
 * @brief Parse the tun arguments.
 * @param argc Arguments count.
 * @param argv Arguments value.
 * @return 0 if the message is not aquired else 1.
 */
_Bool parse_tun(int argc, char** argv) {
  if(argc >= 2) {
    if(!strcmp(argv[1], "tun") || !strcmp(argv[1], "tap")) {
      if(argc < 3) {
	nlog("Invalid argument number for this mode\n");
	usage(EXIT_FAILURE);
      }
      int r;
      nettun_type_t type = !strcmp(argv[1], "tap") ? NETTUN_TAP : NETTUN_TUN; 
      if(!strcmp(argv[2], "del")) {
	struct nettun_s nt;
	strcpy(nt.name, argv[3]);
	nt.type = type;
	if((r = nettun_remove(&nt))) {
	  if(errno)
	    nlog("%s error: (%d) %s\n", argv[1], errno, strerror(errno));
	}
	return 1;
      } else if(!strcmp(argv[2], "add")) {
	struct nettun_s nt;
	strcpy(nt.name, argv[3]);
	nt.type = type;
	if((r = nettun_create(&nt))) {
	  if(errno)
	    nlog("%s error: (%d) %s\n", argv[1], errno, strerror(errno));
	}
	return 1;
      }
    }
  }
  return 0;
}


/**
 * @fn _Bool parse_base(int argc, char** argv)
 * @brief Parse the tun arguments.
 * @param argc Arguments count.
 * @param argv Arguments value.
 * @return 0 if the message is not aquired else 1.
 */
_Bool parse_base(int argc, char** argv) {
  int i, si = 1, count = 0;
  int mtu = INVALID_INT, metric = INVALID_INT;
  _Bool all = 0, up = 0, updown = 0;
  char **keys;
  netiface_ip4_t ip4, bcast, nmask;
  netiface_mac_t mac;
  _Bool first_iface = 0;
  mbool promisc = _nset, notrailers = _nset, debug = _nset, loopback = _nset, broadcast = _nset, multicast = _nset, master = _nset, slave = _nset, portsel = _nset, automedia = _nset, dynamic = _nset, running = _nset, pointopoint = _nset, noarp = _nset;

  struct netiface_info_s info;
  netiface_t iface;

  bzero(ip4, sizeof(netiface_ip4_t));
  bzero(bcast, sizeof(netiface_ip4_t));
  bzero(nmask, sizeof(netiface_ip4_t));
  bzero(mac, sizeof(netiface_mac_t));

  ifaces = netiface_list_new(NETIFACE_LVL_UDP, NETIFACE_KEY_NAME);

    /* chec if the args correspond to: app iface */ 
  if(argc >= 2) {
    if((iface = htable_lookup(ifaces, argv[1]))) {
      first_iface = 1;
      si = 2;
    } else if(string_indexof(argv[1], ":") != -1 && nettools_is_ipv4(argv[2])) {
      if(netiface_create(argv[1], argv[2])) 
	exit(EXIT_FAILURE);
      netiface_list_delete(ifaces);
      ifaces = netiface_list_new(NETIFACE_LVL_UDP, NETIFACE_KEY_NAME);
      if((iface = htable_lookup(ifaces, argv[1]))) {
	first_iface = 1;
	si = 2;
      }
    }
  }
  /* chec if the args correspond to: app iface ip */
  if(argc >= 3 && nettools_is_ipv4(argv[2])) {
    strcpy(ip4, argv[2]);
    si = 3;
  /* chec if the args correspond to: app iface up/dow */
  } else if(argc >= 3 && (!strcmp(argv[2], "up") || !strcmp(argv[2], "down"))) {
    up = (!strcmp(argv[2], "up"));
    si = 4;
    updown = 1;
    if(argc > 3) {
      nlog("Invalid up/down usage!\n");
      usage(EXIT_FAILURE);
    }
  }
  /* only if not up/down */
  if(!updown) {
    for(i = si; i < argc; i++) {
      if(!strlen(ip4) && !first_iface && (!strcmp(argv[i], "all"))) {
	all = 1;
	break;
      } else if(!strcmp(argv[i], "broadcast")) {
	i++;
	if(i == argc) {
	  nlog("Invalid broadcast usage (no broadcast)!\n");
	  usage(EXIT_FAILURE);	  
	} else if(!nettools_is_ipv4(argv[i])) {
	  nlog("Invalid broadcast usage (invalid broadcast)!\n");
	  usage(EXIT_FAILURE);	  
	}
	strcpy(bcast, argv[i]);
      } else if(!strcmp(argv[i], "netmask")) {
	i++;
	if(i == argc) {
	  nlog("Invalid netmask usage (no netmask)!\n");
	  usage(EXIT_FAILURE);	  
	} else if(!nettools_is_ipv4(argv[i])) {
	  nlog("Invalid netmask usage (invalid netmask)!\n");
	  usage(EXIT_FAILURE);	  
	}
	strcpy(nmask, argv[i]);	
      } else if(!strcmp(argv[i], "mac")) {
	i++;
	if(i == argc) {
	  nlog("Invalid MAC usage (no mac)!\n");
	  usage(EXIT_FAILURE);	  
	} else if(!nettools_valid_mac(argv[i])) {
	  nlog("Invalid MAC usage (invalid mac)!\n");
	  usage(EXIT_FAILURE);	  
	}
	strcpy(mac, argv[i]);
      } else if(!strcmp(argv[i], "mtu")) {
	i++;
	if(i == argc) {
	  nlog("Invalid MTU usage (no value)!\n");
	  usage(EXIT_FAILURE);	  
	}
	mtu = string_parse_int(argv[i], 0);
      } else if(!strcmp(argv[i], "metric")) {
	i++;
	if(i == argc) {
	  nlog("Invalid Metric usage (no value)!\n");
	  usage(EXIT_FAILURE);	  
	}
	metric = string_parse_int(argv[i], 0);
      } else if(strlen(argv[i]) > 2 && !strcmp(argv[i] + 1, "promisc")) {
	flag_param(promisc, argv[i]);
      } else if(strlen(argv[i]) > 2 && !strcmp(argv[i] + 1, "notrailers")) {
	flag_param(notrailers, argv[i]);
      } else if(strlen(argv[i]) > 2 && !strcmp(argv[i] + 1, "debug")) {
	flag_param(debug, argv[i]);
      } else if(strlen(argv[i]) > 2 && !strcmp(argv[i] + 1, "loopback")) {
	flag_param(loopback, argv[i]);
      } else if(strlen(argv[i]) > 2 && !strcmp(argv[i] + 1, "broadcast")) {
	flag_param(broadcast, argv[i]);
      } else if(strlen(argv[i]) > 2 && !strcmp(argv[i] + 1, "multicast")) {
	flag_param(multicast, argv[i]);
      } else if(strlen(argv[i]) > 2 && !strcmp(argv[i] + 1, "master")) {
	flag_param(master, argv[i]);
      } else if(strlen(argv[i]) > 2 && !strcmp(argv[i] + 1, "slave")) {
	flag_param(slave, argv[i]);
      } else if(strlen(argv[i]) > 2 && !strcmp(argv[i] + 1, "portsel")) {
	flag_param(portsel, argv[i]);
      } else if(strlen(argv[i]) > 2 && !strcmp(argv[i] + 1, "automedia")) {
	flag_param(automedia, argv[i]);
      } else if(strlen(argv[i]) > 2 && !strcmp(argv[i] + 1, "dynamic")) {
	flag_param(dynamic, argv[i]);
      } else if(strlen(argv[i]) > 2 && !strcmp(argv[i] + 1, "running")) {
	flag_param(running, argv[i]);
      } else if(strlen(argv[i]) > 2 && !strcmp(argv[i] + 1, "pointopoint")) {
	flag_param(pointopoint, argv[i]);
      } else if(strlen(argv[i]) > 2 && !strcmp(argv[i] + 1, "noarp")) {
	flag_param(noarp, argv[i]);
      }
      else {
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
    netiface_read(iface, &info);
    IFACE_SET_UP(info.flags, up);
    if(netiface_write(iface, &info) == -1)
      fprintf(stderr, "ERROR: Unable to change the device flags: (%d) %s\n", errno, strerror(errno));
    exit(EXIT_SUCCESS);
  }
  netiface_read(iface, &info);
  if(strlen(ip4)) strcpy(info.ip4, ip4);
  if(strlen(bcast)) strcpy(info.bcast, bcast);
  if(strlen(nmask)) strcpy(info.mask, nmask);
  if(strlen(mac)) strcpy(info.mac, mac);
  if(metric != INVALID_INT) info.metric = metric;
  if(mtu != INVALID_INT) info.mtu = mtu;
  if(promisc != _nset)
    IFACE_SET_PROMISC(info.flags, !!promisc);
  if(notrailers != _nset)
    IFACE_SET_NOTRAILERS(info.flags, !!notrailers);
  if(debug != _nset)
    IFACE_SET_DEBUG(info.flags, !!debug);
  if(loopback != _nset)
    IFACE_SET_LOOPBACK(info.flags, !!loopback);
  if(broadcast != _nset)
    IFACE_SET_BROADCAST(info.flags, !!broadcast);
  if(multicast != _nset)
    IFACE_SET_MULTICAST(info.flags, !!multicast);
  if(master != _nset)
    IFACE_SET_MASTER(info.flags, !!master);
  if(slave != _nset)
    IFACE_SET_SLAVE(info.flags, !!slave);
  if(portsel != _nset)
    IFACE_SET_PORTSEL(info.flags, !!portsel);
  if(automedia != _nset)
    IFACE_SET_AUTOMEDIA(info.flags, !!automedia);
  if(dynamic != _nset)
    IFACE_SET_DYNAMIC(info.flags, !!dynamic);
  if(running != _nset)
    IFACE_SET_RUNNING(info.flags, !!running);
  if(pointopoint != _nset)
    IFACE_SET_POINTOPOINT(info.flags, !!pointopoint);
  if(noarp != _nset)
    IFACE_SET_NOARP(info.flags, !!noarp);
  if(netiface_write(iface, &info) == -1) {
    fprintf(stderr, "ERROR: Unable to change the device configuration: (%d) %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
  return 1;
}