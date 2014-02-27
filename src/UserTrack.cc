// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: UserTrack.cc
// description: active users tracker

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"


///// dbc_entry

dbc_entry::dbc_entry(db_user d) {
  pthread_mutex_init(&mutex_,0);
  lock();
  entry_ = d;
  date_ = time(0);
  unlock();
}

dbc_entry::~dbc_entry() {
  pthread_mutex_destroy(&mutex_);
}

void
dbc_entry::lock() {
  pthread_mutex_lock(&mutex_);
}

void
dbc_entry::unlock() {
  pthread_mutex_unlock(&mutex_);
}

db_user
dbc_entry::get() {
  lock();
  db_user ret = entry_;
  unlock();
  return ret;
}

time_t
dbc_entry::getTime() {
  lock();
  time_t ret = date_;
  unlock();
  return ret;
}


//// dbc_man

dbc_man::dbc_man() {
  pthread_mutex_init(&mutex_dbc_,0);
}

dbc_man::~dbc_man() {
  lock();
  dbc_entry *tmp;
  for(list<dbc_entry*>::iterator i=dbec_.begin();i!=dbec_.end() && !dbec_.empty();) {
    tmp = *i;
    i = dbec_.erase(i);
    delete tmp;
  }
  unlock();
  pthread_mutex_destroy(&mutex_dbc_);
}

void
dbc_man::lock() {
  pthread_mutex_lock(&mutex_dbc_);
}

void
dbc_man::unlock() {
  pthread_mutex_unlock(&mutex_dbc_);
}

bool
dbc_man::present(const string u) {
  lock();
  dbc_entry *tmp;
  bool ret = false;
  if(dbec_.size() > 0) {
    time_t ppp = time(0);
    for(list<dbc_entry*>::iterator i=dbec_.begin();i!=dbec_.end();) {
      tmp = *i;
      if((ppp - tmp->getTime()) > 31) { // entry expired
        i = dbec_.erase(i);
        delete tmp;
        continue;
      }
      if(tmp->get().getUsername() == u) {
        ret = true;
        break;
      }
      i++;
    }
  }
  unlock();
  return ret;
}

db_user
dbc_man::get(const string u) {
  dbc_entry *tmp;
  db_user ret;
  db_user t;
  lock();
  for(list<dbc_entry*>::const_iterator i=dbec_.begin();i!=dbec_.end();i++) {
    tmp = *i;
    t = tmp->get();
    if(t.getUsername() == u) {
      ret = t;
      break;
    }
  }
  unlock();
  return ret;
}

void
dbc_man::add(const db_user d) {
  dbc_entry *tmp = new dbc_entry(d);
  lock();
  dbec_.push_back(tmp);
  unlock();
}


/////////////////// UserTrack /////////////////////

// constructor
UserTrack::UserTrack() {
  pthread_mutex_init(&mutex_,0);
}

// mutex lock
void
UserTrack::lock() {
  pthread_mutex_lock(&mutex_);
}

// mutex unlock
void
UserTrack::unlock() {
  pthread_mutex_unlock(&mutex_);
}

UserTrack::~UserTrack() {
  pthread_mutex_destroy(&mutex_);
}

// check by list, if not, create new entry, fill it from database,
// return information from database and set it in inactive state
db_user
UserTrack::get2add(string u,ipaddr nas,string p,string t) {
  db_user ret;
  // check in list of currently available users
//  lock(); // !!!!!!!!! have to remove
  ret = ::coreData->getBilling()->getUd(u);
//  unlock(); // !!!!!!!!! have to remove
  if(ret.getResult()) {
    // ok, if this is not this port/nas, we should add it to track
//    lock();
    if(::coreData->getDevice()->usr_add(nas,u,p,t,UNKNOWN_IP,ret)) {
      startlog(LOG_TRACK)<<"add1 inact user "<<u<<"/"<<p<<" to list"<<endlog;
    }
//    unlock();
    return ret;
  }
  // ok, here if this user is not created in list
  bool upresent = false;
  if(dbc_.present(u)) {
    upresent = true;
  }
  ret = dbc_.get(u);
  if(upresent == false) {
//    lock();
    ret = ::coreData->getDb()->get(u); // read from database
    dbc_.add(ret);
//    unlock();
  }
  if(!ret.getResult()) {
    startlog(LOG_INFORM)<<"cant add user to userTrack - no user in database: "<<u<<"/"<<nas<<"/"<<p<<endlog;
    return ret; // no entry or closed, no any operation
  }
  // check for other active user in this place
//  lock();
  if(::coreData->getDevice()->usr_del(nas,p)) {
    startlog(LOG_TRACK)<<"get2add username mismatch - delete old entry "<<endlog;
  }
  // add this user to list (as inactive)
  if(::coreData->getDevice()->usr_add(nas,u,p,t,UNKNOWN_IP,ret)) {
    startlog(LOG_TRACK)<<"add2 inact user "<<u<<"/"<<p<<" to list"<<endlog;
  }
//  unlock();
  return ret;
}

// add user to list or simply renew TTL or set it active
bool
UserTrack::add(string u,ipaddr c,string p,string t,ipaddr ua) {
  // run across all network devices
//  lock();
  if(::coreData->getDevice()->usr_activate(c,u,p,t,ua)) {
//    unlock();
    return true;
  }
//  unlock();
  db_user d = get2add(u,c,p,t); // add this username as inactive
  if(d.getResult() && d.getOpen()) { // only if we can add this user
//    lock();
    if(::coreData->getDevice()->usr_activate(c,u,p,t,ua)) {
//      unlock();
      return true;
    }
//    unlock();
    return true;
  }
  return false;
}

// delete from queue
bool
UserTrack::del(string u,ipaddr c,string p) {
  bool res = false;
//  lock();
  if(::coreData->getDevice()->usr_del(c,p)) {
    res = true;
    startlog(LOG_TRACK)<<"del user "<<u<<"/"<<c<<"/"<<p<<" from tracking system"<<endlog;
  }
//  unlock();
  return res;
}

// control system process
// checks by time and request to snmpSystem for drop, only if this system active
void
UserTrack::periodic() {
  // check by time
//  lock();
  ::coreData->getDevice()->usr_expire();
  time_t t = time(0);
  ::coreData->getDevice()->usr_periodic(t);
//  unlock();
}

///////////////// that's all ////////////////////
