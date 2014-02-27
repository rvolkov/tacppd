// logging to terminal
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

#ifndef __LOGTERM_H__
#define __LOGTERM_H__

#include "global.h"

namespace tacpp {

// identification of one logging terminal
class LogTermAtom : public Packet {
  Logging *logging;
 public:
  /**
  terminal socket id */
  int fh;
  /**
  debug parts for output */
  list<int> fc;
  /**
  id of logging output */
  int output_id;
  /**
  logging object and sock */
  LogTermAtom(Logging*,int);
  /**
  destructor */
  ~LogTermAtom();
  /**
  add debug facility */
  void add_debug_fc(int);
  /**
  delete debug facility */
  void del_debug_fc(int);
  /**
  write from buffer to syslog server */
  void logwrite();
  /**
  write to terminal */
  bool twrite(char*);
};

class LogTerm {
  list<LogTermAtom*> la;
  Logging *logging;
  pthread_mutex_t Mutex;
  void lock();
  void unlock();
 public:
  int output_id;
  LogTerm(Logging*);
  ~LogTerm();
  void add(int);
  void del(int);
  /**
  add debug facility to server */
  void add_debug_fc(int,int);
  /**
  delete debug facility from server */
  void del_debug_fc(int,int);
  /**
  write from buffer to all servers */
  void logwrite();
};

};

#endif //__LOGTERM_H__
