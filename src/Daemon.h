// low-level daemonization
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

#ifndef __DAEMON_H__
#define __DAEMON_H__

#include "global.h"

namespace tacpp {

/**
@short daemonization system
@version 1
*/
class Daemon {
 public:
  /**
  close all descriptors */
  void closeall(int fd);
  /**
   */
  int daemon(int nochdir, int noclose);
  /**
   */
  int fork2();
};

};

#endif //__DAEMON_H__
