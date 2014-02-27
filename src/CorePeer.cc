// tacppd peers for roaming
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

#include "global.h"

/////////////////// Peer_mask ///////////////
// this is regex mask for identify logins/area codes and peers
// which maintain its

Peer_mask::Peer_mask(const char *s) {
  pthread_mutex_init(&mutex_,0);
  lock();
  mask_ = s;
  unlock();
}

Peer_mask::~Peer_mask() {
  pthread_mutex_destroy(&mutex_);
}

void
Peer_mask::lock() {
  pthread_mutex_lock(&mutex_);
}

void
Peer_mask::unlock() {
  pthread_mutex_unlock(&mutex_);
}

string
Peer_mask::getMask() {
  lock();
  string ret = mask_;
  unlock();
  return ret;
}

void
Peer_mask::setMask(string d) {
  lock();
  mask_ = d;
  unlock();
}

bool
Peer_mask::check(const char *s) {
  bool filter;
  filter = false;
  regex_t preg;
  size_t nmatch = 2;
  regmatch_t pmatch[2];

  lock();
  if(mask_.size() > 0) {
    if(regcomp(&preg,mask_.c_str(),REG_EXTENDED) == 0) {
      if(regexec(&preg,s,nmatch,pmatch,0) == 0) filter = true;
      regfree(&preg);
    }
  }
  unlock();
  return filter;
}

/////////////////// Peer_data ///////////////

Peer_data::Peer_data(ipaddr peer,int p) {
  pthread_mutex_init(&mutex_,0);
  descr_ = "none"; // set default value
  peerhost_ = peer;
  port_ = p;
  key_ = "none"; // set default value
}

Peer_data::~Peer_data() {
  Peer_mask *tmp;
  lock();
  for(list<Peer_mask*>::iterator i=mask_.begin();i!=mask_.end() && !mask_.empty();) {
    tmp = *i;
    i = mask_.erase(i);
    delete tmp;
  }
  mask_.clear();
  unlock();
  pthread_mutex_destroy(&mutex_);
}

void
Peer_data::lock() {
  pthread_mutex_lock(&mutex_);
}

void
Peer_data::unlock() {
  pthread_mutex_unlock(&mutex_);
}

void
Peer_data::setDescr(string d) {
  lock();
  descr_ = d;
  unlock();
}

string
Peer_data::getDescr() {
  lock();
  string ret = descr_;
  unlock();
  return ret;
}

void
Peer_data::setKey(string d) {
  lock();
  key_ = d;
  unlock();
}

string
Peer_data::getKey() {
  lock();
  string ret = key_;
  unlock();
  return ret;
}

void
Peer_data::setPeerhost(ipaddr d) {
  lock();
  peerhost_ = d;
  unlock();
}

ipaddr
Peer_data::getPeerhost() {
  lock();
  ipaddr ret = peerhost_;
  unlock();
  return ret;
}

void
Peer_data::setPort(unsigned int d) {
  lock();
  port_ = d;
  unlock();
}

unsigned int
Peer_data::getPort() {
  lock();
  unsigned int ret = port_;
  unlock();
  return ret;
}

bool
Peer_data::check_by_mask(const char *s) {
  Peer_mask *tmp;
  lock();
  for(list<Peer_mask*>::const_iterator i=mask_.begin();i!=mask_.end();i++) {
    tmp = *i;
    if(tmp->check(s)) {
      unlock();
      return true;
    }
  }
  unlock();
  return false;
}

void
Peer_data::add_mask(const char *s) {
  Peer_mask *tmp;
  lock();
  for(list<Peer_mask*>::const_iterator i=mask_.begin();i!=mask_.end();i++) {
    tmp = *i;
    if(tmp->getMask() == s) {
      unlock();
      return;
    }
  }
  tmp = new Peer_mask(s);
  mask_.push_back(tmp);
  unlock();
}

void
Peer_data::del_mask(const char *s) {
  Peer_mask *tmp;
  lock();
  for(list<Peer_mask*>::iterator i=mask_.begin();i!=mask_.end();i++) {
    tmp = *i;
    if(tmp->getMask() == s) {
      i = mask_.erase(i);
      delete tmp;
      unlock();
      return;
    }
  }
  unlock();
}

// build one configuration
string
Peer_data::buildcfstr() {
  string str;
  lock();
  str = str + " peer " + peerhost_.getn() + ":" + utl::toString(port_) + "\n";
  str = str + "  description \'" + descr_ + "\'\n";
  str = str + "  cryptokey " + key_ + "\n";
  Peer_mask *tmp = 0;
  for(list<Peer_mask*>::const_iterator i=mask_.begin();i!=mask_.end();i++) {
    tmp = *i;
    str = str + "  mask " + tmp->getMask() + "\n";
  }
  str = str + "  exit";
  unlock();
  return str;
}

/////////////////// CorePeer ////////////////

// constructor
CorePeer::CorePeer(const string n,const string d) : Core_data(n,d,this) {
  pthread_mutex_init(&mutex_,0);
}

