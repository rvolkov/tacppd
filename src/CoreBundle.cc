// tacppd neighbors for share information in management domain
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

#include "global.h"

/////////////////// CoreBundle ///////////////////

CoreBundle::CoreBundle(const string n,const string d) : Core_data(n,d,this) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  mgroup_ = UNKNOWN_IP;
  keepalive_time_ = time(0);
  unlock();
}

CoreBundle::~CoreBundle() {
  pthread_mutex_destroy(&Mutex_);
}

void
CoreBundle::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
CoreBundle::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

bool
CoreBundle::isBundle() {
  bool ret;
  lock();
  if(mgroup_ == UNKNOWN_IP) ret = false;
  else ret = true;
  unlock();
  return ret;
}

ipaddr
CoreBundle::getMgroup() {
  lock();
  ipaddr ret = mgroup_;
  unlock();
  return ret;
}

unsigned int
CoreBundle::getPort() {
  lock();
  unsigned int ret = port_;
  unlock();
  return ret;
}

int
CoreBundle::getTtl() {
  lock();
  int ret = ttl_;
  unlock();
  return ret;
}

int
CoreBundle::getPri() {
  lock();
  int ret = priority_;
  unlock();
  return ret;
}

string
CoreBundle::getKey() {
  lock();
  string ret = key_;
  unlock();
  return ret;
}

void
CoreBundle::del() {
  lock();
  mgroup_ = UNKNOWN_IP;
  unlock();
}

void
CoreBundle::modifyMgroup(ipaddr ipa) {
  lock();
  mgroup_ = ipa;
  unlock();
}

void
CoreBundle::modifyPort(unsigned int p) {
  lock();
  port_ = p;
  unlock();
}

void
CoreBundle::modifyKey(const string key) {
  lock();
  key_ = key;
  unlock();
}

void
CoreBundle::modifyTtl(int ttl) {
  lock();
  ttl_ = ttl;
  unlock();
}

void
CoreBundle::modifyPri(int pri) {
  lock();
  priority_ = pri;
  unlock();
}

string
CoreBundle::buildcfstr(int i) {
  string str = "";
  if(i > 0) return str;
  if(!isBundle()) return str; // no bundle created
  lock();
  str = str + " bundle " + mgroup_.get() + ":" + utl::toString(port_) + "\n";
  str = str + "  cryptokey " + key_ + "\n";
  str = str + "  priority " + utl::toString(priority_) + "\n";
  str = str + "  ttl " + utl::toString(ttl_) + "\n";
  str = str + "  exit";
  unlock();
  return str;
}

string
CoreBundle::applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd)throw (CmdErr) {
  string str="";
//  cerr<<"---------------cmdName="<<cmdname<<endl;
  if(cmdname=="confBundleNo") {
    del();
  } else if(cmdname=="confBundleNode") {
//  ipaddr ipgroup(utl::splitId(v1,1).c_str());
//  int port = atoi(utl::splitId(v1,2).c_str());

    modifyMgroup(ipaddr(utl::splitId(paramList.begin()->c_str(),1).c_str()));
    modifyPort(atoi(utl::splitId(paramList.begin()->c_str(),2).c_str()));
  } else if(cmdname=="confBundleCrypto") {
    modifyKey(paramList.begin()->c_str());
  } else if(cmdname=="confBundleTTl") {
    modifyTtl(atoi(paramList.begin()->c_str()));
  } else if(cmdname=="confBundlePrio") {
    modifyPri(atoi(paramList.begin()->c_str()));
  }
   return str;
}

void
CoreBundle::periodic() {
  ::serverTrack.checkTtl(); // check tacppds in bundle
  time_t t = time(0);
  if(isBundle()) {
    if((t - keepalive_time_) >= 10) {
      TipSystem tipSystem;
      tipSystem.clear_dt();
      tipSystem.clear_ut();
      tipSystem.send_ka(); // send keepalive packet to bundle
      tipSystem.clear_ut();
      tipSystem.clear_dt();
      keepalive_time_ = t;
    }
  }
}


//////// BundleProcessor ///////////

