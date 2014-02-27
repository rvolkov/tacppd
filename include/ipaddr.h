// tacppd common IP address handling
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information

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

#ifndef __IPADDR_H__
#define __IPADDR_H__

#ifndef _PTHREADS
  #define   _PTHREADS   // enable STL pthread-safe code
#endif //_PTHREADS

// set pthread-safe stdlibs
#ifndef _REENTRANT
  #define	_REENTRANT
#endif //_REENTRANT

#ifdef FREEBSD
#include <sys/types.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#endif //FREEBSD

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <pthread.h>

#ifdef SOLARIS
#include <strings.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/in_systm.h>
#endif

using std::string;
//#ifndef SOLARIS
//using std::snprintf;
//#endif

#include <string.h>
#include <stdlib.h>

#define UNKNOWN_IP "0.0.0.0"

namespace tacpp {

/**
@short basic ip addr storage/handle
@author rv
@version 1
*/
class ipaddr {
  /**
  thread locker */
//  pthread_mutex_t mutex_;
  /**
  set thread lock */
//  void lock() {
//    pthread_mutex_lock(&mutex_);
//  }
  /**
  remove thread lock */
//  void unlock() {
//    pthread_mutex_unlock(&mutex_);
//  }
  /**
  hostname */
  char hname_[100];
  /**
  get ip addr in string form from hostname */
  string getipfromname(const char *name) {
    if(!name) return UNKNOWN_IP;
    if(strlen(name)==0) return UNKNOWN_IP;
    if(strcmp(name,"localhost")==0) return "127.0.0.1";
    if(strcmp(name,"0.0.0.0") == 0) return UNKNOWN_IP;
    if(strcmp(name,UNKNOWN_IP) == 0) return UNKNOWN_IP;
    struct in_addr nas_addr;
    struct hostent host;
    struct hostent res;
    struct hostent *res1 = &res;
    char buf[4096];
    bzero(buf,sizeof(buf));
    char hostaddr[64];
    string ostr;
    bzero(hostaddr,sizeof(hostaddr));
#ifdef SOLARIS
    int error_num = 0;
    if((res1 = gethostbyname_r(name,&host,buf,sizeof(buf),&error_num)) != 0) {
#endif
#ifndef SOLARIS
#ifdef FREEBSD
    if((res1 = gethostbyname(name)) != 0) {
#else
    // Linux
    int error_num = 0;
    if(gethostbyname_r(name,&host,buf,sizeof(buf),&res1,&error_num) != 0) {
#endif
#endif
      // error_num =
      //  HOST_NOT_FOUND
      //  NO_ADDRESS
      //  NO_RECOVERY
      //  TRY_AGAIN
      snprintf(hostaddr,sizeof(hostaddr)-1,"%s",name);
      ostr = hostaddr;
      return ostr;
    }
//    if(sizeof(struct in_addr) < (unsigned int)host->h_length) {
//std::cerr<<"something wrong in ipaddr.h"<<std::endl;
//    }
    bcopy(host.h_addr,(char*)&nas_addr,host.h_length);
    strncpy(hostaddr,(char*)inet_ntoa(nas_addr),sizeof(hostaddr)-1);
    ostr = hostaddr;
//    freehostent(host);
    return ostr;
  }
  /**
  local host ip addr */
  string gethostn() {
    char hostname[64];
    gethostname((char*)hostname,sizeof(hostname));
    string ostr(hostname);
    return ostr;
  }
  /**
  ip address */
  unsigned long ipa_;
 public:
  /**
  return ip addr in unsigned long form */
  unsigned long getIpa() {
    unsigned long ret;
//    lock();
    ret = ipa_;
//    unlock();
    return ret;
  }
  /**
  store ip addr value */
  ipaddr(const char *s) {
//    pthread_mutex_init(&mutex_,0);
    fill(s);
  }
  /**
  ip addr */
  ipaddr(const string s) {
//    pthread_mutex_init(&mutex_,0);
    fill(s.c_str());
  }
  /**
  store localhost ip addr value */
  ipaddr() {
//    pthread_mutex_init(&mutex_,0);
//    fill(gethostn().c_str());
    fill("localhost");
  }
  /**
  destroy */
  ~ipaddr() {
//    pthread_mutex_destroy(&mutex_);
  }
  /**
  change string addr to unsigned long */
  unsigned long ips(string i) {
    unsigned long ret;
    ret = inet_addr(i.c_str());
    return ret;
  }
  /**
  change ip addr from unsigned long form to char* with dots */
  string sip(unsigned long i) {
    struct in_addr a;
    string str;
    a.s_addr = (in_addr_t)i;
    str = inet_ntoa(a);
    return str;
  }
  /**
  fill */
  void fill(const char *s) {
    if(!s && strlen(s)<1) {
      fill("localhost");
      return;
    }
//    lock();
    snprintf(hname_,sizeof(hname_)-1,"%s",s);
    char str[100];
    snprintf(str,sizeof(str)-1,"%s",getipfromname(s).c_str());
    ipa_ = ips(str);
//    unlock();
  }
  /**
  get ip addr string
  @return ip addr in string form */
  string get() {
    string ostr;
//    lock();
    ostr = sip(ipa_);
//    unlock();
    return ostr;
  }
  /**
  get ip addr digit value */
  int get(int n) {
    char str[100];
//    lock();
    snprintf(str,sizeof(str)-1,"%s",sip(ipa_).c_str());
//    unlock();
    int o1 = 0;
    int o2 = 0;
    int o3 = 0;
    int o4 = 0;
    int k=0, i=0;
    char d1[4];
    char d2[4];
    char d3[4];
    char d4[4];
    bzero(d1, sizeof(d1));
    bzero(d2, sizeof(d2));
    bzero(d3, sizeof(d3));
    bzero(d4, sizeof(d4));
    for(i=0; str[k] && str[k]!='.' && i<3; i++,k++) d1[i] = str[k];
    if(str[k]=='.') k++;
    for(i=0; str[k] && str[k]!='.' && i<3; i++,k++) d2[i] = str[k];
    if(str[k]=='.') k++;
    for(i=0; str[k] && str[k]!='.' && i<3; i++,k++) d3[i] = str[k];
    if(str[k]=='.') k++;
    for(i=0; str[k] && str[k]!='.' && str[k]!=' ' && i<3; i++,k++) d4[i] = str[k];
    o1 = atoi(d1);
    o2 = atoi(d2);
    o3 = atoi(d3);
    o4 = atoi(d4);
    if(n == 0) return o1;
    if(n == 1) return o2;
    if(n == 2) return o3;
    if(n == 3) return o4;
    return 0;
  }
  /**
  get host name for this addr
  @return hostname as string */
  string getn() {
//    lock();
    string ostr(hname_);
//    unlock();
    return ostr;
  }
  /**
  compare == */
  bool operator ==(ipaddr a) {
    bool ret;
//    lock();
    ret = false;
    if(ipa_ == a.ipa_) ret = true;
//    unlock();
    return ret;
  }
  /**
  != */
  bool operator !=(ipaddr a) {
    bool ret;
//    lock();
    ret = false;
    if(ipa_ != a.ipa_) ret = true;
//    unlock();
    return ret;
  }
  /**
  == */
  bool operator ==(const char *a) {
    bool ret;
//    lock();
    ret = false;
    ipaddr aaa(a);
    if(ipa_ == aaa.ipa_) ret = true;
//    unlock();
    return ret;
  }
  /**
  != */
  bool operator !=(const char *a) {
    bool ret;
//    lock();
    ret = false;
    ipaddr aaa(a);
    if(ipa_ != aaa.ipa_) ret = true;
//    unlock();
    return ret;
  }
  /**
  copy constructor */
  void operator =(ipaddr a) {
//    lock();
//    a.lock();
    ipa_ = a.ipa_;
    snprintf(hname_,sizeof(hname_),"%s",a.hname_);
//    a.unlock();
//    unlock();
  }
  /**
  fill address with char* string */
  void operator <<(const char *a) {
    fill(a);
  }
  /**
  fill address with char* string */
  void operator =(const char *a) {
    fill(a);
  }
};

};

#endif //__IPADDR_H__
