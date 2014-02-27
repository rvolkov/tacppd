// log file
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

#ifndef __LOGFILE_H__
#define __LOGFILE_H__

#include "global.h"

namespace tacpp {

/**
@short output to log files
@version 1
@author rv
*/
class LogFile {
  /**
  log/debug/other files place */
  char logplace[255];
  /**
   */
  Logging *logging;
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
   */
  pthread_mutex_t mutex_error_;
  /**
   */
  pthread_mutex_t mutex_event_;
  /**
   */
  pthread_mutex_t mutex_inform_;
  /**
   */
  pthread_mutex_t mutex_log_;
  /**
   */
  pthread_mutex_t mutex_netf_;
  /**
   */
  pthread_mutex_t mutex_debug_;
 public:
  /**
  debug parts for file output */
  list<int> fc;
  /**
  id of logging output */
  int output_id;
  /**
  log/event/info/debug file place */
  LogFile(Logging*,char*);
  /**
  destructor */
  ~LogFile();
  /**
  add debug facility */
  void add_debug_fc(int);
  /**
  delete debug facility */
  void del_debug_fc(int);
  /**
  is debug fc present? */
  bool is_debug_fc(int);
  /**
  write from buffer to files */
  void logwrite();
};

};

#endif //__LOGFILE_H__
