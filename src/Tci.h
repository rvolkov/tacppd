// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: Tci.h
// description: Tacppd Control Interface
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
//    @param [name id] [description]	- can be multiple


#ifndef __TCI_H__
#define __TCI_H__

#include "global.h"

namespace tacpp {

/**
@short TacppdControlInterface server
@author rv
@version 1
*/
class TciServer : public Packet {
  /**
  ip of client */
  ipaddr address;
  /**
  client ip */
  struct in_addr in_address;
  /**
  socket handler */
  int handle;
  /**
  read */
  string read();
  /**
  write */
  bool write(string);
 public:
  /**
  init (constructor) */
  TciServer(int,struct in_addr);
  /**
  clear (destructor) */
  ~TciServer();
  /**
  processor */
  void process();
};

};

#endif //__TCI_H__
