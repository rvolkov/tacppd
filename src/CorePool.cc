// address pools
// (c) Copyright 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: CorePool.cc
// description: address pool core layer

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"

//////////////// Addrpool_entry ///////////////

// constructor
Addrpool_entry::Addrpool_entry(ipaddr ip) {
  pthread_mutex_init(&mutex_,0);
  lock();
  address_ = ip;
  tag_ = true; // this address is free
  unlock();
}

void
Addrpool_entry::lock() {
  pthread_mutex_lock(&mutex_);
}

void
Addrpool_entry::unlock() {
  pthread_mutex_unlock(&mutex_);
}

Addrpool_entry::~Addrpool_entry() {
  pthread_mutex_destroy(&mutex_);
}

void
Addrpool_entry::setAddress(ipaddr d) {
  lock();
  address_ = d;
  unlock();
}

ipaddr
Addrpool_entry::getAddress() {
  lock();
  ipaddr ret = address_;
  unlock();
  return ret;
}

void
Addrpool_entry::setTag(bool d) {
  lock();
  tag_ = d;
  unlock();
}

bool
Addrpool_entry::getTag() {
  lock();
  bool ret = tag_;
  unlock();
  return ret;
}

///////////////// Addrpool_data ////////////////

// Addrpool_data constructor
Addrpool_data::Addrpool_data(int p) {
  pthread_mutex_init(&mutex_,0);
  lock();
  pool_ = p; // set pool number
  unlock();
}

void
Addrpool_data::lock() {
  pthread_mutex_lock(&mutex_);
}

void
Addrpool_data::unlock() {
  pthread_mutex_unlock(&mutex_);
}

int
Addrpool_data::getPool() {
  lock();
  int ret = pool_;
  unlock();
  return ret;
}

void
Addrpool_data::setPool(int d) {
  lock();
  pool_ = d;
  unlock();
}

// destructor
Addrpool_data::~Addrpool_data() {
  Addrpool_entry *tmp;
  lock();
  for(list<Addrpool_entry*>::iterator i=ae_.begin();i!=ae_.end() && !ae_.empty();) {
    tmp = *i;
    i = ae_.erase(i);
    delete tmp;
  }
  ae_.clear();
  unlock();
  pthread_mutex_destroy(&mutex_);
}

// add address to pool
void
Addrpool_data::add(ipaddr ip) {
  Addrpool_entry *tmp;
  lock();
  for(list<Addrpool_entry*>::const_iterator i=ae_.begin();i!=ae_.end();i++) {
    tmp = *i;
    if(tmp->getAddress() == ip) {
      unlock();
      return; // such addr already here
    }
  }
  tmp = new Addrpool_entry(ip);
  ae_.push_back(tmp);
  unlock();
}

// delete address from pool
void
Addrpool_data::del(ipaddr ip) {
  Addrpool_entry *tmp;
  lock();
  for(list<Addrpool_entry*>::iterator i=ae_.begin();i!=ae_.end();i++) {
    tmp = *i;
    if(tmp->getAddress() == ip) {
      i = ae_.erase(i);
      delete tmp;
      unlock();
      return;
    }
  }
  unlock();
}

// get from and occupy address in pool
string
Addrpool_data::get() {
  Addrpool_entry *tmp;
  ipaddr ip;
  lock();
  for(list<Addrpool_entry*>::const_iterator i=ae_.begin();i!=ae_.end();i++) {
    tmp = *i;
    if(tmp->getTag()) {
      tmp->setTag(false);
      ip = tmp->getAddress();
      unlock();
      return ip.get();
    }
  }
  unlock();
  return "";
}

// free address in pool
void
Addrpool_data::ret(ipaddr ip) {
  Addrpool_entry *tmp;
  lock();
  for(list<Addrpool_entry*>::const_iterator i=ae_.begin();i!=ae_.end();i++) {
    tmp = *i;
    if(tmp->getAddress() == ip) {
      tmp->setTag(true);
      unlock();
      return;
    }
  }
  unlock();
}

// build config string
string
Addrpool_data::buildcfstr() {
  string str;
  Addrpool_entry *tmp;
  lock();
  str = str + " pool " + utl::toString(pool_) + "\n";
  for(list<Addrpool_entry*>::const_iterator i=ae_.begin();i!=ae_.end();i++) {
    tmp = *i;
    str = str + "  addr " + tmp->getAddress().get() + "\n";
//    str = str + "  addr " + group() + "\n";
  }
  str = str + "  exit";
  unlock();
  return str;
}