BundleProcessor::BundleProcessor(ipaddr ip,unsigned int port, char *buf) {
  startlog(LOG_SERVER)<<"bundle connect from neighbor "<<ip<<" to port "<<port<<endlog;
  // search source address/port and store data
  if(!::coreData->getBundle()->isBundle()) { // no active bundle
    startlog(LOG_ERROR)<<"Try to conect from listener to unexisted bundle from "<<ip<<endlog;
    return;
  }
  if(port != ::coreData->getBundle()->getPort()) {
    startlog(LOG_ERROR)<<"Try to connect to wrong bundle port "<<port<<" from "<<ip<<endlog;
    return;
  }
  char key[128];
  snprintf(key,sizeof(key)-1,"%s",::coreData->getBundle()->getKey().c_str());
  // read and build data from buf
  TipSystem ts;
  int type = ts.packet_type(buf); // type of packet
//  int varkey = ts.packet_varkey(buf); // packet varkey
  int num = ts.packet_num(buf); // number of entryes
  int pri = ts.packet_pri(buf); // priority

//std::cerr<<"received bundle packet type="<<type<<" varkey="<<varkey<<" num="<<num<<" pri="<<pri<<std::endl;

  if(type == TIP_DT_PACKET) { // this is devicetrack packet
//std::cerr<<"received devicetrack packet, varkey="<<varkey<<" num="<<num<<std::endl;
    struct dt_pak_entry *dt = 0;
    for(int i=0; i<num; i++) {
      dt = ts.read_dt(buf,i,key);
      if(dt == 0) break; // this is bottom
      char ipa[16];
      struct in_addr a;
      memcpy((void*)&a,(void*)&dt->device,sizeof(struct in_addr));
      snprintf(ipa,sizeof(ipa)-1,"%s",inet_ntoa(a));
      ipaddr ipad(ipa);
//std::cerr<<"app: receive status="<<dt->status<<" for "<<ipa<<std::endl;
      bool status = (dt->status==1 ? true : false);
      delete dt;
      // set device status with external information weight
      ::coreData->getDevice()->setDeviceStatus(ipad,status);
      if(status == false) { // device go down
        ::coreData->getDevice()->usr_del(ipad);
      }
    }
    // update server track or add new server
    ::serverTrack.update(ip,pri);
  } else if(type == TIP_UT_PACKET) {
//std::cerr<<"received usertrack packet, varkey="<<varkey<<" num="<<num<<std::endl;
    struct ut_pak_entry *ut = 0;
    for(int i=0;i < num; i++) {
      ut = ts.read_ut(buf,i,key);
      if(ut == 0) break;      // this is bottom
      char ipdev[16];
      char ipusr[16];
      struct in_addr a;
      memcpy((void*)&a,(void*)&ut->device,sizeof(struct in_addr));
      snprintf(ipdev,sizeof(ipdev)-1,"%s",inet_ntoa(a));
      ipaddr ipa_dev(ipdev);
      memcpy((void*)&a,(void*)&ut->userip,sizeof(struct in_addr));
      snprintf(ipusr,sizeof(ipusr)-1,"%s",inet_ntoa(a));
      ipaddr ipa_usr(ipusr);
      bool status;
//std::cerr<<"app: receive status="<<ut->status<<" for "<<ut->username<<"/"<<ipdev<<"/"<<ipusr<<std::endl;
      status = (ut->status==1 ? true : false);
      string us(ut->username);
      string pr(ut->port);
      string ph(ut->phone);
      delete ut;
      if(status)
        ::userTrack.add(us,ipa_dev,pr,ph,ipa_usr); // add accepted users
      else
        ::userTrack.del(us,ipa_dev,pr); // delete user from active
    }
    // update server track or add new server
    ::serverTrack.update(ip,pri);
  } else if(type == TIP_KA_PACKET) {
//std::cerr<<"receive keepalive packet from "<<ip.get()<<std::endl;
    // update server track or add new server
    ::serverTrack.update(ip,pri);
  } else if(type == TIP_START_PACKET) {
//std::cerr<<"receive keepalive start packet from "<<ip.get()<<"("<<pri<<")"<<std::endl;
    // update server track or add new server
    ::serverTrack.update(ip,pri);
    // if we are active, than we should send updates to all devices/users,
    // but as we know that if information about users come, than we know,
    // that accosiated network device is active
    if(::serverTrack.isActive()) {
      ipaddr tmp;
      tmp = UNKNOWN_IP;
      DeviceUser_data utmp;
      // runs over all devices
      int i = 0;
      //for(int i=0;(tmp = ::coreData->getDevice()->getip(i))!=UNKNOWN_IP; i++) {
      while(true) {
        tmp = ::coreData->getDevice()->getip(i);
        if(tmp == UNKNOWN_IP) break;
        //if(tmp->getShutdown()) continue; // if shutdown - avoid

        ts.clear_dt();
        ts.add_dt(tmp,::coreData->getDevice()->get_device_status(tmp));
        ts.send_dt();
        ts.clear_dt();

        // about users
        ts.clear_ut();
        int j=0;
        while(true) {
          utmp = ::coreData->getDevice()->getuser(tmp,j);
          if(!utmp.isPresent()) break;
          ts.add_ut(utmp.getUsername(),tmp,utmp.getUserAddr(),
            utmp.getPort(),utmp.getPhone(),true);
          j++;
        }
        ts.send_ut();
        ts.clear_ut();
        //
        i++;
      }
    }
  } else {
//std::cerr<<"unknown bundle packet"<<std::endl;
    startlog(LOG_TIP)<<"received unknown bundle packet from "<<ip<<endlog;
  }
  // ok.
  return;
}

//////////////////////// that is all //////////////////////
