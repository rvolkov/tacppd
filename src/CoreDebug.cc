// tacppd debugging core layer
// (c) Copyright 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

#include "global.h"

////// Debug_entry

Debug_entry::Debug_entry(int i,const char *n,int o,ipaddr ip) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  num_ = i;
  name_ = n;
  output_ = o;
  syslogserv_ = ip;
  if(output_ == 1)
    ::logFileSystem->add_debug_fc(num_); // add this is to debugging
  if(output_ == 2)
    ::logSyslogSystem->add_debug_fc((char*)ip.get().c_str(),num_);
  unlock();
}

Debug_entry::~Debug_entry() {
  lock();
  if(output_ == 1)
    if(::logFileSystem) ::logFileSystem->del_debug_fc(num_);
  if(output_ == 2)
    if(::logSyslogSystem)
      ::logSyslogSystem->del_debug_fc((char*)syslogserv_.get().c_str(),num_);
  unlock();
  pthread_mutex_destroy(&Mutex_);
}

void
Debug_entry::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
Debug_entry::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

int
Debug_entry::getNum() {
  lock();
  int ret = num_;
  unlock();
  return ret;
}

string
Debug_entry::getName() {
  lock();
  string ret = name_;
  unlock();
  return ret;
}

int
Debug_entry::getOutput() {
  lock();
  int ret = output_;
  unlock();
  return ret;
}

ipaddr
Debug_entry::getSyslogserv() {
  lock();
  ipaddr ret = syslogserv_;
  unlock();
  return ret;
}

////// Debug_data

Debug_data::Debug_data(int i,const char *a) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  output_ = i;
  syslogserv_ = a;
  unlock();
}

void
Debug_data::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
Debug_data::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

Debug_data::~Debug_data() {
  Debug_entry *tmp;
  lock();
  for(list<Debug_entry*>::iterator i=de_.begin();i!=de_.end() && !de_.empty();) {
    tmp = *i;
    i = de_.erase(i);
    delete tmp;
  }
  de_.clear();
  unlock();
  pthread_mutex_destroy(&Mutex_);
}

int
Debug_data::getOutput() {
  lock();
  int ret = output_;
  unlock();
  return ret;
}

ipaddr
Debug_data::getSyslogserv() {
  lock();
  ipaddr ret = syslogserv_;
  unlock();
  return ret;
}

// build configuration for one debug object
string
Debug_data::buildcfstr() {
  string str = "";
  lock();
  if(output_ == 1) str = str + " debug file\n";
  if(output_ == 2) {
    if(syslogserv_ == "127.0.0.1")
      str = str + " debug syslog\n";
    else
      str = str + " debug " + syslogserv_.getn() + "\n";
  }
  Debug_entry *tmp;
  for(list<Debug_entry*>::const_iterator i=de_.begin(); i!=de_.end() && !de_.empty(); i++) {
    tmp = *i;
    str = str + "  facility " + utl::lower(tmp->getName()) + "\n";
  }
  unlock();
  str = str + "  exit";
  return str;
}

// add facility number 'k' with name 'n'
void
Debug_data::add(int k,const char *n) {
  Debug_entry *tmp;
  lock();
  for(list<Debug_entry*>::const_iterator i=de_.begin(); i!=de_.end() && !de_.empty(); i++) {
    tmp = *i;
    if(tmp->getNum() == k) {
      unlock();
      return; // already here
    }
  }
  tmp = new Debug_entry(k,n,output_,syslogserv_);
  de_.push_back(tmp);
  unlock();
}

// del facility number 'n'
void
Debug_data::del(int n) {
  Debug_entry *tmp;
  lock();
  for(list<Debug_entry*>::iterator i=de_.begin(); i!=de_.end() && !de_.empty();) {
    tmp = *i;
    if(tmp->getNum() == n) {
      i = de_.erase(i);
      delete tmp;
      unlock();
      return;
    }
    i++;
  }
  unlock();
}

/////////////////////// CoreDebug /////////////////

// constructor
CoreDebug::CoreDebug(const string n,const string d) : Core_data(n,d,this) {
  pthread_mutex_init(&Mutex_,0);
}

//destructor
CoreDebug::~CoreDebug() {
  Debug_data *tmp;
  lock();
  for(list<Debug_data*>::iterator i=dd_.begin();i!=dd_.end() && !dd_.empty();) {
    tmp = *i;
    i = dd_.erase(i);
    delete tmp;
  }
  dd_.clear();
  unlock();
  pthread_mutex_destroy(&Mutex_);
}

