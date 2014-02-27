// tacppd packet handling
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// all comments which start from /** - this is comments for KDoc */
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

#ifndef __PACKET_H__
#define __PACKET_H__

#include "global.h"

namespace tacpp {

/**
@short Packet
@version 1
@author rv
*/
class Packet {
  /**
  mutex locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  socket id */
  int fid_;
 public:
  /**
  get fid_ */
  int getFid();
  /**
  set fid_ */
  void setFid(int);
  /**
  init data */
  Packet(int);
  /**
  destroy data */
  ~Packet();
  /**
  read from socket */
  int sockread(unsigned char*,int,int);
  /**
  write socket */
  int sockwrite(unsigned char*,int,int);
};

};

#endif //__PACKET_H__