string
Addrpool_data::group() {
  string str;
  Addrpool_entry *tmp1, *tmp2;
  ipaddr ip1, ip2;
  int a1, a2, a3, a4;
  int b1, b2, b3, b4;
  for(list<Addrpool_entry*>::const_iterator i=ae_.begin();i!=ae_.end();i++) {
    tmp1 = *i;
    ip1 = tmp1->getAddress();
    a1 = ip1.get(0);
    a2 = ip1.get(1);
    a3 = ip1.get(2);
    a4 = ip1.get(3);
    i++;
    tmp2 = *i;
    if(i==ae_.end()) {
      str = str + "," + ip1.get() + "," + tmp2->getAddress().get();
      break;
    }
    ip2 = tmp2->getAddress();
    b1 = ip2.get(0);
    b2 = ip2.get(1);
    b3 = ip2.get(2);
    b4 = ip2.get(3);
    for(;i!=ae_.end() && a1==b1 && a2==b2 && a3==b3; i++) {
      tmp2 = *i;
      ip2 = tmp2->getAddress();
      b1 = ip2.get(0);
      b2 = ip2.get(1);
      b3 = ip2.get(2);
      b4 = ip2.get(3);
    }
    if(a1==b1 && a2==b2 && a3==b3) {
        str = str + "," + ip1.get() + "-" + tmp2->getAddress().get();
    } else {
      str = str + "," + ip1.get() + "," + tmp2->getAddress().get();
    }
  }
  return str;
}

////////////// CorePool /////////////

// translate addr strings like 192.168.10.1-192.168.10.11,192.168.11.1
// to series of single addresses
string
CorePool::getAddr(const char *str,int n) {
  string ret;

  char s1[100];
  s1[0] = 0;
  char s2[100];
  s2[0] = 0;
  n++;
  int k = 0;

  int d1 = 0, d2 = 0, d3 = 0;
  int i = 0;
  int j = 0;

  while(str[i]) {
    j = 0;
    while(str[i]!=',' && str[i]!='-' && str[i]!=0) s1[j++] = str[i++];
    s1[j] = 0;
    if(str[i] == 0 || str[i] == ',') {
      k++;
      if(k == n) {
        ret = s1;
        return ret;
      }
    }
    if(str[i] == '-') {
      i++;
      j = 0;
      while(str[i]!=',' && str[i]!='-' && str[i]!=0) s2[j++] = str[i++];
      s2[j] = 0;
      if(strlen(s1)>0 && strlen(s2)>0) {
        ipaddr ip1(s1);
        ipaddr ip2(s2);
        d1 = ip1.get(0);
        d2 = ip1.get(1);
        d3 = ip1.get(2);
        int left = ip1.get(3);
        int right = ip2.get(3);
        // we have two boundary addresses,
        // create addresses between
        for(int p = left; p <= right; p++) {
          k++;
          if(k == n) {
            ret = ret + utl::toString(d1) + "." + utl::toString(d2) + "." + utl::toString(d3) + "." + utl::toString(p);
            if(utl::isIp((char*)ret.c_str())) return ret;
            return "";
          }
        }
      } else {      // single address
        k++;
        if(k == n) {
          ret = s1;
          if(utl::isIp((char*)ret.c_str())) return ret;
          return "";
        }
      }
    }
  }
  return "";
}

// Addrpool constructor
CorePool::CorePool(const string n,const string d) : Core_data(n,d,this) {
  pthread_mutex_init(&mutex_,0);
}

// destructor
CorePool::~CorePool() {
  Addrpool_data *tmp = 0;
  lock();
  for(list<Addrpool_data*>::iterator i=ap_.begin();i!=ap_.end() && !ap_.empty();) {
    tmp = *i;
    i = ap_.erase(i);
    delete tmp;
  }
  ap_.clear();
  unlock();
  pthread_mutex_destroy(&mutex_);
}

void
CorePool::lock() {
  pthread_mutex_lock(&mutex_);
}

void
CorePool::unlock() {
  pthread_mutex_unlock(&mutex_);
}