void
CoreDebug::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
CoreDebug::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

// add
void
CoreDebug::add(int o,const char *ip) {
  Debug_data *tmp;
  lock();
  for(list<Debug_data*>::const_iterator i=dd_.begin();i!=dd_.end() && !dd_.empty(); i++) {
    tmp = *i;
//std::cerr<<"Check output="<<tmp->output<<" and syslog="<<tmp->syslogserv.getn()<<" with ip="<<ip<<std::endl;
    if((o==1 && tmp->getOutput()==1) ||
        (o==2 && tmp->getOutput()==2 && tmp->getSyslogserv()==ip)) { // already present
      unlock();
      return;
    }
  }
  tmp = new Debug_data(o,ip);
  dd_.push_back(tmp);
  unlock();
}

// add facility
void
CoreDebug::add(int o,const char *ip,int f) {
  Debug_data *tmp;
  lock();
  for(list<Debug_data*>::const_iterator i=dd_.begin();i!=dd_.end() && !dd_.empty(); i++) {
    tmp = *i;
    if((o==1 && tmp->getOutput()==1) ||
        (o==2 && tmp->getOutput()==2 && tmp->getSyslogserv()==ip)) {
      tmp->add(f,fname(f).c_str());
      unlock();
      return;
    }
  }
  unlock();
  add(o,ip);
  add(o,ip,f);
}

// del whole debug output
void
CoreDebug::del(int o,const char *ip) {
  Debug_data *tmp;
  lock();
  for(list<Debug_data*>::iterator i=dd_.begin();i!=dd_.end() && !dd_.empty();i++) {
    tmp = *i;
    if((o==1 && tmp->getOutput()==1) ||
        (o==2 && tmp->getOutput()==2 && tmp->getSyslogserv()==ip)) {
      i = dd_.erase(i);
      delete tmp;
      unlock();
      return;
    }
  }
  unlock();
  return;
}

// del only debug facility
void
CoreDebug::del(int o,const char *ip,int f) {
  Debug_data *tmp;
  lock();
  for(list<Debug_data*>::const_iterator i=dd_.begin();i!=dd_.end() && !dd_.empty();i++) {
    tmp = *i;
    if((o==1 && tmp->getOutput()==1) ||
        (o==2 && tmp->getOutput()==2 && tmp->getSyslogserv()==ip)) {
      tmp->del(f);
      unlock();
      return;
    }
  }
  unlock();
}

// create one configuration string
string
CoreDebug::buildcfstr(int n) {
  string str = "";
  lock();
  if((int)dd_.size()>n && !dd_.empty()) {
    Debug_data *tmp = 0;
    int count = 0;
    list<Debug_data*>::const_iterator i;
    for(i=dd_.begin();i!=dd_.end() && (count<n) && !dd_.empty();i++,count++);
    if(count==n) {
      tmp = *i;
      if(tmp) str = tmp->buildcfstr();
    }
  }
  unlock();
  return str;
}

string
CoreDebug::applycf(char * const *mask,const string cmdname,const list<string>paramList,Command *cmd)throw
(CmdErr) {
  string str = "";
  if(cmdname=="confDebugNoFile") {
    del(1,"127.0.0.1");
  } else if(cmdname=="confDebugNoIP") {
    del(2,paramList.begin()->c_str());
  } else if(cmdname=="confDebugNodeFile") {
    add(1,"127.0.0.1");
  } else if(cmdname=="confDebugNodeIP") {
    add(2,paramList.begin()->c_str());
  } else if(cmdname=="confDebugF") {
    add(1,"127.0.0.1",fnum(cmd->getCommand(1)));
  } else if(cmdname=="confDebugFIntNo") {
    del(1,"127.0.0.1",fnum(cmd->getCommand(2)));
  } else if(cmdname=="confDebugIP") {
    add(2,cmd->upNode->paramList.begin()->c_str(),fnum(cmd->getCommand(1)));
  } else if(cmdname=="confDebugIPIntNo") {
    del(2,cmd->upNode->paramList.begin()->c_str(),fnum(cmd->getCommand(2)));
  }
  return str;
}

////////////////////// that is all //////////////////////////////
