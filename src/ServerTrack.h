// tacppd servers tracker
// (c) Copyright 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: ServerTrack.h
// description: tacppd servers tracker
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

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

#ifndef __SERVERTRACK_H__
#define __SERVERTRACK_H__

#include "global.h"

namespace tacpp {

////////////// tacppd servers database ////////////

/**
@short entry about one tacppd server
@version 1
@author rv
*/
class ServerTrack_data {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  tacppd server ip */
  ipaddr ip_;
  /**
  tacppd server priority */
  int priority_;
  /**
  entry creation/update time */
  time_t entry_time_;
 public:
  /**
  destroy */
  ~ServerTrack_data();
  /**
  store information about available tacppd server */
  ServerTrack_data(ipaddr,int);
  /**
  if tacppd server - localhost */
  ServerTrack_data(int);
  /**
  update entry_time and priority value */
  void update(int);
  /**
  get ip of this server */
  ipaddr getIp();
  /**
  get priority of this server */
  int getPri();
  /**
  return entry_time */
  time_t getTime();
};

/**
@short tacppd servers handler
@version 1
@author rv
*/
class ServerTrack {
  /**
  data list about tacppd servers */
  list<ServerTrack_data*> csrv_;
  /**
  thread locker */
  pthread_mutex_t Mutex_;
  /**
  set thread lock */
  void lock();
  /**
  remove thread lock */
  void unlock();
 public:
  /**
  void constructor */
  ServerTrack();
  /**
  delete entries from csrv */
  ~ServerTrack();
  /**
  send start and add localhost to list */
  void sendStart();
  /**
  check ttl value in list and expire it */
  void checkTtl();
  /**
  add or update server to list */
  void update(ipaddr,int);
  /**
  check if this server active */
  bool isActive();
};

};

#endif //__SERVERTRACK_H__
