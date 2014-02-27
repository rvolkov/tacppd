// address pools
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
//    @param [name id] [description]	- can be multiple

#ifndef __CORE_POOL_H__
#define __CORE_POOL_H__

#include "global.h"

namespace tacpp {

/**
@short this is one entry in addrpool
@version 1
@author rv
*/
class Addrpool_entry {
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
  address */
  ipaddr address_;
  /**
  ocuppyed/free */
  bool tag_;
 public:
  /**
  address_ */
  void setAddress(ipaddr);
  /**
  get addr value */
  ipaddr getAddress();
  /**
  tag_ */
  void setTag(bool);
  /**
  get tag value */
  bool getTag();
  /**
  init */
  Addrpool_entry(ipaddr);
  /**
  free */
  ~Addrpool_entry();
};

/**
@short this is one addrpool
@version 1
@author rv
*/
class Addrpool_data {
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
  addrpool entries */
  list<Addrpool_entry*> ae_;
  /**
  pool number */
  int pool_;
 public:
  /**
  pool_ */
  int getPool();
  /**
  set pool value */
  void setPool(int);
  /**
  init */
  Addrpool_data(int);
  /**
  free */
  ~Addrpool_data();
  /**
  add addr to addrpool */
  void add(ipaddr);
  /**
  del addr from addrpool */
  void del(ipaddr);
  /**
  get addr */
  string get();
  /**
  return addr */
  void ret(ipaddr);
  /**
  build config for this object */
  string buildcfstr();
  /**
  group */
  string group();
};

/**
@short addreses pool manager
@version 1
@author rv
*/
class CorePool : public Core_data {
  /**
  address pools */
  list<Addrpool_data*> ap_;
  /**
  translate addr strings to separate addrs */
  string getAddr(const char*,int);
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
 public:
  /**
  init */
  CorePool(const string,const string);
  /**
  free */
  ~CorePool();
  /**
  add addresses to pool */
  void add(int,const char*);
  /**
  delete whole pool */
  void del(int);
  /**
  delete address from pool */
  void del(int,const char*);
  /**
  get addr from pool */
  string get(int);
  /**
  retrieve addr to pool */
  void ret(ipaddr);
  /**
  build configuration for object */
  string buildcfstr(int);
  /**
  apply config */
  string applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd=NULL)throw (CmdErr);
};

};

#endif // __CORE_POOL_H__
