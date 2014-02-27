// tacppd database description types
// (c) Copyright 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information

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

#ifndef __DB_MODULE_H__
#define __DB_MODULE_H__

#ifndef _PTHREADS
#define   _PTHREADS   // enable STL pthread-safe code
#endif //_PTHREADS

#ifndef _REENTRANT
#define   _REENTRANT  // Pthread safe stuff in the stdlibs
#endif //_REENTRANT   // or you should compile with -pthread key

#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#ifdef SOLARIS
#include <strings.h>
#endif //SOLARIS

#include <pthread.h>
#include <string>
#include <list>
#include <stdlib.h>

#include "ipaddr.h"

using std::list;
using std::string;
using namespace tacpp;

#define T_INT_TYPE      (int)0
#define T_UINT_TYPE     (unsigned int)0
#define T_STR_TYPE      ""
#define T_DATE_TYPE     (time_t)0
#define T_BOOL_TYPE     false
#define T_FLOAT_TYPE    (float)0
#define T_IP_TYPE       ipaddr()

#define T_NOIDX         0
#define T_INDEX         1
#define T_UNIQUE        2

#define DB_MODULE_ERRLEN  300

/**
@short database field
@version 1
@author rv
*/
class DbField {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock() {
    pthread_mutex_lock(&mutex_);
  }
  /**
  remove lock */
  void unlock() {
    pthread_mutex_unlock(&mutex_);
  }
  /**
  database field name */
  string name_;
  /**
  field description */
  string description_;
  /**
  default value */
  string defv_;
  /**
  type 1-b,2-i,3-f,4-s,5-t,6-ip */
  int t_;
  /**
  update flag */
  bool update_;
  /**
  request flag */
  bool request_;
  /**
  bool type storage */
  bool bool_type_;
  /**
  int type storage */
  unsigned long int int_type_;
  /**
  float type storage */
  float float_type_;
  /**
  string type storage */
  string string_type_;
  /**
  string len */
  int string_len_;
  /**
  date type storage */
  time_t date_type_;
  /**
  ipaddr type store */
  ipaddr ip_type_;
  /**
  index type: 0-no, 1-index, 2-unique index */
  int idx_;
 public:
  /**
  name_
  @return name_ value */
  string getName() {
    lock();
    string ret = name_;
    unlock();
    return ret;
  }
  /**
  set name_ value */
  void setName(string d) {
    lock();
    name_ = d;
    unlock();
  }
  /**
  get description_
  @return description_ value */
  string getDescription() {
    lock();
    string ret = description_;
    unlock();
    return ret;
  }
  /**
  set description_ value */
  void setDescription(string d) {
    lock();
    description_ = d;
    unlock();
  }
  /**
  get defv_
  @return defv_ value */
  string getDefv() {
    string ret;
    lock();
    ret = defv_;
    unlock();
    return ret;
  }
  /**
  set defv_ */
  void setDefv(string d) {
    lock();
    defv_ = d;
    unlock();
  }
  /**
  type 1-b,2-i,3-f,4-s,5-t,6-ip */
  int getT() {
    int ret;
    lock();
    ret = t_;
    unlock();
    return ret;
  }
  /**
  set t */
  void setT(int d) {
    lock();
    t_ = d;
    unlock();
  }
  /**
  update_ */
  bool getUpdate() {
    bool ret;
    lock();
    ret = update_;
    unlock();
    return ret;
  }
  /**
  set update */
  void setUpdate(bool d) {
    lock();
    update_ = d;
    unlock();
  }
  /**
  request_ */
  bool getRequest() {
    bool ret;
    lock();
    ret = request_;
    unlock();
    return ret;
  }
  /**
  set request */
  void setRequest(bool d) {
    lock();
    request_ = d;
    unlock();
  }
  /**
  bool_type_ */
  bool getBoolType() {
    bool ret;
    lock();
    ret = bool_type_;
    unlock();
    return ret;
  }
  /**
  set bool type */
  void setBoolType(bool d) {
    lock();
    bool_type_ = d;
    unlock();
  }
  /**
  get int type value
  @return int_type_ value */
  unsigned long int getIntType() {
    unsigned long int ret;
    lock();
    ret = int_type_;
    unlock();
    return ret;
  }
  /**
  set int type */
  void setIntType(unsigned long int d) {
    lock();
    int_type_ = d;
    unlock();
  }
  /**
  get float type value
  @return float_type_ */
  float getFloatType() {
    float ret;
    lock();
    ret = float_type_;
    unlock();
    return ret;
  }
  /**
  set float type value */
  void setFloatType(float d) {
    lock();
    float_type_ = d;
    unlock();
  }
  /**
  get string type
  @return string_type_ */
  string getStringType() {
    string ret;
    lock();
    ret = string_type_;
    unlock();
    return ret;
  }
  /**
  set string type */
  void setStringType(string d) {
    lock();
    string_type_ = d;
    unlock();
  }
  /**
  get string length
  @return string_len_ */
  int getStringLen() {
    int ret;
    lock();
    ret = string_len_;
    unlock();
    return ret;
  }
  /**
  set string length */
  void setStringLen(int d) {
    lock();
    string_len_ = d;
    unlock();
  }
  /**
  get date type
  @return date_type_ */
  time_t getDateType() {
    time_t ret;
    lock();
    ret = date_type_;
    unlock();
    return ret;
  }
  /**
  set date type */
  void setDateType(time_t d) {
    lock();
    date_type_ = d;
    unlock();
  }
  /**
  get ip type
  @return ip_type_ */
  ipaddr getIpType() {
    ipaddr ret;
    lock();
    ret = ip_type_;
    unlock();
    return ret;
  }
  /**
  set ip type */
  void setIpType(ipaddr d) {
    lock();
    ip_type_ = d;
    unlock();
  }
  /**
  index type: 0-no, 1-index, 2-unique index */
  int getIdx() {
    int ret;
    lock();
    ret = idx_;
    unlock();
    return ret;
  }
  /**
  set idx */
  void setIdx(int d) {
    lock();
    idx_ = d;
    unlock();
  }
  /**
  clear (destructor) */
  ~DbField() {
    pthread_mutex_destroy(&mutex_);
  }
  /**
  init */
  DbField(const string n,const string d,const string f,bool v,int ix=0) {
    pthread_mutex_init(&mutex_,0);
    lock();
    t_ = 1;  // 1-bool,2-int/uint/ulong,3-float,4-string/char*,5-time_t,6-ipaddr
    update_ = false;
    request_ = false;
    name_ = n;
    description_ = d;
    defv_ = f;
    bool_type_ = v;
    int_type_ = 0;
    float_type_ = 0;
    string_len_ = 0;
    date_type_ = 0;
    ip_type_ = UNKNOWN_IP;
    idx_ = ix;
    unlock();
  }
  /**
  init int type !!!! */
  DbField(const string n,const string d,const string f,int v,int ix=0) {
    pthread_mutex_init(&mutex_,0);
    lock();
    t_ = 2; // 1-bool,2-int/uint/ulong,3-float,4-string/char*,5-time_t,6-ipaddr
    update_ = false;
    request_ = false;
    name_ = n;
    description_ = d;
    defv_ = f;
    bool_type_ = false;
    int_type_ = (unsigned long int)v;
    float_type_ = 0;
    string_len_ = 0;
    date_type_ = 0;
    ip_type_ = UNKNOWN_IP;
    idx_ = ix;
    unlock();
  }
  /**
  init uint type */
  DbField(const string n,const string d,const string f,unsigned int v,int ix=0) {
    pthread_mutex_init(&mutex_,0);
    lock();
    t_ = 2; // 1-bool,2-int/uint/ulong,3-float,4-string/char*,5-time_t,6-ipaddr
    update_ = false;
    request_ = false;
    name_ = n;
    description_ = d;
    defv_ = f;
    bool_type_ = false;
    int_type_ = (unsigned long int)v;
    float_type_ = 0;
    string_len_ = 0;
    date_type_ = 0;
    ip_type_ = UNKNOWN_IP;
    idx_ = ix;
    unlock();
  }
  /**
  init ulong type */
  DbField(const string n,const string d,const string f,unsigned long int v,int ix=0) {
    pthread_mutex_init(&mutex_,0);
    lock();
    t_ = 2; // 1-bool,2-int/uint/ulong,3-float,4-string/char*,5-time_t,6-ipaddr
    update_ = false;
    request_ = false;
    name_ = n;
    description_ = d;
    defv_ = f;
    bool_type_ = false;
    int_type_ = v;
    float_type_ = 0;
    string_len_ = 0;
    date_type_ = 0;
    ip_type_ = UNKNOWN_IP;
    idx_ = ix;
    unlock();
  }
  /**
  init float type */
  DbField(const string n,const string d,const string f,float v,int ix=0) {
    pthread_mutex_init(&mutex_,0);
    lock();
    t_ = 3; // 1-bool,2-int/uint/ulong,3-float,4-string/char*,5-time_t,6-ipaddr
    update_ = false;
    request_ = false;
    name_ = n;
    description_ = d;
    defv_ = f;
    bool_type_ = false;
    int_type_ = 0;
    float_type_ = v;
    string_len_ = 0;
    date_type_ = 0;
    ip_type_ = UNKNOWN_IP;
    idx_ = ix;
    unlock();
  }
  /**
  init for string */
  DbField(const string n,const string d,const string f,string v,int l,int ix=0) {
    pthread_mutex_init(&mutex_,0);
    lock();
    t_ = 4; // 1-bool,2-int/uint/ulong,3-float,4-string/char*,5-time_t,6-ipaddr
    update_ = false;
    request_ = false;
    name_ = n;
    description_ = d;
    defv_ = f;
    bool_type_ = false;
    int_type_ = 0;
    float_type_ = 0;
    string_type_ = v;
    string_len_ = l;
    date_type_ = 0;
    ip_type_ = UNKNOWN_IP;
    idx_ = ix;
    unlock();
  }
  /**
  init for char* */
  DbField(const string n,const string d,const string f,char *v,int l,int ix=0) {
    pthread_mutex_init(&mutex_,0);
    lock();
    t_ = 4; // 1-bool,2-int/uint/ulong,3-float,4-string/char*,5-time_t,6-ipaddr
    update_ = false;
    request_ = false;
    name_ = n;
    description_ = d;
    defv_ = f;
    bool_type_ = false;
    int_type_ = 0;
    float_type_ = 0;
    string_type_ = v;
    string_len_ = l;
    date_type_ = 0;
    ip_type_ = UNKNOWN_IP;
    idx_ = ix;
    unlock();
  }


