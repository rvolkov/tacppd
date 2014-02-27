// tacppd debugging
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

#ifndef __CORE_DEBUG_H__
#define __CORE_DEBUG_H__

#include "global.h"

namespace tacpp {

/**
@short debug descriptors
@version 1
@author rv
*/
class Debug_entry {
  /**
  thread locker */
  pthread_mutex_t Mutex_;
  /**
  set thread lock */
  void lock();
  /**
  remove thread lock */
  void unlock();
  /**
  debug number */
  int num_;
  /**
   */
  string name_;
  /**
   */
  int output_;
  /**
  syslog server */
  ipaddr syslogserv_;
 public:
  /**
  */
  int getNum();
  /**
  */
  string getName();
  /**
  */
  int getOutput();
  /**
  */
  ipaddr getSyslogserv();
  /**
  create entry with debug facility name plus output
  type and syslog ip */
  Debug_entry(int,const char*,int,ipaddr);
  /**
  delete debug facility */
  ~Debug_entry();
};

/**
@short debug outputs in config (no descriptions needed)
@version 1
@author rv
*/
class Debug_data {
  /**
  debug faciliti entries */
  list<Debug_entry*> de_;
  /**
  1 - file; 2 - syslog */
  int output_;
  /**
  syslog server */
  ipaddr syslogserv_;
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
   */
  int getOutput();
  /**
   */
  ipaddr getSyslogserv();
  /**
  create config string for this object */
  string buildcfstr();
  /**
  set type of output and ipaddr of syslog */
  Debug_data(int,const char*);
  /**
  delete all from debug list */
  ~Debug_data();
  /**
  add facility number 'k' with name 'n' */
  void add(int,const char*);
  /**
  del facility number 'n' */
  void del(int);
};

/**
@short debug log
@version 1
@author rv
*/
class CoreDebug : public Core_data, public Debug {
  /**
  debug descriptions */
  list<Debug_data*> dd_;
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
  init */
  CoreDebug(const string,const string);
  /**
  free */
  ~CoreDebug();
  /**
  add facility to output type */
  void add(int,const char*,int);
  /**
  del facility from output type */
  void del(int,const char*,int);
  /**
  add output type */
  void add(int,const char*);
  /**
  del output type */
  void del(int,const char*);
  /**
  build config */
  string buildcfstr(int);
  /**
  apply cf */
  string applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd=NULL)throw (CmdErr);
};

};

#endif // __CORE_DEBUG_H__
