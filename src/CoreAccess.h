// tacppd acl core layer
// (c) Copyright 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// all comments which start from /**<CR> - this is comments for KDoc
//  for classes
//    @short [short description of the class]
//    @author [class author]
//    @version [class version]
//    @see [references to other classes or methods]
//  for methods
//    @see [references]
//    @return [sentence describing the return value]
//    @exception [list the exeptions that could be thrown]
//    @param [name id] [description] - can be multiple

#ifndef __CORE_ACCESS_H__
#define __CORE_ACCESS_H__

#include "global.h"

namespace tacpp {

/**
@short one entry in access list
@verson 1
@author rv
*/
class Acl_entry {
  /**
  permit regex string */
  string permit_;
  /**
  deny regex string */
  string deny_;
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
  get permit value */
  string getPermit();
  /**
  set permit value */
  void setPermit(const string);
  /**
  get deny value */
  string getDeny();
  /**
  set deny value */
  void setDeny(const string);
  /**
  check string with permit regex */
  bool checkPermit(const string);
  /**
  check string with deny regex */
  bool checkDeny(const string);
  /**
  store values in variables */
  Acl_entry(const string,const string);
  /**
  clear all */
  ~Acl_entry();
};

/**
@short access list data
@version 1
@author rv
*/
class Acl_data {
  /**
  entries in this acl */
  list<Acl_entry*> acl_;
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
  acl number */
  int number_;
 public:
  /**
  returns access list number */
  int getNumber();
  /**
  access list constructor */
  Acl_data(const int);
  /**
  access list destructor */
  ~Acl_data();
  /**
  check string with whole access list */
  bool check(const char*);
  /**
  create config for this object */
  string buildcfstr();
  /**
  add permit acl entry */
  bool add_permit(const char*);
  /**
  add deny acl entry */
  bool add_deny(const char*);
  /**
  del permit entry */
  bool del_permit(const char*);
  /**
  del deny entry */
  bool del_deny(const char*);
};

/**
@short working with acl - access lists manager
@version 1
@author rv
*/
class CoreAccess : public Core_data {
  /**
  access list entries */
  list<Acl_data*> acl_;
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
  set data for acl */
  CoreAccess(const string,const string);
  /**
  remove data */
  ~CoreAccess();
  /**
  add permit rule to acl */
  bool add_permit(int,const char*);
  /**
  add deny rule to acl */
  bool add_deny(int,const char*);
  /**
  add whole acl with number */
  void add(int);
  /**
  delete whole acl with number */
  bool del(int);
  /**
  delete one permit rule from acl */
  bool del_permit(int,const char*);
  /**
  delete one deny rule from acl */
  bool del_deny(int,const char*);
  /**
  check rule with all for whole acl */
  bool check(int,const char*);
  /**
  build config for one acl */
  string buildcfstr(int);
  /**
  process config strings */
  string applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd=NULL)throw (CmdErr);
};

};

#endif // __CORE_ACCESS_H__
