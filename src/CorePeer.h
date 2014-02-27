// tacppd peers for roaming
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

#ifndef __CORE_PEER_H__
#define __CORE_PEER_H__

#include "global.h"

namespace tacpp {

#define PEERMASK_LEN    64

/**
@short peer masks
@version 1
@author rv
*/
class Peer_mask {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  regex mask */
  string mask_;
 public:
  /**
  mask_ */
  string getMask();
  /**
  set mask value */
  void setMask(string);
  /**
  check by this mask */
  bool check(const char*);
  /**
  init */
  Peer_mask(const char*);
  /**
  free */
  ~Peer_mask();
};

/**
@short Peer data
@version 1
@author rv
*/
class Peer_data {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  peer masks */
  list<Peer_mask*> mask_;
  /**
  description */
  string descr_;
  /**
  key */
  string key_;
  /**
  peer host */
  ipaddr peerhost_;
  /**
  peer host port */
  unsigned int port_;
 public:
  /**
  descr_ */
  void setDescr(string);
  /**
  get descr value */
  string getDescr();
  /**
  key_ */
  void setKey(string);
  /**
  get key value */
  string getKey();
  /**
  peerhost_ */
  void setPeerhost(ipaddr);
  /**
  get peer host value */
  ipaddr getPeerhost();
  /**
  port_ */
  void setPort(unsigned int);
  /**
  get port value */
  unsigned int getPort();
  /**
  init */
  Peer_data(ipaddr,int);
  /**
  free */
  ~Peer_data();
  /**
  build config for one peer */
  string buildcfstr();
  /**
  check string by all masks */
  bool check_by_mask(const char*);
  /**
  add mask string */
  void add_mask(const char*);
  /**
  del mask string */
  void del_mask(const char*);
};

/**
@short core Peer object
@version 1
@author rv
*/
class CorePeer : public Core_data {
  /**
  peers list */
  list<Peer_data*> pd_;
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
 public:
  /**
  init */
  CorePeer(const string,const string);
  /**
  free */
  ~CorePeer();
  /**
  add peer */
  void add(ipaddr,unsigned int);
  /**
  delete peer */
  void del(ipaddr,unsigned int);
  /**
  add mask to peer */
  void add_mask(ipaddr,unsigned int,const char*);
  /**
  del mask from peer */
  void del_mask(ipaddr,unsigned int,const char*);
  /**
  modify description */
  void modify_des(ipaddr,unsigned int,const char*);
  /**
  modify key */
  void modify_key(ipaddr,unsigned int,const char*);
  /**
  build config */
  string buildcfstr(int);
  /**
  config applyer */
  string applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd=NULL)throw (CmdErr);
};

};

#endif // __CORE_PEER_H__
