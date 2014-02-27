// tacppd support for join several tacppd to one bundle
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

#ifndef __CORE_BUNDLE_H__
#define __CORE_BUNDLE_H__

#include "global.h"

namespace tacpp {

/**
@short bundle core handler (there are only one bundle entry possible)
@version 1
@author rv
*/
class CoreBundle : public Core_data {
  /**
  thread locker */
  pthread_mutex_t Mutex_;
  /**
  thread lock */
  void lock();
  /**
  thread unlock */
  void unlock();
  /**
  multicast group number */
  ipaddr mgroup_;
  /**
  bundle udp port number */
  unsigned int port_;
  /**
  multicast packet ttl value */
  int ttl_;
  /**
  bundle priority value */
  int priority_;
  /**
  bundle crypto key */
  string key_;
  /**
  bundle keepalive packets timer counter */
  time_t keepalive_time_;
 public:
  /**
  send info to CoreData constructor */
  CoreBundle(const string,const string);
  /**
  clear all - this is destructor */
  ~CoreBundle();
  /**
  is bundle created? */
  bool isBundle();
  /**
  returns bundle group number */
  ipaddr getMgroup();
  /**
  returns bundle port number */
  unsigned int getPort();
  /**
  returns bundle ttl value */
  int getTtl();
  /**
  returns bundle priority */
  int getPri();
  /**
  returns crypto key */
  string getKey();
  /**
  delete bundle */
  void del();
  /**
  change bundle group number */
  void modifyMgroup(ipaddr);
  /**
  change bundle port */
  void modifyPort(unsigned int);
  /**
  modify key */
  void modifyKey(const string);
  /**
  modify ttl */
  void modifyTtl(int);
  /**
  modify priority */
  void modifyPri(int);
  /**
  build config for bundle (only available for int=0 value) */
  string buildcfstr(int);
  /**
  apply object config changes */
  string applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd=NULL)throw (CmdErr);
  /**
  check and send keepalive packet every 10 seconds to bundle */
  void periodic();
};

/**
@short bundle connect processor
@version 1
@author rv
*/
class BundleProcessor {
 public:
  /**
  packet processor */
  BundleProcessor(ipaddr,unsigned int,char*);
};

};

#endif // __CORE_BUNDLE_H__
