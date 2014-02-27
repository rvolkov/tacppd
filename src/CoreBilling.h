// tacppd resources system information
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

#ifndef __CORE_BILLING_H__
#define __CORE_BILLING_H__

#include "global.h"

namespace tacpp {

/**
@short user information data
@author rv
@version 1
*/
class user_data {
  /**
  thread mutex locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  user data from database */
  db_user ddata_;
  /**
  number of users of this record */
  int numptr_;
 public:
  /**
  set database user data, init mutex */
  user_data(db_user);
  /**
  remove mutex */
  ~user_data();
  /**
  get user database info */
  db_user get();
  /**
  return number of entry users */
  int getNumPtr();
  /**
  increase by 1 number of users */
  void incNumPtr();
  /**
  decrease by 1 number of users */
  void decNumPtr();
};

/**
@short billing objects
@author rv
@version 1
*/
class Billing_data {
  /**
  dynamic module loader */
  DLoad *dl_;
  /**
  thread locker */
  pthread_mutex_t Mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  number of billing module */
  int number_;
  /**
  description */
  string descr_;
  /**
  billing module name */
  string module_;
  /**
  working position or not */
  bool shutdown_;
 public:
  /**
  get billing module number */
  int getNumber();
  /**
  set descr */
  void setDescr(string);
  /**
  get descr */
  string getDescr();
  /**
  set module */
  void setModule(string);
  /**
  get module */
  string getModule();
  /**
  shutdown module */
  void setShutdown(bool);
  /**
  get status */
  bool getShutdown();
  /**
  init data */
  Billing_data(int);
  /**
  free data */
  ~Billing_data();
  /**
  load module */
  string load();
  /**
  get configuration for this module */
  string buildcfstr();
};

/**
@short resources/billing handle
@version 1
@author rv
*/
class CoreBilling : public Core_data {
  /**
  resource modules list */
  list<Billing_data*> bd;
  /**
  thread locker */
  pthread_mutex_t Mutex;
  /**
  set thread lock */
  void lock();
  /**
  remove thread lock */
  void unlock();
  /**
  users data */
  list<user_data*> ud_;
 public:
  /**
  init */
  CoreBilling(const string,const string);
  /**
  free */
  ~CoreBilling();
  /**
  add default resource module num */
  void add(int);
  /**
  del module */
  void del(int);
  /**
  load/reload module */
  string load(int);
  /**
  returns module name for this number */
  string getmodulename(int);
  /**
  set shutdown or not */
  void modify_shut(int,bool);
  /**
  modify loaded module */
  void modify_mod(int,const char*);
  /**
  modify description */
  void modify_des(int,const char*);
  /**
  build config */
  string buildcfstr(int);
  /**
  apply config */
  string applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd=NULL)throw (CmdErr);
  /**
  get user data */
  db_user getUd(string);
  /**
  add user data */
  void addUd(db_user);
  /**
  del user data */
  void delUd(string);
};

};

#endif // __CORE_BILLING_H__
