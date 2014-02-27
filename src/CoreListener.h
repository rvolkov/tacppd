// tacppd core listener support
// (c) Copyright 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

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

#ifndef __CORE_LISTENER_H__
#define __CORE_LISTENER_H__

#include "global.h"

namespace tacpp {

/**
@short data for listener
@version 1
@author rv
*/
class Listener_data {
  /**
  thread locker */
  pthread_mutex_t Mutex_;
  /**
  thread locker 2 */
  pthread_mutex_t mutex2_;
  /**
  thread descriptors number for maxconnect process */
  int qDescrNum_;
  /**
  is server workable */
  bool created_;
  /**
  string server name */
  string servname_;
  /**
  maximum available connections */
  int maxconnect_;
  /**
  access list */
  int acl_;
  /**
  port on which server work */
  unsigned int port_;
  /**
  index of type */
  int servertype_;
  /**
  mutex for all listeners */
  pthread_mutex_t *lmutex_;
 public:
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  set lock */
  void lock2();
  /**
  remove lock */
  void unlock2();
  /**
  all listeners lock */
  void llock();
  /**
  all listeners unlock */
  void lunlock();
  /**
  is server can continue work */
  bool isCreated();
  /**
  set flag for server continue or stop work */
  void setCreated(bool);
  /**
  add thread descriptor counter */
  int add_session(Connect*);
  /**
  delete from counter */
  void del_session(int);
  /**
  number of threads */
  int con_session();
  /**
  servname_ */
  void setServname(string);
  /**
  servname */
  string getServname();
  /**
  maxconnect_ */
  void setMaxconnect(int);
  /**
  maximum connect */
  int getMaxconnect();
  /**
  acl_ */
  void setAcl(int);
  /**
  get acl value */
  int getAcl();
  /**
  function pointer */
  void (*UsersFunction)(int,struct in_addr,int,char*);
  /**
  port_ */
  void setPort(unsigned int);
  /**
  get port value */
  unsigned int getPort();
  /**
  servertype_ */
  void setServertype(int);
  /**
  get servertype value */
  int getServertype();
  /**
  init */
  Listener_data(unsigned int,int,int,void(*)(int,struct in_addr,int,char*),int,pthread_mutex_t*);
  /**
  create threads */
  void create();
  /**
  free */
  ~Listener_data();
  /**
  destroy server */
  void destroy();
  /**
  queue for data for processor threads */
//  queue::myQueue lQueue;
};

/**
@short TCP/UDP listener manager
@version 1
@author 1
*/
class CoreListener : public Core_data {
  /**
  servers list */
  list<Listener_data*> srv_;
  /**
  thread locker */
  pthread_mutex_t Mutex_;
  /**
  listeners thread locker */
  pthread_mutex_t LMutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
 public:
  /**
  create threads (for later servers start) */
  void init();
  /**
  init */
  CoreListener(const string,const string);
  /**
  desctructor - stop all servers */
  ~CoreListener();
  /**
  add server */
  void add(unsigned int);
  /**
   */
  void add(unsigned int,int,const char*,int);
  /**
  delete/stop server */
  void del(unsigned int);
  /**
  check by access list */
  bool check(unsigned int,const char*);
  /**
  FIXME */
  void modify1(unsigned int,const char*);
  /**
  FIXME */
  void modify2(unsigned int,int);
  /**
  FIXME */
  void modify3(unsigned int,int);
  /**
  build config */
  string buildcfstr(int);
  /**
  apply config */
  string applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd=NULL)throw (CmdErr);
};

};

#endif // __CORE_LISTENER_H__
