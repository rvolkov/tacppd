// tacppd Authorization pairs modifyer
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

#ifndef __AMODIFY_H__
#define __AMODIFY_H__

#include "global.h"

namespace tacpp {

/**
@short abstraction layer for several modifyer types
@version 1
@author rv
*/
class AModifyers {
 public:
  /**
  stripped command processor */
  virtual string modifyer(const char*)=0;
  /**
  this is abstract class and we should use virtual destructor */
  virtual ~AModifyers() {}
};

/**
@short internal modifyers
@version 1
@author rv
*/
class AModify_INT : public AModifyers {
 public:
  /**
  internal modifyer body
  @return modifyed authorization string */
  string modifyer(const char*);
};

/**
@short external sql modifyer
@version 1
*/
class AModify_SQL : public AModifyers {
 public:
  /**
  sql modifyer body
  @return modifyed authorization string */
  string modifyer(const char*);
};

/**
@short external program modifyer
@version 1
*/
class AModify_EXT : public AModifyers {
 public:
  /**
  external program modifyer body
  @return modifyed authorization string */
  string modifyer(const char*);
};

/**
@short end-user interface for AV pairs modifyers
@version 1
@author rv
*/
class AModify {
  /**
   */
  AModifyers *am;
  /**
   */
  string name;
  /**
  network device */
  ipaddr nas;
  /**
  portname */
  string port;
  /**
  phone string */
  string phone;
  /**
  thread locker */
  pthread_mutex_t Mutex_;
  /**
  set variables: change $name to name value
  $port to port value and
  $nas to nas value
  $phone to rem_addr value (typical telephone number for dialin) */
  string setvars(const string);
 public:
  /**
  constr which set username,cisco,port,phone */
  AModify(const string,const ipaddr,const string,const string);
  /**
  data modifyer */
  db_user modifyer(const db_user);
  /**
   */
  ~AModify();
};

};

#endif //__AMODIFY_H__
