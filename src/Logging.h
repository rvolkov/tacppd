// tacppd logging code
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
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

#ifndef __LOGGING_H__
#define __LOGGING_H__

#include "global.h"

#define MAX_LOG_STR_LEN     900
#define LOG_BUFFER_SIZE     20

// log critical error messages
#define LOG_ERROR       0
// log NetFlow
#define LOG_NETFLOWLOG  252
// log events
#define LOG_EVENT       253
// log informational
#define LOG_INFORM      254
// log logging information to other file
#define LOG_LOGGING     255
// numbers 1-250 - several parts of debugging with lowest level

namespace tacpp {

/**
@short logging data - one entry
@version 1
@author rv
*/
class LogData {
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
  logging level */
  int level;
  /**
  logging string */
  string str_;
  /**
  is data wrote? */
  bool is_written_;
 public:
  /**
  add log entry to list */
  LogData(int,string);
  /**
  returns log level digit */
  int get_level();
  /**
  returns log string */
  string get_string();
  /**
   */
  ~LogData();
  /**
  is data being wroute? */
  bool isWritten();
  /**
  set is_wroute flag */
  void setWritten(bool);
};

/**
@short logging list descriptor
@version 1
@author rv
*/
class LogList {
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
  output id */
  int num_;
  /**
  log strings for this LogList */
  list<LogData*> logs;
 public:
  /**
   */
  int getNum();
  /**
  constructor */
  LogList(int);
  /**
  destructor */
  ~LogList();
  /**
  add entry to list */
  void add(int,string);
  /**
  get/del entry from list with level */
  string del(int);
  /**
  get and delete log entry, return 0 if no any */
  LogData *getEntry();
  /**
   */
  int size();
};

/**
@short
@version 1
@author rv
*/
class Logging_end {
 public:
};

/**
@short
@version 1
@author rv
*/
class Logging_set {
 public:
  /**
  logging level */
  int level;
  /**
  set logging level */
  Logging_set(int a) {
    level = a;
  }
  /**
  copy constructor */
  void operator =(Logging_set x) {
    level = x.level;
  }
};

/**
@short common logging
@version 1
@author rv
*/
class Logging {
  /**
  log messages common list with open tasks */
  list<LogList*> tacpp_loglist;
  /**
  mutex locker */
  pthread_mutex_t Mutex_;
  /**
  second mutex locker */
  pthread_mutex_t Mutex2_;
  /**
  set mutex lock */
  void lock2();
  /**
  remove mutex lock */
  void unlock2();
 public:
  /**
  set mutex lock */
  void lock();
  /**
  remove mutex lock */
  void unlock();
  /**
   */
  Logging();
  /**
   */
  ~Logging();
  /**
  get log list for id */
  LogList *getl(int);
  /**
  add to output list */
  int add();
  /**
  delete from output list */
  void del(int);
  /**
  output to log buffer with level */
  void out_msg(int, string);
  /**
   */
  void out_netflow(string);
  /**
   */
  void out_logging(string);
  /**
  prepare for output */
  void log(int, char *fmt,...);
  /**
  clear queues */
  void clear();
  /**
   */
  int size();
  /**
  output buffer */
  string sbuf;
  /**
   */
  int log2_level;
  /**
   */
  bool present;
  /**
   */
  Logging_end Logging_end_endlog();
  /**
   */
  Logging_set Logging_set_setlog(int);
  /**
   */
  Logging& operator <<(const char*);
  /**
   */
  Logging& operator <<(const int);
  /**
   */
  Logging& operator <<(const unsigned int);
  /**
   */
  Logging& operator <<(const string);
  /**
   */
  Logging& operator <<(ipaddr);
  /**
   */
  Logging& operator <<(const float);
  /**
   */
  Logging& operator <<(const double);
  /**
   */
  Logging& operator <<(const bool);
  /**
   */
  Logging& operator <<(Logging_end);
  /**
   */
  Logging& operator <<(Logging_set);
};

};

#endif //__LOGGING_H__
