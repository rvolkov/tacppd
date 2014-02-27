// debug subsystem
// (c) Copyright 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: Debug.cc
// description: debug layer

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"

/////////////////////// Debug Descriptions /////////
DebugFacility::DebugFacility(int n,const string nm,const string ds) {
  pthread_mutex_init(&mutex_,0);
  lock();
  num_ = n;
  name_ = nm;
  description_ = ds;
  unlock();
}

DebugFacility::~DebugFacility() {
  pthread_mutex_destroy(&mutex_);
}

void
DebugFacility::lock() {
  pthread_mutex_lock(&mutex_);
}

void
DebugFacility::unlock() {
  pthread_mutex_unlock(&mutex_);
}

int
DebugFacility::getNum() {
  lock();
  int ret = num_;
  unlock();
  return ret;
}

string
DebugFacility::getName() {
  lock();
  string ret = name_;
  unlock();
  return ret;
}

string
DebugFacility::getDescription() {
  lock();
  string ret = description_;
  unlock();
  return ret;
}

/////////////////////// Debug information /////////////////

// constructor
Debug::Debug() {
  pthread_mutex_init(&mutex_,0);
  // add debug descriptions
  lock();
  ddf_.push_back(new DebugFacility(LOG_TACACS,"TACacs","tacacs+"));
  ddf_.push_back(new DebugFacility(LOG_BILLING,"BILling","billing system"));
  ddf_.push_back(new DebugFacility(LOG_AAA,"AAA","authentication/authorzation/accounting"));
  ddf_.push_back(new DebugFacility(LOG_DB,"DB","database system"));
  ddf_.push_back(new DebugFacility(LOG_SNMP,"SNmp","snmp activity"));
  ddf_.push_back(new DebugFacility(LOG_NETFLOW,"NETFlow","NetFlow collector system"));
  ddf_.push_back(new DebugFacility(LOG_SERVER,"SERver","TCP/UDP servers"));
  ddf_.push_back(new DebugFacility(LOG_CC,"CC","Crypto Card"));
  ddf_.push_back(new DebugFacility(LOG_POOL,"POOl","address pool"));
  ddf_.push_back(new DebugFacility(LOG_TRACK,"TRack","user tracking"));
  ddf_.push_back(new DebugFacility(LOG_HTTP,"HTtp","http interface"));
  ddf_.push_back(new DebugFacility(LOG_TIP,"TIP","tacppd interchange protocol"));
  ddf_.push_back(new DebugFacility(LOG_TEP,"TEP","tacppd exchange protocol"));
  unlock();
}

//destructor
Debug::~Debug() {
  DebugFacility *tmp;
  lock();
  for(list<DebugFacility*>::iterator i=ddf_.begin();i!=ddf_.end() && !ddf_.empty();) {
    tmp = *i;
    i = ddf_.erase(i);
    delete tmp;
  }
  ddf_.clear();
  unlock();
  pthread_mutex_destroy(&mutex_);
}

void
Debug::lock() {
  pthread_mutex_lock(&mutex_);
}

void
Debug::unlock() {
  pthread_mutex_unlock(&mutex_);
}

// get facility name from its number
string
Debug::fname(int f) {
  lock();
  string ostr;
  DebugFacility *tmp;
  for(list<DebugFacility*>::const_iterator i=ddf_.begin();i!=ddf_.end();i++) {
    tmp = *i;
    if(tmp->getNum() == f) {
      ostr = tmp->getName();
      unlock();
      return ostr;
    }
  }
  unlock();
  return ostr;
}

// get facility description from number
string
Debug::fdescr(int f) {
  lock();
  string ostr;
  DebugFacility *tmp;
  for(list<DebugFacility*>::const_iterator i=ddf_.begin();i!=ddf_.end();i++) {
    tmp = *i;
    if(tmp->getNum() == f) {
      ostr = tmp->getDescription();
      unlock();
      return ostr;
    }
  }
  unlock();
  return ostr;
}

// get facility number from it's name
int
Debug::fnum(const char *fn) {
  lock();
  DebugFacility *tmp;
  int n = 0;
  for(list<DebugFacility*>::const_iterator i=ddf_.begin();i!=ddf_.end();i++) {
    tmp = *i;
    if(utl::match(fn,tmp->getName().c_str())) {
      n = tmp->getNum();
      unlock();
      return n;
    }
  }
  unlock();
  return 0;
}

////////////////////// that is all //////////////////////////////