  /**
  init for datetime */
  DbField(const string n,const string d,const string f,time_t v,int ix=0) {
    pthread_mutex_init(&mutex_,0);
    lock();
    t_ = 5; // 1-bool,2-int/uint/ulong,3-float,4-string/char*,5-time_t,6-ipaddr
    update_ = false;
    request_ = false;
    name_ = n;
    description_ = d;
    defv_ = f;
    bool_type_ = false;
    int_type_ = 0;
    float_type_ = 0;
    string_len_ = 0;
    date_type_ = v;
    ip_type_ = UNKNOWN_IP;
    idx_ = ix;
    unlock();
  }


  /**
  init for ip addr type */
  DbField(const string n,const string d,const string f,ipaddr v,int ix=0) {
    pthread_mutex_init(&mutex_,0);
    lock();
    t_ = 6; // 1-bool,2-int/uint/ulong,3-float,4-string/char*,5-time_t,6-ipaddr
    update_ = false;
    request_ = false;
    name_ = n;
    description_ = d;
    defv_ = f;
    bool_type_ = false;
    int_type_ = 0;
    float_type_ = 0;
    string_len_ = 0;
    date_type_ = 0;
    ip_type_ = v;
    idx_ = ix;
    unlock();
  }
};

/**
@short database table description
@version 1
@author rv
*/
class DbTable {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock() {
    pthread_mutex_lock(&mutex_);
  }
  /**
  remove lock */
  void unlock() {
    pthread_mutex_unlock(&mutex_);
  }
  /**
  database fields */
  list<DbField*> f_;
  /**
  table name */
  string name_;
  /**
  table name mask */
  string namemask_;
  /**
  table description */
  string description_;
 public:
  /**
  get table name
  @return name_ */
  string getName() {
    string ret;
    lock();
    ret = name_;
    unlock();
    return ret;
  }
  /**
  set table name */
  void setName(string d) {
    lock();
    name_ = d;
    unlock();
  }
  /**
  get table name mask
  @return namemask_ */
  string getNamemask() {
    string ret;
    lock();
    ret = namemask_;
    unlock();
    return ret;
  }
  /**
  set table namemask */
  void setNamemask(string d) {
    lock();
    namemask_ = d;
    unlock();
  }
  /**
  get table description
  @return description_ */
  string getDescription() {
    string ret;
    lock();
    ret = description_;
    unlock();
    return ret;
  }
  /**
  set table description */
  void setDescription(string d) {
    lock();
    description_ = d;
    unlock();
  }
  /**
  size */
  int size() {
    int ret;
    lock();
    ret = f_.size();
    unlock();
    return ret;
  }
  /**
  init */
  DbTable(const string n,const string m,const string d) {
    pthread_mutex_init(&mutex_,0);
    lock();
    description_ = d;
    namemask_ = m;
    name_ = n;
    unlock();
  }
  /**
  free */
  ~DbTable() {
    lock();
    DbField *tmp;
    for(list<DbField*>::iterator i=f_.begin();i!=f_.end() && !f_.empty();) {
      tmp = *i;
      i = f_.erase(i);
      delete tmp;
    }
    f_.clear();
    unlock();
    pthread_mutex_destroy(&mutex_);
  }
  /**
  clear all */
  void clear() {
    DbField *tmp;
    lock();
    for(list<DbField*>::iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      unlock();
      tmp->setRequest(false);
      tmp->setUpdate(false);
      tmp->setBoolType(false);
      tmp->setIntType(0);
      tmp->setFloatType(0);
      tmp->setStringType("");
      tmp->setStringLen(0);
      tmp->setDateType(0);
      tmp->setIpType(UNKNOWN_IP);
      lock();
    }
    unlock();
  }
  /**
  set request field */
  void set_request(const string n) {
    DbField *tmp;
    lock();
    for(list<DbField*>::iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n)
        tmp->setRequest(true);
    }
    unlock();
  }
  /**
  set update field */
  void set_update(const string n) {
    DbField *tmp;
    lock();
    for(list<DbField*>::iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n)
        tmp->setUpdate(true);
    }
    unlock();
  }
  /**
  add bool type */
  void add(const string n,const string d,const string df,bool v,int ix=0) {
    DbField *tmp;
    lock();
    for(list<DbField*>::iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        tmp->setBoolType(v);
        unlock();
        return;
      }
    }
    f_.push_back(new DbField(n,d,df,v,ix));
    unlock();
  }
  /**
  stripped form */
  void add(const string n,bool v) {
    add(n,"","",v);
  }
  /**
  add int type */
  void add(const string n,const string d,const string df,int v,int ix=0) {
    DbField *tmp;
    lock();
    for(list<DbField*>::iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        tmp->setIntType((unsigned long int)v);
        unlock();
        return;
      }
    }
    f_.push_back(new DbField(n,d,df,v,ix));
    unlock();
  }
  /**
  short form of add */
  void add(const string n,int v) {
    add(n,"","",v);
  }
  /**
  add unsigned int type */
  void add(const string n,const string d,const string df,unsigned int v,int ix=0) {
    DbField *tmp;
    lock();
    for(list<DbField*>::iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        tmp->setIntType((unsigned long int)v);
        unlock();
        return;
      }
    }
    f_.push_back(new DbField(n,d,df,v,ix));
    unlock();
  }
  /**
  short form of add */
  void add(const string n,unsigned int v) {
    add(n,"","",v);
  }
  /**
  add unsigned long int type */
  void add(const string n,const string d,const string df,unsigned long int v,int ix=0) {
    DbField *tmp;
    lock();
    for(list<DbField*>::iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        tmp->setIntType(v);
        unlock();
        return;
      }
    }
    f_.push_back(new DbField(n,d,df,v,ix));
    unlock();
  }
  /**
  short form of add */
  void add(const string n,unsigned long int v) {
    add(n,"","",v);
  }
  /**
  add float type */
  void add(const string n,const string d,const string df,float v,int ix=0) {
    DbField *tmp;
    lock();
    for(list<DbField*>::iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        tmp->setFloatType(v);
        unlock();
        return;
      }
    }
    f_.push_back(new DbField(n,d,df,v,ix));
    unlock();
  }
  /**
  short form */
  void add(const string n,float v) {
    add(n,"","",v);
  }
  /**
  add string type */
  void add(const string n,const string d,const string df,string v,int l,int ix=0) {
    DbField *tmp;
    lock();
    for(list<DbField*>::iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        tmp->setStringType(v);
        unlock();
        return;
      }
    }
    f_.push_back(new DbField(n,d,df,v,l,ix));
    unlock();
  }
  /**
  short form */
  void add(const string n,string v,int l=0) {
    add(n,"","",v,l);
  }
  /**
  add char* type */
  void add(const string n,const string d,const string df,char *v,int l,int ix=0) {
    DbField *tmp;
    lock();
    string stmp = v;
    for(list<DbField*>::iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        tmp->setStringType(stmp);
        unlock();
        return;
      }
    }
    f_.push_back(new DbField(n,d,df,stmp,l,ix));
    unlock();
  }
  /**
  short form */
  void add(const string n,char *v,int l=0) {
    add(n,"","",v,l);
  }
  
  /**
  add time_t type */
  void add(const string n,const string d,const string df,time_t v,int ix=0) {
    DbField *tmp;
    lock();
    for(list<DbField*>::iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        tmp->setDateType(v);
        unlock();
        return;
      }
    }
    f_.push_back(new DbField(n,d,df,v,ix));
    unlock();
  }
  /**
  short form of add */
  void add(const string n,time_t v) {
    add(n,"","",v);
  }

  
  
  /**
  add ipaddr type */
  void add(const string n,const string d,const string df,ipaddr v,int ix=0) {
    DbField *tmp;
    lock();
    for(list<DbField*>::iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        tmp->setIpType(v);
        unlock();
        return;
      }
    }
    f_.push_back(new DbField(n,d,df,v,ix));
    unlock();
  }
  /**
  short form of add */
  void add(const string n,ipaddr v) {
    add(n,"","",v);
  }
  /**
  get database field name */
  string get_name(int n) {
    string ret;
    lock();
    if(f_.size() > 0 && (int)f_.size() > n) {
      DbField *tmp = 0;
      int count = 0;
      list<DbField*>::const_iterator i;
      for(i=f_.begin();i!=f_.end();i++,count++) {
        if(count == n) {
          tmp = *i;
          ret = tmp->getName();
          break;
        }
      }
    }
    unlock();
    return ret;
  }
  /**
  get database field description */
  string get_descr(int n) {
    string ret;
    lock();
    if(f_.size() > 0 && (int)f_.size() > n) {
      DbField *tmp = 0;
      int count = 0;
      list<DbField*>::const_iterator i;
      for(i=f_.begin();i!=f_.end();i++,count++) {
        if(count == n) {
          tmp = *i;
          ret = tmp->getDescription();
          break;
        }
      }
    }
    unlock();
    return ret;
  }
  /**
  get database field default value */
  string get_defv(int n) {
    string ret;
    lock();
    if(f_.size() > 0 && (int)f_.size() > n) {
      DbField *tmp = 0;
      int count = 0;
      list<DbField*>::const_iterator i;
      for(i=f_.begin();i!=f_.end();i++,count++) {
        if(count == n) {
          tmp = *i;
          ret = tmp->getDefv();
          break;
        }
      }
    }
    unlock();
    return ret;
  }
  /**
  get db field type number */
  int get_type(int n) {
    int ret;
    lock();
    ret = 0;
    if(f_.size() > 0 && (int)f_.size() > n) {
      DbField *tmp = 0;
      int count = 0;
      list<DbField*>::const_iterator i;
      for(i=f_.begin();i!=f_.end();i++,count++) {
        if(count == n) {
          tmp = *i;
          ret = tmp->getT();
          break;
        }
      }
    }
    unlock();
    return ret;
  }
  /**
  get db field index */
  int get_idx(int n) {
    int ret;
    lock();
    ret = 0;
    if(f_.size() > 0 && (int)f_.size() > n) {
      DbField *tmp = 0;
      int count = 0;
      list<DbField*>::const_iterator i;
      for(i=f_.begin();i!=f_.end();i++,count++) {
        if(count == n) {
          tmp = *i;
          ret = tmp->getIdx();
          break;
        }
      }
    }
    unlock();
    return ret;
  }
  /**
  get request */
  bool get_request(int n) {
    bool ret;
    lock();
    ret = false;
    if(f_.size() > 0 && (int)f_.size() > n) {
      DbField *tmp = 0;
      int count = 0;
      list<DbField*>::const_iterator i;
      for(i=f_.begin();i!=f_.end();i++,count++) {
        if(count == n) {
          tmp = *i;
          ret = tmp->getRequest();
          break;
        }
      }
    }
    unlock();
    return ret;
  }
  /**
  get update */
  bool get_update(int n) {
    bool ret;
    lock();
    ret = false;
    if(f_.size() > 0 && (int)f_.size() > n) {
      DbField *tmp = 0;
      int count = 0;
      list<DbField*>::const_iterator i;
      for(i=f_.begin();i!=f_.end();i++,count++) {
        if(count == n) {
          tmp = *i;
          ret = tmp->getUpdate();
          break;
        }
      }
    }
    unlock();
    return ret;
  }
  /**
  get bool value */
  bool get(const string n,bool *v) {
    DbField *tmp;
    lock();
    for(list<DbField*>::const_iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        if(tmp->getT() != 1) {
          unlock();
          return false;
        }
        *v = tmp->getBoolType();
        unlock();
        return true;
      }
    }
    unlock();
    return false;
  }
  /**
  set bool value */
  void set(const string n,bool v) {
    DbField *tmp;
    lock();
    for(list<DbField*>::const_iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        if(tmp->getT() != 1) {
          unlock();
          return;
        }
        tmp->setBoolType(v);
        unlock();
        return;
      }
    }
    unlock();
    return;
  }
  /**
  get int value */
  bool get(const string n,int *v) {
    DbField *tmp;
    lock();
    for(list<DbField*>::const_iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        if(tmp->getT() != 2) {
          unlock();
          return false;
        }
        *v = tmp->getIntType();
        unlock();
        return true;
      }
    }
    unlock();
    return false;
  }
  /**
  set int value */
  void set(const string n,int v) {
    DbField *tmp;
    lock();
    for(list<DbField*>::const_iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        if(tmp->getT() != 2) {
          unlock();
          return;
        }
        tmp->setIntType(v);
        unlock();
        return;
      }
    }
    unlock();
    return;
  }
  /**
  get float value */
  bool get(const string n,float *v) {
    DbField *tmp;
    lock();
    for(list<DbField*>::const_iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        if(tmp->getT() != 3) {
          unlock();
          return false;
        }
        *v = tmp->getFloatType();
        unlock();
        return true;
      }
    }
    unlock();
    return false;
  }
  /**
  set float value */
  void set(const string n,float v) {
    DbField *tmp;
    lock();
    for(list<DbField*>::const_iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        if(tmp->getT() != 3) {
          unlock();
          return;
        }
        tmp->setFloatType(v);
        unlock();
        return;
      }
    }
    unlock();
    return;
  }
  /**
  get string(char*) value */
  bool get(const string n,char *v) {
    DbField *tmp;
    lock();
    for(list<DbField*>::const_iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        if(tmp->getT() != 4) {
          unlock();
          return false;
        }
        strncpy(v,tmp->getStringType().c_str(),300);
        unlock();
        return true;
      }
    }
    unlock();
    return false;
  }
  /**
  get string(char*) value */
  bool get(const string n,string *v) {
    DbField *tmp;
    lock();
    for(list<DbField*>::const_iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        if(tmp->getT() != 4) {
          unlock();
          return false;
        }
        *v = tmp->getStringType();
        unlock();
        return true;
      }
    }
    unlock();
    return false;
  }
  /**
  get string length */
  int getslen(const string n) {
    DbField *tmp;
    int ret;
    lock();
    ret = 0;
    for(list<DbField*>::const_iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        ret = tmp->getStringLen();
        unlock();
        return ret;
      }
    }
    unlock();
    return ret;
  }
  /**
  set string(char*) value */
  void set(const string n,string v) {
    DbField *tmp;
    lock();
    for(list<DbField*>::const_iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        if(tmp->getT() != 4) {
          unlock();
          return;
        }
        tmp->setStringType(v);
        unlock();
        return;
      }
    }
    unlock();
    return;
  }
  /**
  get datetime value */
  bool get(const string n,time_t *v) {
    DbField *tmp;
    lock();
    for(list<DbField*>::const_iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        if(tmp->getT() != 5) {
          unlock();
          return false;
        }
        *v = tmp->getDateType();
        unlock();
        return true;
      }
    }
    unlock();
    return false;
  }
  /**
  set datetime value */
  void set(const string n,time_t v) {
    DbField *tmp;
    lock();
    for(list<DbField*>::const_iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        if(tmp->getT() != 5) {
          unlock();
          return;
        }
        tmp->setDateType(v);
        unlock();
        return;
      }
    }
    unlock();
    return;
  }
  /**
  get ipaddr value
  @return true if found, false if not */
  bool get(const string n,ipaddr *v) {
    DbField *tmp;
    lock();
    for(list<DbField*>::const_iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        if(tmp->getT() != 6) {
          unlock();
          return false;
        }
        *v = tmp->getIpType();
        unlock();
        return true;
      }
    }
    unlock();
    return false;
  }
  /**
  set ipaddr value */
  void set(const string n,ipaddr v) {
    DbField *tmp;
    lock();
    for(list<DbField*>::const_iterator i=f_.begin();i!=f_.end();i++) {
      tmp = *i;
      if(tmp->getName() == n) {
        if(tmp->getT() != 6) {
          unlock();
          return;
        }
        tmp->setIpType(v);
        unlock();
        return;
      }
    }
    unlock();
    return;
  }
};

#endif //__DB_MODULE_H__
