// tacppd administrators
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

#ifndef __CORE_MANAGER_H__
#define __CORE_MANAGER_H__

#include "global.h"

namespace tacpp {

//
// monitor users
//

/**
@short user data, uses md5 for crypt/encrypt password
@version 1
@author rv
*/
//class User_data : public md5 {
class User_data {
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
  user */
  string username_;
  /**
  crypted password (in base64 too) */
  string password_;
  /**
  type of user */
  string type_;
  /**
  user's access list */
  int acl_;
 public:
  /**
  username_ */
  string getUsername();
  /**
  set username value */
  void setUsername(string);
  /**
  password_ */
  string getPassword();
  /**
  set password value */
  void setPassword(string);
  /**
  type_ */
  string getType();
  /**
  set type value */
  void setType(string);
  /**
  acl_ */
  int getAcl();
  /**
  set acl value */
  void setAcl(int);
  /**
  check passwords */
  bool check(const char*);
  /**
  init */
  User_data(const char*,char*,int,char*);
  /**
  free */
  ~User_data();
  /**
  for crypt */
  void create_md5_hash(unsigned char*,unsigned char*);
};

/**
@short work with administrator users list
@version 1
@author rv
*/
class CoreManager : public Core_data {
  /**
  managers list */
  list<User_data*> usr_;
  /**
  thread locker */
  pthread_mutex_t Mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
 public:
  /**
  init */
  CoreManager(const string,const string);
  /**
  free */
  ~CoreManager();
  /**
  add user */
  void add(const char*);
  /**
  del it */
  void del(const char*);
  /**
  change password */
  void modify(const char*,const char*);
  /**
  change acl */
  void modify(const char*,int);
  /**
  change type */
  void modify2(const char*,const char*);
  /**
  change for already encrypted password */
  void cmodify(const char*,const char*);
  /**
  check all access (via acl too) */
  bool check(const char*,const char*,const char*,const char*);
  /**
  build config */
  string buildcfstr(int);
  /**
  process config strings */
  string applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd=NULL)throw (CmdErr);
};

};

#endif // __CORE_MANAGER_H__
