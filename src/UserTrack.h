// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: UserTrack.h
// description: work users tracker

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

#ifndef __USERTRACK_H__
#define __USERTRACK_H__

#include "global.h"

namespace tacpp {

////////////// working users database ////////////

/**
@short db user cache entry
@author rv
@version 1
*/
class dbc_entry {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set thread lock */
  void lock();
  /**
  remove thread lock */
  void unlock();
  /**
  database data about user */
  db_user entry_;
  /**
  time when data was added (for expiration) */
  time_t date_;
 public:
  /**
  fill data (constructor) */
  dbc_entry(db_user);
  /**
  clear (destructor) */
  ~dbc_entry();
  /**
  returns database data
  @return data about user*/
  db_user get();
  /**
  get time info
  @return time info */
  time_t getTime();
};

/**
@short database cache manager
@author rv
@version 1
*/
class dbc_man {
  /**
  thread locker */
  pthread_mutex_t mutex_dbc_;
  /**
  set thread lock */
  void lock();
  /**
  remove thread lock */
  void unlock();
  /**
  database data cache */
  list<dbc_entry*> dbec_;
 public:
  /**
  init (constructor) */
  dbc_man();
  /**
  clear (destructor) */
  ~dbc_man();
  /**
  is data presented in cache
  @return true if data there */
  bool present(const string);
  /**
  user information
  @return user informaton from cache */
  db_user get(const string);
  /**
  add database data to cache */
  void add(const db_user);
};

/**
@short active user entries handler
@author rv
@version 1
*/
class UserTrack {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set pthread lock */
  void lock();
  /**
  remove pthread lock */
  void unlock();
  /**
  database data cache manager */
  dbc_man dbc_;
 public:
  /**
  init locker */
  UserTrack();
  /**
  free locker */
  ~UserTrack();
  /**
  get from db system and add as inactive to list if presents
  @return database data for user */
  db_user get2add(string,ipaddr,string,string);
  /**
  add user to list or renew ttl or do it active
  @return true if added */
  bool add(string,ipaddr,string,string,ipaddr);
  /**
  delete user from list
  @return true if deleted */
  bool del(string,ipaddr,string);
  /**
  periodic cache checking */
  void periodic();
};

};

#endif //__USERTRACK_H__
