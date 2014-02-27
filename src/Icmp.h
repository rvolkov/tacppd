// icmp code
// (c) Copyright 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

// all comments which start from /** - this is comments for KDoc
//  for classes
//    @short [short description of the class]
//    @author [class author]
//    @version [class version]
//    @see [references to other classes or methods]
//  for methods
//    @see [references]
//    @return [sentence describing the return value]
//    @exception [list the exeptions that could be thrown]
//    @param [name id] [description] - can be multiple

#ifndef __ICMP_H__
#define __ICMP_H__

#include "global.h"

namespace tacpp {

/**
@short basic icmp testing
@version 1
*/
class Icmp {
  /**
  icmp socket */
  int sock;
  /**
  create icmp socket */
  int make_socket();
  /**
  ping checksum */
  int in_cksum(unsigned short*,int);
  /**
  send/receive icmp echo */
  bool test_ping(struct sockaddr_in);
  /**
  returns current time */
  int time_start(struct timeval*);
  /**
  returns time offset */
  int time_end(struct timeval*);
  /**
  compare */
  int time_minus(struct timeval*,struct timeval*);
 public:
  /**
   */
  Icmp();
  /**
   */
  ~Icmp();
  /**
  check device by icmp */
  bool test(ipaddr);
};

};

#endif //__ICMP_H__
