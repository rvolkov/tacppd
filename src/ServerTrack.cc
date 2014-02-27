// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: ServerTrack.cc
// description: tacppd servers tracker

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"

/////////////////// ServerTrack_data /////////////

ServerTrack_data::~ServerTrack_data() {
  pthread_mutex_destroy(&mutex_);
}

void
ServerTrack_data::lock() {
  pthread_mutex_lock(&mutex_);
}

void
ServerTrack_data::unlock() {
  pthread_mutex_unlock(&mutex_);
}

ServerTrack_data::ServerTrack_data(ipaddr ipa, int p) {
  pthread_mutex_init(&mutex_,0);
  lock();
  ip_ = ipa;
  priority_ = p;
  entry_time_ = time(0);
  unlock();
}

ServerTrack_data::ServerTrack_data(int p) {
  pthread_mutex_init(&mutex_,0);
  lock();
  ip_ = UNKNOWN_IP;
  priority_ = p;
  entry_time_ = time(0);
  unlock();
}

void
ServerTrack_data::update(int p) {
  lock();
  entry_time_ = time(0);
  priority_ = p;
  unlock();
}

time_t
ServerTrack_data::getTime() {
  lock();
  time_t ret = entry_time_;
  unlock();
  return ret;
}

ipaddr
ServerTrack_data::getIp() {
  lock();
  ipaddr ret = ip_;
  unlock();
  return ret;
}

int
ServerTrack_data::getPri() {
  lock();
  int ret = priority_;
  unlock();
  return ret;
}


/////////////////// ServerTrack /////////////////////

ServerTrack::ServerTrack() {
  pthread_mutex_init(&Mutex_,0);
}

ServerTrack::~ServerTrack() {
  lock();
  ServerTrack_data *tmp;
  tmp = 0;
  for(list<ServerTrack_data*>::iterator i=csrv_.begin();i!=csrv_.end() && !csrv_.empty();) {
    tmp = *i;
    i = csrv_.erase(i);
    delete tmp;
  }
  csrv_.clear();
  unlock();
  pthread_mutex_destroy(&Mutex_);
}

void
ServerTrack::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
ServerTrack::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

void
ServerTrack::sendStart() {
  lock();
  ServerTrack_data *tmp;
  tmp = 0;
  // add this server to list
  tmp = new ServerTrack_data(::coreData->getBundle()->getPri());
  csrv_.push_back(tmp);
  TipSystem ts;
  if(::coreData->getBundle()->isBundle()) ts.send_start();
  unlock();
}

void
ServerTrack::checkTtl() {
  lock();
  ServerTrack_data *tmp;
  tmp = 0;
  time_t ttt = time(NULL);
  ipaddr localip;
  for(list<ServerTrack_data*>::iterator i=csrv_.begin();i!=csrv_.end();) {
    tmp = *i;
    // we never delete localhost
    if(tmp->getIp()==localip || tmp->getIp()==UNKNOWN_IP) {
      i++;
      continue;
    }
    if((ttt - tmp->getTime()) > 31) { // 3 keepalive intervals + 1 sec
      i = csrv_.erase(i);
      startlog(LOG_EVENT)<<"Bundle server "<<tmp->getIp()<<" set as inactive"<<endlog;
      delete tmp;
      continue;
    }
    i++;
  }
  unlock();
}

void
ServerTrack::update(ipaddr ipa,int p) {
  lock();
  ServerTrack_data *tmp;
  tmp = 0;
  bool is_pres = false;
  for(list<ServerTrack_data*>::const_iterator i=csrv_.begin();i!=csrv_.end(); i++) {
    tmp = *i;
    if(tmp->getIp()==ipa || tmp->getIp()==UNKNOWN_IP) {
      tmp->update(p); // update ttl and priority for this server
      is_pres = true;
      break;
    }
  }
  if(!is_pres) { // add new server to list
    tmp = new ServerTrack_data(ipa,p);
    csrv_.push_back(tmp);
  }
  unlock();
}

bool
ServerTrack::isActive() {
  lock();
  ServerTrack_data *tmp;
  tmp = 0;
  ipaddr localip;
  ipaddr activeip;
  int activepri = 0;
  for(list<ServerTrack_data*>::const_iterator i=csrv_.begin();i!=csrv_.end(); i++) {
    tmp = *i;
    if(activepri < tmp->getPri()) {
      activepri = tmp->getPri();
      activeip = tmp->getIp();
    }
  }
  unlock();
  if(activeip==localip || activeip==UNKNOWN_IP) return true;
  return false;
}

///////////////// that's all ////////////////////
