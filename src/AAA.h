// tacppd AAA objects
// (c) Copyright 2000-2008 by Roman Volkov and contributors
// See http://tacppd.org for more information

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

#ifndef __AAA_H__
#define __AAA_H__

#include "global.h"

namespace tacpp {

/**
@short Basic Authentication/Authorization/Accounting for TACACS+ and RADIUS
@version 1
@see Tacacs
@author rv
*/
//class AAA : public md5 {
class AAA {
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
  compare database password with received password (for CHAP too)
  @return true if compare successful */
  bool cpass(const string,const string);
 public:
  /**
  user authentication
  @return true if authentication with db successfull */
  bool db_authentication(const string,const string,const string,
      const ipaddr,const string,const string);
  /**
  get authorization data
  @return database data */
  db_user db_authorization(const string,const ipaddr,const string,const string);
  /**
  write accounting information to DB system */
  void db_accounting(list<string>);
  /**
  constructor */
  AAA();
  /**
  destructor */
  ~AAA();
};

};

#endif //__AAA_H__
