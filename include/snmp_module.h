// tacppd store end send/receive snmp information
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

#ifndef __SNMP_MODULE_H__
#define __SNMP_MODULE_H__

#ifndef _PTHREADS
#define   _PTHREADS   // enable STL pthread-safe code
#endif //_PTHREADS

#ifndef _REENTRANT
#define   _REENTRANT  // Pthread safe stuff in the stdlibs
#endif //_REENTRANT   // or you should compile with -pthread key

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#ifdef SOLARIS
 #include <strings.h>
#endif

#include <string>
#include <list>

using std::list;
using std::string;

#define SNMP_ERROR_MSG_LEN   200

#ifndef USERNAME_LEN
#define USERNAME_LEN    32
#endif //USERNAME_LEN

#ifndef PORT_LEN
#define PORT_LEN        32
#endif //PORT_LEN

#ifndef IP_LEN
#define IP_LEN          16
#endif //IP_LEN

#ifndef REM_ADDR_LEN
#define REM_ADDR_LEN    64
#endif //REM_ADDR_LEN

#ifndef MAX_SNMP_OUT
#define MAX_SNMP_OUT    3000
#endif //MAX_SNMP_OUT

// common interface OIDs
#define SNMP_ifIndex            ".1.3.6.1.2.1.2.2.1.1"
#define SNMP_ifName             ".1.3.6.1.2.1.31.1.1.1.1"
#define SNMP_ifAdminStatus      ".1.3.6.1.2.1.2.2.1.7"
#define SNMP_ifOperStatus       ".1.3.6.1.2.1.2.2.1.8"

// OIDs for traffic
#define SNMP_ifInOctets         ".1.3.6.1.2.1.2.2.1.10"
#define SNMP_ifOutOctets        ".1.3.6.1.2.1.2.2.1.16"

/**
@short piece of snmp-infomration
@version 1
@author rv
*/
class snmp_d {
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
 public:
  /**
  response */
  string res;
  /**
  index */
  int idx;
  /**
  previous index */
  int idx1;
  /**
  init */
  snmp_d(string o) {
    pthread_mutex_init(&mutex_,0);
    lock();
    res = o;
    idx = 0;
    idx1=0;
    unlock();
  }
  /**
  init */
  snmp_d(string o,int i) {
    pthread_mutex_init(&mutex_,0);
    lock();
    res = o;
    idx = i;
    idx1 = 0;
    unlock();
  }
  /**
  init */
  snmp_d(string o,int i,int i1) {
    pthread_mutex_init(&mutex_,0);
    lock();
    res = o;
    idx = 1;
    idx1 = i1;
    unlock();
  }
  /**
  free */
  ~snmp_d() {
    pthread_mutex_destroy(&mutex_);
  }
};

/**
@short snmp_d container/handler
@version 1
@author rv
*/
class SnmpD {
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
  snmp data list */
  list<snmp_d*> sd_;
 public:
  /**
  init */
  SnmpD() {
    pthread_mutex_init(&mutex_,0);
  }
  /**
  free */
  ~SnmpD() {
    lock();
    snmp_d *tmp = 0;
    for(list<snmp_d*>::iterator i=sd_.begin();i!=sd_.end() && !sd_.empty();) {
      tmp = *i;
      i = sd_.erase(i);
      delete tmp;
    }
    sd_.clear();
    unlock();
    pthread_mutex_destroy(&mutex_);
  }
  /**
  add */
  void add(snmp_d *s) {
    lock();
    sd_.push_back(s);
    unlock();
  }
  /**
  get
  @return snmp_d */
  snmp_d *get(int n) {
    snmp_d *tmp;
    lock();
    tmp = 0;
    if(sd_.size() > 0 && (int)sd_.size() > n) {
      int count = 0;
      for(list<snmp_d*>::const_iterator i=sd_.begin();i!=sd_.end();i++,count++) {
        if(count == n) {
          tmp = *i;
          break;
        }
      }
    }
    unlock();
    return tmp;
  }
};

