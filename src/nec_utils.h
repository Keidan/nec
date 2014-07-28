/**
 *******************************************************************************
 * @file nec_utils.h
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
#ifndef __NEC_UTILS_H__
  #define __NEC_UTILS_H__

  #include "nec_config.h"
  #include <tk/io/net/nettun.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <errno.h>
  #include <string.h>
  #include <tk/utils/string.h>
  #include <tk/io/net/net.h>
  #include <limits.h>


  #define INVALID_INT INT_MIN

  #define nlog(...) ({				\
    fprintf(stderr, __VA_ARGS__);		\
  })

  #define flag_param(result, name) ({			\
    if(name[0] != '-' && name[0] != '+') {		\
      nlog("Invalid %s flag usage (state)!\n", name);	\
      usage(EXIT_FAILURE);				\
    }							\
    result = (name[0] == '-') ? _false : _true;		\
  })

  typedef enum {
    _nset = -1,
    _false = 0,
    _true = 1
  } mbool;

  /**
   * @fn void usage_tun()
   * @brief Print the tun parts of the usage function.
   */
  void usage_tun();

  /**
   * @fn void usage_ping()
   * @brief Print the ping parts of the usage function.
   */
  void usage_ping();

  /**
   * @fn void usage_route()
   * @brief Print the route parts of the usage function.
   */
  void usage_route();

  /**
   * @fn void usage_base()
   * @brief Print the base parts of the usage function.
   */
  void usage_base();

  /**
   * @fn _Bool parse_tun(int argc, char** argv)
   * @brief Parse the tun arguments.
   * @param argc Arguments count.
   * @param argv Arguments value.
   * @return 0 if the message is not aquired else 1.
   */
  _Bool parse_tun(int argc, char** argv);

  /**
   * @fn _Bool parse_route(int argc, char** argv)
   * @brief Parse the route arguments.
   * @param argc Arguments count.
   * @param argv Arguments value.
   * @return 0 if the message is not aquired else 1.
   */
  _Bool parse_route(int argc, char** argv);

  /**
   * @fn _Bool parse_ping(int argc, char** argv)
   * @brief Parse the ping arguments.
   * @param argc Arguments count.
   * @param argv Arguments value.
   * @return 0 if the message is not aquired else 1.
   */
  _Bool parse_ping(int argc, char** argv);

  /**
   * @fn _Bool parse_base(int argc, char** argv)
   * @brief Parse the tun arguments.
   * @param argc Arguments count.
   * @param argv Arguments value.
   * @return 0 if the message is not aquired else 1.
   */
  _Bool parse_base(int argc, char** argv);

#endif /* __NEC_UTILS_H__ */
