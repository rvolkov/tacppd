// debug subsystem
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

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "global.h"

namespace tacpp {

// Debug Facilities 1-250
#define LOG_TACACS    1     // tacacs+ protocol events
#define LOG_BILLING   2     // billing subsytem
#define LOG_AAA       3     // AAA
#define LOG_DB        4     // database system activity
#define LOG_SNMP      5     // snmp system activity
#define LOG_NETFLOW   6     // netflow collector
#define LOG_SERVER    7     // tcp/udp server
#define LOG_CC        8     // cryptocard
#define LOG_POOL      9     // address pool system
#define LOG_TRACK     10    // user track system
#define LOG_HTTP      11    // http interface
#define LOG_TIP       12    // bundle protocol
#define LOG_TEP       13    // external protocol

/**
@short debug descriptors
@version 1
@author rv
*/
class DebugFacility {
  /**
  pthread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  facility number */
  int num_;
  /**
  facility string */
  string name_;
  /**
  facility description */
  string description_;
 public:
  /**
  get facility number
  @return facility number */
  int getNum();
  /**
  get facility name
  @return facility name */
  string getName();
  /**
  get facility descr
  @return facility descr */
  string getDescription();
  /**
  init */
  DebugFacility(int,const string,const string);
  /**
  clear */
  ~DebugFacility();
};

/**
@short debug log
@version 1
@author rv
*/
class Debug {
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
  debug descriptions */
  list<DebugFacility*> ddf_;
 public:
  /**
  init */
  Debug();
  /**
  clear */
  ~Debug();
  /**
  facility name
  @return facility name */
  string fname(int);
  /**
  output facility description
  @return facility description */
  string fdescr(int);
  /**
  facility number from name
  @return facility number */
  int fnum(const char*);
};

};

#endif // __DEBUG_H__