// destructor - for call virtual
CorePeer::~CorePeer() {
  Peer_data *tmp;
  lock();
  for(list<Peer_data*>::iterator i=pd_.begin();i!=pd_.end() && !pd_.empty();) {
    tmp = *i;
    i = pd_.erase(i);
    delete tmp;
  }
  pd_.clear();
  unlock();
  pthread_mutex_destroy(&mutex_);
}

void
CorePeer::lock() {
  pthread_mutex_lock(&mutex_);
}

void
CorePeer::unlock() {
  pthread_mutex_unlock(&mutex_);
}

// add peer
void
CorePeer::add(ipaddr peer,unsigned int port) {
  Peer_data *tmp;
  lock();
  for(list<Peer_data*>::const_iterator i=pd_.begin();i!=pd_.end();i++) {
    tmp = *i;
    if(tmp->getPeerhost()==peer && tmp->getPort()==port) {
      unlock();
      return;
    }
  }
  tmp = new Peer_data(peer,port);
  pd_.push_back(tmp);
  unlock();
}

// delete peer
void
CorePeer::del(ipaddr peer,unsigned int port) {
  Peer_data *tmp;
  lock();
  for(list<Peer_data*>::iterator i=pd_.begin();i!=pd_.end();i++) {
    tmp = *i;
    if(tmp->getPeerhost()==peer && tmp->getPort()==port) {
      i = pd_.erase(i);
      delete tmp;
      unlock();
      return;
    }
  }
  unlock();
}

// add mask
void
CorePeer::add_mask(ipaddr peer,unsigned int port,const char *s) {
  Peer_data *tmp;
  lock();
  for(list<Peer_data*>::const_iterator i=pd_.begin();i!=pd_.end();i++) {
    tmp = *i;
    if(tmp->getPeerhost()==peer && tmp->getPort()==port) {
      tmp->add_mask(s);
      unlock();
      return;
    }
  }
  unlock();
}

// delete mask
void
CorePeer::del_mask(ipaddr peer,unsigned int port,const char *s) {
  Peer_data *tmp;
  lock();
  for(list<Peer_data*>::const_iterator i=pd_.begin();i!=pd_.end();i++) {
    tmp = *i;
    if(tmp->getPeerhost()==peer && tmp->getPort()==port) {
      tmp->del_mask(s);
      unlock();
      return;
    }
  }
  unlock();
}

// change description
void
CorePeer::modify_des(ipaddr peer,unsigned int port,const char *s) {
  Peer_data *tmp;
  lock();
  for(list<Peer_data*>::const_iterator i=pd_.begin();i!=pd_.end();i++) {
    tmp = *i;
    if(tmp->getPeerhost()==peer && tmp->getPort()==port) {
      tmp->setDescr(s);
      unlock();
      return;
    }
  }
  unlock();
}

// change key value
void
CorePeer::modify_key(ipaddr peer,unsigned int port,const char *s) {
  Peer_data *tmp;
  lock();
  for(list<Peer_data*>::const_iterator i=pd_.begin();i!=pd_.end();i++) {
    tmp = *i;
    if(tmp->getPeerhost()==peer && tmp->getPort()==port) {
      tmp->setKey(s);
      unlock();
      return;
    }
  }
  unlock();
}

// build config for peer
string
CorePeer::buildcfstr(int n) {
  string str;
  lock();
  if((int)pd_.size() > n && !pd_.empty()) {
    int count = 0;
    Peer_data *tmp = 0;
    list<Peer_data*>::const_iterator i;
    for(i=pd_.begin();i!=pd_.end() && count<n;i++,count++);
    if(count==n) {
      tmp = *i;
      if(tmp) str = tmp->buildcfstr();
    }
  }
  unlock();
  return str;
}

// config applyer
string
CorePeer::applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd)throw (CmdErr) {
 string str;
//  cerr<<"---------------cmdName="<<cmdname<<endl;
    ipaddr peer;
    int port;

  if(cmdname=="confPeerNo") {
    peer.fill(utl::splitId(paramList.begin()->c_str(),1).c_str());
    port=atoi(utl::splitId(paramList.begin()->c_str(),2).c_str());
    del(peer,port);
  }
  else if(cmdname=="confPeerNode") {
    peer.fill(utl::splitId(paramList.begin()->c_str(),1).c_str());
    port=atoi(utl::splitId(paramList.begin()->c_str(),2).c_str());
    add(peer,port);
  }
  else {
    peer.fill(utl::splitId(cmd->upNode->paramList.begin()->c_str(),1).c_str());
    port=atoi(utl::splitId(cmd->upNode->paramList.begin()->c_str(),2).c_str());
    if(cmdname=="confPeerDescr") {
      modify_des(peer,port,paramList.begin()->c_str());
    }
    else if(cmdname=="confPeerCrypto") {
      modify_key(peer,port,paramList.begin()->c_str());
    }
    else if(cmdname=="confPeerMask") {
      add_mask(peer,port,paramList.begin()->c_str());
    }
    else if(cmdname=="confPeerNoMask") {
      del_mask(peer,port,paramList.begin()->c_str());
    }
  }

  return str;
}

//////////////////// that is all /////////////////
