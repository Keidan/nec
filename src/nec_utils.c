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
#include <tk/utils/llist_iter.h>
#include <unistd.h>

extern htable_t ifaces;
extern void usage(int err);
extern ping_t ping;
extern _Bool end_loop;
static uint32_t ping_max_count = 0;
static uint32_t ping_count = 1;

/**
 * @fn void usage_ping()
 * @brief Print the ping parts of the usage function.
 */
void usage_ping() {
  fprintf(stdout, "Ping Management:\n");
  fprintf(stdout, "  - Ping an address from a specific device.\n");
  fprintf(stdout, "  Possible uses:\n");
  fprintf(stdout, "    nec ping <iface> <address>\n");
  fprintf(stdout, "    nec ping <iface> <address> <count>\n");
  fprintf(stdout, "    nec ping <iface> <address> <count> <timeout_in_sec>\n");
  fprintf(stdout, "    Note:\n");
  fprintf(stdout, "    if count = 0, infinite ping.\n");
  fprintf(stdout, "    Default timeout value = 15 sec.\n");
} 

/**
 * @fn void usage_route()
 * @brief Print the route parts of the usage function.
 */
void usage_route() {
  fprintf(stdout, "Route Management:\n");
  fprintf(stdout, "  - List all routes: nec route\n");
  fprintf(stdout, "  - Delete route: nec route del [options]\n");
  fprintf(stdout, "  - Create route: nec route add [options]\n");
  fprintf(stdout, "  - Possible route options:\n");
  fprintf(stdout, "    - gw <addr>: Gateway address.\n");
  fprintf(stdout, "    - dst <addr>: Destination address.\n");
  fprintf(stdout, "    - mask <addr>: Destination mask address.\n");
  fprintf(stdout, "    - src <iface_name>: Source interface name..\n");
}

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
 * @fn _Bool parse_route(int argc, char** argv)
 * @brief Parse the route arguments.
 * @param argc Arguments count.
 * @param argv Arguments value.
 * @return 0 if the message is not aquired else 1.
 */
_Bool parse_route(int argc, char** argv) {
  if(argc >= 2) {
    if(!strcmp(argv[1], "route")) {
      if(argc == 2) {
	llist_t list = netroute_ls();
	llist_iter_t it = llist_iter_alloc(list);
	printf("%20s%20s%20s%20s\n", "Destination","Gateway","Interface","Source");
	while(llist_iter_has_more(it)) {
	  struct netroute_item_ls_s *i = (struct netroute_item_ls_s*)llist_value(llist_iter_next(it));
	  printf("%20s%20s%20s%20s\n", i->dst, i->gateway, i->iface, i->src);
	}
	llist_iter_free(it);
	llist_clear(&list);
	return 1;
      } else if(argc > 4 && (!strcmp(argv[2], "del") || !strcmp(argv[2], "add"))) {
	struct netroute_item_add_s route;
	bzero(&route, sizeof(struct netroute_item_add_s));
	int i = 3;
	for(; i < argc; i++) {
	  if(!strcmp(argv[i], "gw"))
	    strcpy(route.gateway, argv[++i]);
	  else if(!strcmp(argv[i], "dst"))
	    strcpy(route.dst, argv[++i]);
	  else if(!strcmp(argv[i], "mask"))
	    strcpy(route.dst_mask, argv[++i]);
	  else if(!strcmp(argv[i], "src"))
	    strcpy(route.src_iface, argv[++i]);
	  else {
	    nlog("Invalid argument '%s' for route mode\n", argv[i]);
	    usage(EXIT_FAILURE);
	  }
	}
	if(!strcmp(argv[2], "add")) {
	  if(netroute_add(&route)) {
	    if(errno)
	      nlog("%s error: (%d) %s\n", argv[1], errno, strerror(errno));
	  }
	} else {
	  if(netroute_del(&route)) {
	    if(errno)
	      nlog("%s error: (%d) %s\n", argv[1], errno, strerror(errno));
	  }
	}
	return 1;
      } else if(!strcmp(argv[2], "h") || !strcmp(argv[2], "help")) {
	usage_route();
	exit(0);
      } else {
	nlog("Invalid argument number or action for this mode\n");
	usage(EXIT_FAILURE);
      }
    }
  }
  return 0;
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
      if(!strcmp(argv[2], "h") || !strcmp(argv[2], "help")) {
	usage_tun();
	exit(0);
      }
      if(argc < 3) {
	nlog("Invalid argument number for this mode\n");
	usage(EXIT_FAILURE);
      }
      nettun_type_t type = !strcmp(argv[1], "tap") ? NETTUN_TAP : NETTUN_TUN; 
      if(!strcmp(argv[2], "del")) {
	struct nettun_s nt;
	strcpy(nt.name, argv[3]);
	nt.type = type;
	if(nettun_remove(&nt)) {
	  if(errno)
	    nlog("%s error: (%d) %s\n", argv[1], errno, strerror(errno));
	}
	return 1;
      } else if(!strcmp(argv[2], "add")) {
	struct nettun_s nt;
	strcpy(nt.name, argv[3]);
	nt.type = type;
	if(nettun_create(&nt)) {
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


void ping_event_handler(ping_t p, struct ping_event_data_s data) {
  nlog("Ping result: %d - %s(%s)\n", data.result, data.host, data.ip);
  if(data.result == PING_RESULT_SUCCESS)
    nlog("Ping ok (seq %d) in %.02f msec\n", data.seq, data.timestamp);
  if(ping_max_count && ping_count == ping_max_count) {
    end_loop = 1;
    return;
  }
  sleep(1);
  ping_start(p, data.host, data.timeout);
  ping_count++;
}

/**
 * @fn _Bool parse_ping(int argc, char** argv)
 * @brief Parse the tun arguments.
 * @param argc Arguments count.
 * @param argv Arguments value.
 * @return 0 if the message is not aquired else 1.
 */
_Bool parse_ping(int argc, char** argv) {
  if(argc >= 2) {
    if(!strcmp(argv[1], "ping")) {
      if(!strcmp(argv[2], "h") || !strcmp(argv[2], "help")) {
	usage_tun();
	exit(0);
      }
      if(argc < 4) {
	nlog("Invalid argument number for this mode\n");
	usage(EXIT_FAILURE);
      }
      const char* iface = argv[2];
      const char* addr = argv[3];
      int timeout = 15;
      if(argc == 5) {
	if(!string_isint(argv[4])) {
	  nlog("Invalid count value.\n");
	  usage(EXIT_FAILURE);
	}
	ping_max_count = string_parse_int(argv[4], 0);
      }
      if(argc == 6) {
	if(!string_isint(argv[5])) {
	  nlog("Invalid timeout value.\n");
	  usage(EXIT_FAILURE);
	}
	timeout = string_parse_int(argv[5], 15);
      }
      ping = ping_new(iface);
      if(!ping) {
	nlog("Unable to ping using the iface '%s'\n", iface);
	usage(EXIT_FAILURE);
      }
      ping_set_event_handler(ping, ping_event_handler, NULL);
      if(ping_start(ping, addr, timeout)) {
	ping_delete(ping);
	nlog("Unable to ping the address '%s'\n", addr);
	usage(EXIT_FAILURE);
      }
      return 1;
    }
  }
  return 0;
}