// add addresses to pool
// control for avoid duplicate addresses
void
CorePool::add(int p,const char *a) {
  char r[16];
  Addrpool_data *tmp = 0;
//  bool aaa = false;
  lock();
AddPoolAgain:
  for(list<Addrpool_data*>::iterator i=ap_.begin();i!=ap_.end();i++) {
    tmp = *i;
    if(tmp->getPool() == p) break;
  }
  if(!tmp || tmp->getPool()!=p) {      // add new pool
    tmp = new Addrpool_data(p);
    ap_.push_back(tmp);
    goto AddPoolAgain;
  }
  snprintf(r,sizeof(r),"%s",getAddr(a,0).c_str());
  for(int i=0; strlen(r)>0; i++) {
    ipaddr ip(r);
    startlog(LOG_POOL)<<"add address "<<r<<" to pool "<<p<<endlog;
    tmp->add(ip);
    snprintf(r,sizeof(r),"%s",getAddr(a,i).c_str());
  }
  unlock();
}

// delete whole pool
void
CorePool::del(int p) {
  Addrpool_data *tmp = 0;
  lock();
  for(list<Addrpool_data*>::iterator i=ap_.begin();i!=ap_.end() && !ap_.empty();) {
    tmp = *i;
    if(tmp->getPool() == p) {
      i = ap_.erase(i);
      delete tmp;
      continue;
    }
    i++;
  }
  unlock();
}

// delete addresses from pool
// if unknown address - no action
void
CorePool::del(int p,const char *a) {
  Addrpool_data *tmp = 0;
  char r[16];
  lock();
  snprintf(r,sizeof(r),"%s",getAddr(a,0).c_str());
  for(int i=0; strlen(r)>0; i++) {
    ipaddr ip(r);
    for(list<Addrpool_data*>::const_iterator j=ap_.begin();j!=ap_.end();j++) {
      tmp = *j;
      if(tmp->getPool() == p) tmp->del(ip);
    }
    snprintf(r,sizeof(r),"%s",getAddr(a,i).c_str());
  }
  unlock();
}

// get addr from pool
string
CorePool::get(int p) {
  string addr;
  Addrpool_data *tmp;
  lock();
  for(list<Addrpool_data*>::const_iterator i=ap_.begin();i!=ap_.end();i++) {
    tmp = *i;
    if(tmp->getPool() == p) {
      addr = tmp->get();
      unlock();
      if(addr.size() == 0)
        startlog(LOG_ERROR)<<"No free addresses in pool "<<p<<endlog;
      return addr;
    }
  }
  unlock();
  startlog(LOG_ERROR)<<"No address pool "<<p<<", no addresses available"<<endlog;
  return addr;
}

// return address to pool
void
CorePool::ret(ipaddr ip) {
  Addrpool_data *tmp;
  lock();
  for(list<Addrpool_data*>::const_iterator i=ap_.begin();i!=ap_.end();i++) {
    tmp = *i;
    tmp->ret(ip);
  }
  unlock();
}

// build config string
string
CorePool::buildcfstr(int n) {
  lock();
  string str;
  if((int)ap_.size()>n && !ap_.empty()) {
    Addrpool_data *tmp = 0;
    list<Addrpool_data*>::const_iterator i;
    int count = 0;
    for(i=ap_.begin();i!=ap_.end() && count!=n;i++,count++);
    if(count == n) {
      tmp = *i;
      if(tmp) str = tmp->buildcfstr();
    }
  }
  unlock();
  return str;
}

string
CorePool::applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd)throw (CmdErr) {
 string str;
//  cerr<<"---------------cmdName="<<cmdname<<endl;
  if(cmdname=="confPoolNo") {
    startlog(LOG_EVENT)<<"Deleting whole address pool "<<paramList.begin()->c_str()<<endlog;
    del(atoi(paramList.begin()->c_str()));
  } else if(cmdname=="confPoolNode") {
  } else if(cmdname=="confPoolAddr") {
    add(atoi(paramList.begin()->c_str()),cmd->upNode->paramList.begin()->c_str());
  } else if(cmdname=="confPoolNoAddr") {
    startlog(LOG_EVENT)<<"Deleting address "<<paramList.begin()->c_str()<<" from pool "<<cmd->upNode->paramList.begin()->c_str()<<endlog;
    del(atoi(cmd->upNode->paramList.begin()->c_str()),paramList.begin()->c_str());       // delete from pool
  }
  return "";
}
// apply configuration

////////////////////// end of address pool /////////////////////////