/**
@short snmp output about active users
@version 1
@author rv
*/
class SnmpOut {
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
  username */
  string user_;
  /**
  port */
  string port_;
  /**
  interface ifOutOctets */
  unsigned long int ifOutOctets_;
  /**
  interface ifInOctets */
  unsigned long int ifInOctets_;
 public:
  /**
  get username data */
  string getUser() {
    string ret;
    lock();
    ret = user_;
    unlock();
    return ret;
  }
  /**
  set username info */
  void setUser(string d) {
    lock();
    user_ = d;
    unlock();
  }
  /**
  get port info */
  string getPort() {
    string ret;
    lock();
    ret = port_;
    unlock();
    return ret;
  }
  /**
  set port info */
  void setPort(string d) {
    lock();
    port_ = d;
    unlock();
  }
  /**
  get if */
  unsigned long int getIfOO() {
    unsigned long int ret;
    lock();
    ret = ifOutOctets_;
    unlock();
    return ret;
  }
  /**
  set if */
  void setIfOO(unsigned long int d) {
    lock();
    ifOutOctets_ = d;
    unlock();
  }
  /**
  get if IO */
  unsigned long int getIfIO() {
    unsigned long int ret;
    lock();
    ret = ifInOctets_;
    unlock();
    return ret;
  }
  /**
  set if io */
  void setIfIO(unsigned long int d) {
    lock();
    ifInOctets_ = d;
    unlock();
  }
  /**
  init */
  SnmpOut() {
    pthread_mutex_init(&mutex_,0);
    lock();
    ifOutOctets_ = 0;
    ifInOctets_ = 0;
    unlock();
  }
  /**
  free */
  ~SnmpOut() {
    pthread_mutex_destroy(&mutex_);
  }
};

/**
@short snmp
@version 1
@author rv
*/
class SnmpDevice {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set thread lock */
  void lock() {
    pthread_mutex_lock(&mutex_);
  }
  /**
  remove thread lock */
  void unlock() {
    pthread_mutex_unlock(&mutex_);
  }
  /**
  data */
  list<SnmpOut*> s_;
  /**
  snmp device */
  string device_;
  /**
  snmp community */
  string commun_;
 public:
  /**
  size */
  int size() {
    int ret;
    lock();
    ret = s_.size();
    unlock();
    return ret;
  }
  /**
  get dev */
  string getDevice() {
    string ret;
    lock();
    ret = device_;
    unlock();
    return ret;
  }
  /**
  set device */
  void setDevice(string d) {
    lock();
    device_ = d;
    unlock();
  }
  /**
  get commun */
  string getCommun() {
    string ret;
    lock();
    ret = commun_;
    unlock();
    return ret;
  }
  /**
  set commun */
  void setCommun(string d) {
    lock();
    commun_ = d;
    unlock();
  }
  /**
  init */
  SnmpDevice(char *d,char *c) {
    pthread_mutex_init(&mutex_,0);
    lock();
    device_ = d;
    commun_ = c;
    unlock();
  }
  /**
  free */
  ~SnmpDevice() {
    SnmpOut *tmp;
    tmp = 0;
    lock();
    for(list<SnmpOut*>::iterator i=s_.begin();i!=s_.end() && !s_.empty();) {
      tmp = *i;
      i = s_.erase(i);
      delete tmp;
    }
    s_.clear();
    unlock();
    pthread_mutex_destroy(&mutex_);
  }
  /**
  get */
  SnmpOut *get(int n) {
    SnmpOut *tmp;
    tmp = 0;
    lock();
    if(s_.size() > 0 && (int)s_.size() > n) {
      int count = 0;
      for(list<SnmpOut*>::const_iterator i=s_.begin();i!=s_.end(); i++,count++) {
        if(count == n) {
          tmp = *i;
          break;
        }
      }
    }
    unlock();
    return tmp;
  }
  /**
  add */
  void add(SnmpOut *so) {
    lock();
    s_.push_back(so);
    unlock();
  }
};



#endif //__SNMP_MODULE_H__
