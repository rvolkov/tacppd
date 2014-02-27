// logging to syslog servers
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
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

#ifndef __LOGSYSLOG_H__
#define __LOGSYSLOG_H__

#include "global.h"

namespace tacpp {

/**
@short output to syslog server, do not delete logging queue
@version 1
@author rv
*/
class LogSyslogAtom {
  Logging *logging;
 public:
  /**
  syslog server */
  ipaddr syslogserv;
  /**
  debug parts for syslog output */
  list<int> fc;
  /**
  id of logging output */
  int output_id;
  /**
  logging object and ip addr of syslog server */
  LogSyslogAtom(Logging*,char*);
  ~LogSyslogAtom();
  /**
  add debug facility */
  void add_debug_fc(int);
  /**
  delete debug facility */
  void del_debug_fc(int);
  /**
  write from buffer to syslog server */
  void logwrite();
};

/**
@short syslog output handler
@version 1
@author rv
*/
class LogSyslog {
  list<LogSyslogAtom*> la;
  Logging *logging;
  pthread_mutex_t Mutex;
  void lock();
  void unlock();
 public:
  LogSyslog(Logging*);
  ~LogSyslog();
  /**
  add syslog server ip */
  void add(char*);
  /**
  delete syslog server ip */
  void del(char*);
  /**
  add debug facility to syslog server ip */
  void add_debug_fc(char*,int);
  /**
  delete debug facility from syslog server ip */
  void del_debug_fc(char*,int);
  /**
  write from buffer to all syslog servers */
  void logwrite();
};

};

#endif //__LOGSYSLOG_H__
