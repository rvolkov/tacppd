// tacppd network devices core layer
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

#include "global.h"

/////////////////// DeviceUser_data //////////////

DeviceUser_data::DeviceUser_data(const string u,const string p,
    const string t,const ipaddr n,const ipaddr ua,const db_user d) {
  pthread_mutex_init(&mutex_,0);
  lock();
  present_ = true;
  username_ = u;
  port_ = p; // user port
  phone_ = t; // user phone
  ::coreData->getBilling()->addUd(d); // add user database information
  active_ = false; // set as inactive
  lastdatatime_ = time(NULL);
  nasaddr_ = n; // nas ip addr
  useraddr_ = ua; // user ip addr
  unlock();
}

DeviceUser_data::DeviceUser_data() {
  pthread_mutex_init(&mutex_,0);
  lock();
  present_ = false;
  active_ = false; // set as inactive
  lastdatatime_ = time(0);
  unlock();
}

DeviceUser_data::~DeviceUser_data() {
  lock();
  ::coreData->getBilling()->delUd(username_);
  unlock();
  pthread_mutex_destroy(&mutex_);
}

void
DeviceUser_data::lock() {
  pthread_mutex_lock(&mutex_);
}

void
DeviceUser_data::unlock() {
  pthread_mutex_unlock(&mutex_);
}

void
DeviceUser_data::update(const string p,const string t,const ipaddr ua) {
  lock();
  port_ = p;
  if(t.size() > phone_.size()) phone_ = t; // if string longer, than information more
  if((ipaddr)ua != UNKNOWN_IP && (ipaddr)ua != "0.0.0.0") useraddr_ = ua;
  lastdatatime_ = time(NULL);
  unlock();
}

void
DeviceUser_data::update() {
  lock();
  lastdatatime_ = time(NULL);
  unlock();
}

void
DeviceUser_data::setActive() {
  lock();
  active_ = true;
  unlock();
}

bool
DeviceUser_data::isActive() {
  lock();
  bool ret = active_;
  unlock();
  return ret;
}

bool
DeviceUser_data::isExpired() {
  lock();
  bool ret;
  if(active_) {
    ret = false;
    unlock();
    return ret;
  }
  time_t t = time(NULL) - lastdatatime_;
  if(t > 61) ret = true; // inactive entry expire
  else ret = false;
  unlock();
  return ret;
}

bool
DeviceUser_data::isPresent() {
  lock();
  bool ret = present_;
  unlock();
  return ret;
}

void
DeviceUser_data::setPresent(const bool d) {
  lock();
  present_ = d;
  unlock();
}

string
DeviceUser_data::getUsername() {
  lock();
  string ret = username_;
  unlock();
  return ret;
}

bool
DeviceUser_data::compareUsername(const string u) {
  lock();
  bool ret = false;
  if(username_ == u) ret = true;
  unlock();
  return ret;
}

string
DeviceUser_data::getPort() {
  lock();
  string ret = port_;
  unlock();
  return ret;
}

bool
DeviceUser_data::comparePort(const string p) {
  lock();
  bool ret = false;
  if(port_ == p) ret = true;
  unlock();
  return ret;
}

string
DeviceUser_data::getPhone() {
  lock();
  string ret = phone_;
  unlock();
  return ret;
}

ipaddr
DeviceUser_data::getUserAddr() {
  lock();
  ipaddr ret = useraddr_;
  unlock();
  return ret;
}

db_user
DeviceUser_data::getDbData() {
  lock();
  db_user ret = ::coreData->getBilling()->getUd(username_);
  unlock();
  return ret;
}

ipaddr
DeviceUser_data::getNasAddr() {
  lock();
  ipaddr ret = nasaddr_;
  unlock();
  return ret;
}

bool
DeviceUser_data::compareNasAddr(const ipaddr a) {
  lock();
  bool ret = false;
  if(nasaddr_ == a) ret = true;
  unlock();
  return ret;
}

bool
DeviceUser_data::match_time(const time_t t) {
  lock();
  db_user ddata_ = ::coreData->getBilling()->getUd(username_);
  if(!ddata_.getResult()) {
    unlock();
    return true;
  }
  bool ret = false;
  // check by open/close time
  if(ddata_.getOpen()>t || (ddata_.getClose()<t && ddata_.getClose()>100)) {
    unlock();
    return false;
  }
  // check by table checkfrom
  db_user_acc acc;
  for(int k = 0; k < ddata_.acc_size(); k++) {
    acc = ddata_.acc_get(k);
    if(utl::check_time((char*)acc.getTime().c_str())) { // check by time
      ret = true;
      break;
    } else ret = false;
  }
  unlock();
  return ret;
}

void
DeviceUser_data::operator =(DeviceUser_data a) {
  lock();
  a.lock();
  present_ = a.present_;
  username_ = a.username_;
  port_ = a.port_;
  phone_ = a.phone_;
  db_user ddata_ = ::coreData->getBilling()->getUd(a.username_);
  ::coreData->getBilling()->addUd(ddata_);
  active_ = a.active_;
  lastdatatime_ = a.lastdatatime_;
  nasaddr_ = a.nasaddr_;
  useraddr_ = a.useraddr_;
  a.unlock();
  unlock();
}

/////////////////// Device_data //////////////////

Device_data::Device_data(const ipaddr h) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  dl_ = 0;
  hostaddr_ = h;
  community_ = "public";
  descr_ = "unknown";
  module_ = "none.so";
  tac_key_ = "tacacs";
  loginstring_ = "login:";
  pwdstring_ = "password:";
  counter_ = false;
  delay_ = 60; // sec
  shutdown_ = true; // disabled as default
  snmppolling_ = false;
  icmppolling_ = false;
  def_author_ = false; // no permit default authorization - security!
  rest_ = 0;
  status_ = false;
  unlock();
}

Device_data::Device_data() {
  Device_data(UNKNOWN_IP);
}

// destructor
Device_data::~Device_data() {
  lock();
  if(dl_) delete dl_; // remove snmp loadable module
  DeviceUser_data *tmp;
  // remove all user information from device
  for(list<DeviceUser_data*>::iterator i=cusr_.begin();i!=cusr_.end() && !cusr_.empty();) {
    tmp = *i;
    i = cusr_.erase(i);
    delete tmp;
  }
  cusr_.clear();
  unlock();
  pthread_mutex_destroy(&Mutex_);
}

void
Device_data::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
Device_data::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

DeviceUser_data
Device_data::gets(const int n) {
  lock();
  DeviceUser_data ret;
  DeviceUser_data *tmp = 0;
  if(cusr_.size() > 0 && (int)cusr_.size() > n) {
    int count = 0;
    for(list<DeviceUser_data*>::const_iterator i=cusr_.begin();i!=cusr_.end() && !cusr_.empty();i++,count++) {
      if(count==n) {
        tmp = *i;
        break;
      }
    }
  }
  if(tmp) ret = *tmp;
  unlock();
  return ret;
}

DeviceUser_data
Device_data::gets(ipaddr a) {
  lock();
  DeviceUser_data ret;
  DeviceUser_data *tmp = 0;
  if(cusr_.size() > 0) {
    for(list<DeviceUser_data*>::const_iterator i=cusr_.begin();i!=cusr_.end() && !cusr_.empty();i++) {
      tmp = *i;
      if(tmp->getUserAddr() == a) break;
      else tmp = 0;
    }
  }
  if(tmp) ret = *tmp;
  unlock();
  return ret;
}

/*
DeviceUser_data*
Device_data::erase(const int n) {
  lock();
  DeviceUser_data *tmp = 0;
  if(cusr_.size() > 0 && (int)cusr_.size() > n) {
    int count = 0;
    for(list<DeviceUser_data*>::iterator i=cusr_.begin();i!=cusr_.end();i++,count++) {
      if(count == n) {
        tmp = *i;
        i = cusr_.erase(i);
        break;
      }
    }
  }
  unlock();
  return tmp;
}
*/

/*
DeviceUser_data*
Device_data::erase(const string un,const string prt,const ipaddr d) {
  lock();
  DeviceUser_data *tmp = 0;
  bool match = false;
  for(list<DeviceUser_data*>::iterator i=cusr_.begin();i!=cusr_.end() && !cusr_.empty();i++) {
    tmp = *i;
    if(tmp->compareUsername(un) && tmp->comparePort(prt) && tmp->compareNasAddr(d)) {
      i = cusr_.erase(i);
      match = true;
      break;
    }
  }
  if(!match) tmp = 0;
  unlock();
  return tmp;
}
*/

bool
Device_data::add(DeviceUser_data *tmp) {
  lock();
  DeviceUser_data *utmp = 0;
  bool eq = false;
  for(list<DeviceUser_data*>::iterator i=cusr_.begin();i!=cusr_.end() && !cusr_.empty();i++) {
    utmp = *i;
    if(utmp->compareNasAddr(tmp->getNasAddr()) &&
      utmp->compareUsername(tmp->getUsername()) &&
        utmp->comparePort(tmp->getPort())) {
      eq = true;
      break;
    }
  }
  bool ret = false;
  if(!eq) {
    cusr_.push_back(tmp);
    ret = true;
  }
  unlock();
  return ret;
}

void
Device_data::usr_expire() {
  DeviceUser_data *utmp = 0;
  lock();
  for(list<DeviceUser_data*>::iterator i=cusr_.begin();i!=cusr_.end() && !cusr_.empty();) {
    utmp = *i;
    if(!utmp->isActive() && utmp->isExpired()) {
      startlog(LOG_TRACK)<<"del user "<<utmp->getUsername()<<
        "/"<<utmp->getNasAddr()<<"/"<<utmp->getPort()<<
          " as expired inactive"<<endlog;
      i = cusr_.erase(i);
      delete utmp;
      continue;
    }
    i++;
  }
  unlock();
}

bool
Device_data::usr_del(const string p) {
  DeviceUser_data *utmp = 0;
  bool res = false;
  lock();
  for(list<DeviceUser_data*>::iterator i=cusr_.begin();i!=cusr_.end() && !cusr_.empty();) {
    utmp = *i;
    if(utmp->comparePort(p)) {
      startlog(LOG_TRACK)<<"delete user entry "<<
        utmp->getUsername()<<"("<<utmp->getNasAddr()<<"/"<<utmp->getPort()<<")"<<endlog;
      i = cusr_.erase(i);
      delete utmp;
      res = true;
      break;
    }
    i++;
  }
  unlock();
  return res;
}

void
Device_data::usr_del() {
  DeviceUser_data *utmp = 0;
  lock();
  for(list<DeviceUser_data*>::iterator i=cusr_.begin();i!=cusr_.end() && !cusr_.empty();) {
    utmp = *i;
    i = cusr_.erase(i);
    delete utmp;
  }
  unlock();
}

bool
Device_data::usr_activate(const string u,const string p,const string t,const ipaddr ua) {
  DeviceUser_data *utmp = 0;
  bool res = false;
  lock();
  for(list<DeviceUser_data*>::iterator i=cusr_.begin();i!=cusr_.end() && !cusr_.empty();) {
    utmp = *i;
    if(utmp->comparePort(p)) {
      if(utmp->compareUsername(u)) {  // that user present
        startlog(LOG_TRACK)<<"update "<<utmp->getUsername()<<"/"<<
          utmp->getNasAddr()<<"/"<<utmp->getPort()<<" activate if not"<<endlog;
        utmp->update(p,t,ua); // do update
        utmp->setActive(); // set this entry active
        res = true;
        break;
      } else {
        startlog(LOG_TRACK)<<"add username mismatch - delete old entry "<<
          utmp->getUsername()<<"("<<utmp->getNasAddr()<<"/"<<utmp->getPort()<<")"<<endlog;
        i = cusr_.erase(i);
        delete utmp;
        res = false;
        break;
      }
    }
    i++;
  }
  unlock();
  return res;
}

bool
Device_data::usr_add(const ipaddr n,const string u,const string p,const string t,const ipaddr a,const db_user d) {
  bool ret = false;
  DeviceUser_data *utmp = 0;
  bool present = false;
  lock();
  for(list<DeviceUser_data*>::const_iterator i=cusr_.begin();i!=cusr_.end() && !cusr_.empty();i++) {
    utmp = *i;
    if(utmp->comparePort(p) && utmp->compareUsername(u)) {
      present = true;
      break;
    }
  }
  if(!present) {
    utmp = new DeviceUser_data(u,p,t,n,a,d);
    cusr_.push_back(utmp);
    ret = true;
  }
  unlock();
  return ret;
}

void
Device_data::usr_periodic(const time_t t) {
  lock();
  DeviceUser_data *utmp = 0;
  for(list<DeviceUser_data*>::const_iterator i=cusr_.begin();i!=cusr_.end() && !cusr_.empty();i++) {
    utmp = *i;
    if(!utmp->match_time(t)) {
      startlog(LOG_TRACK)<<"request to clear user: "<<
         utmp->getUsername()<<","<<utmp->getNasAddr()<<","<<utmp->getPort()<<","<<
           utmp->getPhone()<<endlog;
      char saddr[20];
      snprintf(saddr,sizeof(saddr)-1,"%s",utmp->getNasAddr().get().c_str());
      char sport[40];
      snprintf(sport,sizeof(sport)-1,"%s",utmp->getPort().c_str());
      unlock(); // this can require some extra time, so remove lock
      usleep(3);
      ::snmpSystem.drop(saddr,sport);
      usleep(3);
      lock();
    }
  }
  unlock();
}

int
Device_data::numsess(const string u) {
  int ret = 0;
  DeviceUser_data *utmp = 0;
  lock();
  for(list<DeviceUser_data*>::const_iterator i=cusr_.begin();i!=cusr_.end() && !cusr_.empty();i++) {
    utmp = *i;
    if(utmp->compareUsername(u) && utmp->isActive() &&
      (utl::scomp(utmp->getPort().c_str(),"As",2) ||
        utl::scomp(utmp->getPort().c_str(),"Se",2))) ret++;
  }
  unlock();
  return ret;
}

int
Device_data::size() {
  lock();
  int ret = cusr_.size();
  unlock();
  return ret;
}

int
Device_data::getDelay() {
  lock();
  int ret = delay_;
  unlock();
  return ret;
}

void
Device_data::setDelay(const int d) {
  lock();
  delay_ = d;
  unlock();
}

string
Device_data::getDescr() {
  lock();
  string ret = descr_;
  unlock();
  return ret;
}

void
Device_data::setDescr(const string d) {
  lock();
  descr_ = d;
  unlock();
}

string
Device_data::getCommunity() {
  lock();
  string ret = community_;
  unlock();
  return ret;
}

void
Device_data::setCommunity(const string d) {
  lock();
  community_ = d;
  unlock();
}

ipaddr
Device_data::getHostaddr() {
  lock();
  ipaddr ret = hostaddr_;
  unlock();
  return ret;
}

void
Device_data::setHostaddr(const ipaddr d) {
  lock();
  hostaddr_ = d;
  unlock();
}

bool
Device_data::compareHostaddr(const ipaddr d) {
  lock();
  bool ret = false;
  if(hostaddr_ == d) ret = true;
  unlock();
  return ret;
}

string
Device_data::getModule() {
  lock();
  string ret = module_;
  unlock();
  return ret;
}

void
Device_data::setModule(const string d) {
  lock();
  module_ = d;
  unlock();
}

bool
Device_data::getCounter() {
  lock();
  bool ret = counter_;
  unlock();
  return ret;
}

void
Device_data::setCounter(const bool d) {
  lock();
  counter_ = d;
  unlock();
}

bool
Device_data::getShutdown() {
  lock();
  bool ret = shutdown_;
  unlock();
  return ret;
}

void
Device_data::setShutdown(const bool d) {
  lock();
  shutdown_ = d;
  unlock();
}

string
Device_data::getTacKey() {
  lock();
  string ret = tac_key_;
  unlock();
  return ret;
}

void
Device_data::setTacKey(const string d) {
  lock();
  tac_key_ = d;
  unlock();
}

string
Device_data::getLoginstring() {
  lock();
  string ret = loginstring_;
  unlock();
  return ret;
}

void
Device_data::setLoginstring(const string d) {
  lock();
  loginstring_ = d;
  unlock();
}

string
Device_data::getPwdstring() {
  lock();
  string ret = pwdstring_;
  unlock();
  return ret;
}

void
Device_data::setPwdstring(const string d) {
  lock();
  pwdstring_ = d;
  unlock();
}

bool
Device_data::getDefAuthor() {
  lock();
  bool ret = def_author_;
  unlock();
  return ret;
}

void
Device_data::setDefAuthor(const bool d) {
  lock();
  def_author_ = d;
  unlock();
}

bool
Device_data::getSnmppolling() {
  lock();
  bool ret = snmppolling_;
  unlock();
  return ret;
}

void
Device_data::setSnmppolling(const bool d) {
  lock();
  snmppolling_ = d;
  unlock();
}

bool
Device_data::getIcmppolling() {
  lock();
  bool ret = icmppolling_;
  unlock();
  return ret;
}

void
Device_data::setIcmppolling(const bool d) {
  lock();
  icmppolling_ = d;
  unlock();
}

bool
Device_data::getStatus() {
  lock();
  bool ret = status_;
  unlock();
  return ret;
}

void
Device_data::setStatus(const bool d) {
  lock();
  status_ = d;
  unlock();
}

int
Device_data::getRest() {
  lock();
  int ret = rest_;
  unlock();
  return ret;
}

void
Device_data::setRest(const int d) {
  lock();
  rest_ = d;
  unlock();
}

// load/reload module for snmp processing
// return 0 if success, or error message string
string
Device_data::load() {
  lock();
  if(dl_) delete dl_;
  dl_ = new DLoad;
  string str = "";
//std::cerr<<"try to load module "<<module.get()<<std::endl;
  str = dl_->snmpload((char*)module_.c_str());
  if(str.size() > 0) {
    startlog(LOG_ERROR)<<"snmp module: "<<str<<endlog;
    // error in load module
    getnas = 0;
    dropnas = 0;
    unlock();
    return str;
  }
  getnas = (bool (*)(SnmpDevice*,SnmpD*(*)(char*,char*,char*),char*))
    dl_->getfunc("getnas");
  dropnas = (bool (*)(SnmpDevice*,char*,SnmpD*(*)(char*,char*,char*),
          bool (*)(char*,char*,char*,char*,char),char*))
              dl_->getfunc("dropnas");
  if(getnas == 0 || dropnas == 0) {
    str = str + "Unknown snmp module " + module_ + " format";
  }
  unlock();
  return str;
}

// build configuration for one device
string
Device_data::buildcfstr() {
  lock();
  string str;
  str = str + " device " + hostaddr_.getn() + "\n";
  str = str + "  description \'" + descr_ + "\'\n";
  str = str + "  tacacskey \'" + tac_key_ + "\'\n";
  str = str + "  snmpcommunity \'" + community_ + "\'\n";
  str = str + "  loginstring \'" + loginstring_ + "\'\n";
  str = str + "  pwdstring \'" + pwdstring_ + "\'\n";
  str = str + "  defauthorization " + (def_author_?"yes":"no") + "\n";
  str = str + "  module " + module_ + "\n";
  str = str + "  polldelay " + utl::toString(delay_) + "\n";
  str = str + "  inttrfcount " + (counter_?"yes":"no") + "\n";
  str = str + "  snmppolling " + (snmppolling_?"yes":"no") + "\n";
  str = str + "  icmppolling " + (icmppolling_?"yes":"no") + "\n";
  str = str + "  shutdown " + (shutdown_?"yes":"no") + "\n";
  str = str + "  exit";
  unlock();
  return str;
}


///////////// CoreDevice ///////////////

CoreDevice::CoreDevice(const string n,const string d) : Core_data(n,d,this) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  runPeriodic_ = false;
  unlock();
}

// destructor
CoreDevice::~CoreDevice() {
  Device_data *tmp = 0;
  lock();
  for(list<Device_data*>::iterator i=nas.begin();i!=nas.end() && !nas.empty();) {
    tmp = *i;
    i = nas.erase(i);
    delete tmp;
  }
  nas.clear();
  unlock();
  pthread_mutex_destroy(&Mutex_);
}

void
CoreDevice::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
CoreDevice::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

// get data structure, 0 if no (without locking)
Device_data*
CoreDevice::get_(const ipaddr ip) {
  Device_data *tmp = 0;
  for(list<Device_data*>::const_iterator i=nas.begin();i!=nas.end();i++) {
    tmp = *i;
    if(tmp->getHostaddr()==ip) {
      return tmp;
    }
  }
  return 0;
}

// get data structure, 0 if no
Device_data*
CoreDevice::get(const ipaddr ip) {
std::cerr<<"Before strange lock()"<<endl;
  lock();
std::cerr<<"After strange lock()"<<endl;
  Device_data *tmp = 0;
  tmp = get_(ip);
  unlock();
  return tmp;
}

// load/reload module, 0-success
string
CoreDevice::load(const ipaddr ip) {
  string str = "";
  Device_data *tmp = 0;
  lock();
  if((tmp=get_(ip)) != 0) {
    str = tmp->load();
    unlock();
    return str;
  }
  unlock();
  return "";
}

// return modulename
string
CoreDevice::getmodulename(const ipaddr ip) {
  string str;
  Device_data *tmp = 0;
  lock();
  if((tmp = get_(ip)) != 0) {
    str = tmp->getModule();
    unlock();
    return str;
  }
  unlock();
  return "none.so";
}

// add NAS to list
void
CoreDevice::add(const ipaddr ip) {
  lock();
  if(get_(ip) != 0) {
    unlock();
    return;
  }
  Device_data *x = new Device_data(ip);
  nas.push_back(x);
  unlock();
}

// delete NAS from list
void
CoreDevice::del(const ipaddr ip) {
  Device_data *tmp = 0;
  lock();
  for(list<Device_data*>::iterator i=nas.begin();i!=nas.end();) {
    tmp = *i;
    if(tmp->getHostaddr() == ip) {
      i = nas.erase(i);
      delete tmp;
      continue;
    }
    i++;
  }
  unlock();
}

void
CoreDevice::usr_expire() {
  Device_data *tmp = 0;
  lock();
  for(list<Device_data*>::iterator i=nas.begin();i!=nas.end();i++) {
    tmp = *i;
    tmp->usr_expire();
  }
  unlock();
}

bool
CoreDevice::usr_del(const ipaddr n,const string p) {
  Device_data *tmp = 0;
  bool ret = false;
  lock();
  for(list<Device_data*>::const_iterator i=nas.begin();i!=nas.end();i++) {
    tmp = *i;
    if(tmp->compareHostaddr(n)) {
      if(tmp->usr_del(p)) {
        ret = true;
        break;
      }
    }
  }
  unlock();
  return ret;
}

void
CoreDevice::usr_del(const ipaddr n) {
  Device_data *tmp = 0;
  lock();
  for(list<Device_data*>::const_iterator i=nas.begin();i!=nas.end();i++) {
    tmp = *i;
    if(tmp->compareHostaddr(n)) {
      tmp->usr_del();
      break;
    }
  }
  unlock();
}

bool
CoreDevice::usr_activate(const ipaddr n,const string u,const string p,const string t,const ipaddr a) {
  bool ret = false;
  Device_data *tmp = 0;
  lock();
  for(list<Device_data*>::const_iterator i=nas.begin();i!=nas.end();i++) {
    tmp = *i;
    if(tmp->compareHostaddr(n)) {
      if(tmp->usr_activate(u,p,t,a)) {
        ret = true;
        break;
      }
    }
  }
  unlock();
  return ret;
}

bool
CoreDevice::usr_add(const ipaddr n,const string u,const string p,const string t,const ipaddr a,const db_user d) {
  bool ret = false;
  Device_data *tmp = 0;
  lock();
  for(list<Device_data*>::const_iterator i=nas.begin();i!=nas.end();i++) {
    tmp = *i;
    if(tmp->compareHostaddr(n)) {
      if(tmp->usr_add(n,u,p,t,a,d)) {
        ret = true;
      }
      break;
    }
  }
  unlock();
  return ret;
}

void
CoreDevice::usr_periodic(const time_t t) {
  if(!::serverTrack.isActive()) return; // we do not do
  Device_data *tmp = 0;
  lock();
  for(list<Device_data*>::const_iterator i=nas.begin();i!=nas.end();i++) {
    tmp = *i;
    tmp->usr_periodic(t);
  }
  unlock();
}

int
CoreDevice::numsess(const string u) {
  int ret = 0;
  Device_data *tmp = 0;
  lock();
  for(list<Device_data*>::const_iterator i=nas.begin();i!=nas.end();i++) {
    tmp = *i;
    ret += tmp->numsess(u);
  }
  unlock();
  return ret;
}

void
CoreDevice::modifyCommunity(const ipaddr ip,const string c) {
  Device_data *tmp = 0;
  lock();
  if((tmp = get_(ip)) != 0) {
    tmp->setCommunity(c);
  }
  unlock();
}

void
CoreDevice::modifyCounter(const ipaddr ip,const bool co) {
  Device_data *tmp = 0;
  lock();
  if((tmp = get_(ip)) != 0) {
    tmp->setCounter(co);
  }
  unlock();
}

void
CoreDevice::modifyDelay(ipaddr ip,int pd) {
  Device_data *tmp = 0;
  lock();
  if((tmp = get_(ip)) != 0) {
    tmp->setDelay(pd);
  }
  unlock();
}

// set/modify description
void
CoreDevice::modifyDescr(ipaddr ip,const string d) {
  Device_data *tmp = 0;
  lock();
  if((tmp = get_(ip)) != 0) {
    tmp->setDescr(d);
  }
  unlock();
}

// set/modify snmp processor module
void
CoreDevice::modifyModule(ipaddr ip,const string m) {
  Device_data *tmp = 0;
  lock();
  if((tmp = get_(ip)) != 0) {
    tmp->setModule(m);
  }
  unlock();
}

void
CoreDevice::modifyKey(ipaddr ip,const string tk) {
  Device_data *tmp = 0;
  lock();
  if((tmp = get_(ip)) != 0) {
    tmp->setTacKey(tk);
  }
  unlock();
}

void
CoreDevice::modifyLogin(ipaddr ip,const string l) {
  Device_data *tmp = 0;
  lock();
  if((tmp = get_(ip)) != 0) {
    tmp->setLoginstring(l);
  }
  unlock();
}

void
CoreDevice::modifyPwd(ipaddr ip,const string p) {
  Device_data *tmp = 0;
  lock();
  if((tmp = get_(ip)) != 0) {
    tmp->setPwdstring(p);
  }
  unlock();
}

void
CoreDevice::modifyDef(ipaddr ip,bool a) {
  Device_data *tmp = 0;
  lock();
  if((tmp = get_(ip)) != 0) {
    tmp->setDefAuthor(a);
  }
  unlock();
}

void
CoreDevice::modifyShut(ipaddr ip,bool s) {
  Device_data *tmp = 0;
  lock();
  if((tmp = get_(ip)) != 0) {
    tmp->setShutdown(s);
  }
  unlock();
}

void
CoreDevice::modifySnmp(ipaddr ip,bool s) {
  Device_data *tmp = 0;
  lock();
  if((tmp = get_(ip)) != 0) {
    tmp->setSnmppolling(s);
  }
  unlock();
}

void
CoreDevice::modifyIcmp(ipaddr ip,bool s) {
  Device_data *tmp = 0;
  lock();
  if((tmp = get_(ip)) != 0) {
    tmp->setIcmppolling(s);
  }
  unlock();
}

Device_data*
CoreDevice::get(int n) {
  Device_data *tmp = 0;
  lock();
  if(nas.size() > 0 && (int)nas.size() > n) {
    int count = 0;
    list<Device_data*>::const_iterator i;
    for(i=nas.begin();i!=nas.end();i++,count++) {
      if(count == n) {
        tmp = *i;
        break;
      }
    }
  }
  unlock();
  return tmp;
}

ipaddr
CoreDevice::getip(int n) {
  Device_data *tmp = 0;
  ipaddr ret = UNKNOWN_IP;
  lock();
  if(nas.size() > 0 && (int)nas.size() > n) {
    int count = 0;
    list<Device_data*>::const_iterator i;
    for(i=nas.begin();i!=nas.end();i++,count++) {
      if(count == n) {
        tmp = *i;
        ret = tmp->getHostaddr();
        unlock();
        return ret;
      }
    }
  }
  unlock();
  return ret;
}

DeviceUser_data
CoreDevice::getuser(int n) {
  Device_data *tmp = 0;
  DeviceUser_data utmp;
  DeviceUser_data res;
  lock();
  if(nas.size() > 0) {
    int count = 0;
    list<Device_data*>::const_iterator i;
    for(i=nas.begin();i!=nas.end();i++) {
      tmp = *i;
      int j = 0;
      while(true) {
        utmp = tmp->gets(j);
        if(!utmp.isPresent()) break;
        if(count == n) {
          unlock();
          return utmp;
        }
        count++;
        j++;
      }
    }
  }
  unlock();
  return res;
}

DeviceUser_data
CoreDevice::getuser(ipaddr ip,int n) {
  Device_data *tmp = 0;
  DeviceUser_data utmp;
  DeviceUser_data res;
  lock();
  if(nas.size() > 0) {
    list<Device_data*>::const_iterator i;
    for(i=nas.begin();i!=nas.end();i++) {
      tmp = *i;
      if(tmp->compareHostaddr(ip)) {
        utmp = tmp->gets(n);
        unlock();
        return utmp;
      }
    }
  }
  unlock();
  return res;
}

// get username from known user ipv4 address
DeviceUser_data
CoreDevice::getuser4ip(ipaddr ip) {
  Device_data *tmp = 0;
  DeviceUser_data utmp;
  DeviceUser_data res;
  lock();
  if(nas.size() > 0) {
    list<Device_data*>::const_iterator i;
    for(i=nas.begin();i!=nas.end();i++) {
      tmp = *i;
      utmp = tmp->gets(ip);
      if(utmp.isPresent()) {
        unlock();
        return utmp;
      }
    }
  }
  unlock();
  return res;
}

int
CoreDevice::size() {
  lock();
  int s = nas.size();
  unlock();
  return s;
}

// get configuration string for specific device
string
CoreDevice::buildcfstr(int n) {
  string str = "";
  if((int)nas.size() < n) return str;
  Device_data *tmp = get(n);
  if(tmp) str = tmp->buildcfstr();
  return str;
}

// apply configuration
string
CoreDevice::applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd)throw (CmdErr) {
  string str = "";
  string tmp;
  if(cmdname=="confDeviceNo") {
    del(ipaddr(paramList.begin()->c_str()));
  } else if(cmdname=="confDeviceNode") {
    add(ipaddr(paramList.begin()->c_str()));
  } else if(cmdname=="confDeviceDescr" ) {
    modifyDescr(ipaddr(cmd->upNode->paramList.begin()->c_str()), paramList.begin()->c_str());
  } else if(cmdname=="confDeviceTKey") {
//cerr<<"tacacs+ key="<<paramList.begin()->c_str()<<" on device="<<ipaddr(cmd->upNode->paramList.begin()->c_str()).get()<<endl;
    modifyKey(ipaddr(cmd->upNode->paramList.begin()->c_str()), paramList.begin()->c_str());
  } else if(cmdname=="confDeviceSnmpComm" ) {
//    std::cerr<<std::endl<<"!!!!commun="<<paramList.begin()->c_str()<<std::endl;
    modifyCommunity(ipaddr(cmd->upNode->paramList.begin()->c_str()), paramList.begin()->c_str());
  } else if( cmdname=="confDeviceLogin" ) {
    modifyLogin(ipaddr(cmd->upNode->paramList.begin()->c_str()), paramList.begin()->c_str());
  } else if( cmdname=="confDevicePwd") {
    modifyPwd(ipaddr(cmd->upNode->paramList.begin()->c_str()), paramList.begin()->c_str());
  } else if(cmdname=="confDeviceDauth") {
    modifyDef(ipaddr(cmd->upNode->paramList.begin()->c_str()), utl::vmatch(paramList.begin()->c_str(),"Yes").size() ? true : false);
  } else if(cmdname=="confDeviceModule") {
//cerr<<"CoreDevice load module: "<<getmodulename(ipaddr(cmd->upNode->paramList.begin()->c_str()))<<
//" "<<paramList.begin()->c_str()<<" "<<cmd->upNode->paramList.begin()->c_str()<<endl;
    tmp = getmodulename(ipaddr(cmd->upNode->paramList.begin()->c_str()));
    modifyModule(ipaddr(cmd->upNode->paramList.begin()->c_str()), paramList.begin()->c_str());
    str = load(cmd->upNode->paramList.begin()->c_str());
    if(str.size() > 0) {
      modifyModule(ipaddr(cmd->upNode->paramList.begin()->c_str()),tmp.c_str());
      load(ipaddr(cmd->upNode->paramList.begin()->c_str()));
    }
 //   return str;
  } else if(cmdname=="confDevicePollDelay") {
    modifyDelay(ipaddr(cmd->upNode->paramList.begin()->c_str()), atoi(paramList.begin()->c_str()));
  } else if(cmdname=="confDeviceIntrfCount") {
    modifyCounter(ipaddr(cmd->upNode->paramList.begin()->c_str()), utl::vmatch(paramList.begin()->c_str(),"Yes").size() ? true : false);
  } else if(cmdname=="confDeviceSnmpPolling") {
    modifySnmp(ipaddr(cmd->upNode->paramList.begin()->c_str()), utl::vmatch(paramList.begin()->c_str(),"Yes").size() ? true : false);
  } else if(cmdname=="confDeviceIcmpPolling") {
    modifyIcmp(ipaddr(cmd->upNode->paramList.begin()->c_str()), utl::vmatch(paramList.begin()->c_str(),"Yes").size() ? true : false);
  } else if(cmdname=="confDeviceShut") {
    modifyShut(ipaddr(cmd->upNode->paramList.begin()->c_str()), utl::vmatch(paramList.begin()->c_str(),"Yes").size() ? true : false);
  }
  return str;
}

//
void
CoreDevice::setDeviceStatus(ipaddr ip,bool s) {
  Device_data *tmp = 0;
  lock();
  for(list<Device_data*>::const_iterator i=nas.begin();i!=nas.end();i++) {
    tmp = *i;
    if(tmp->getHostaddr() == ip) {
      tmp->setStatus(s);
      break;
    }
  }
  unlock();
}

//
bool
CoreDevice::get_device_status(ipaddr ip) {
  bool ret = false;
  Device_data *tmp = 0;
  lock();
  for(list<Device_data*>::const_iterator i=nas.begin();i!=nas.end();i++) {
    tmp = *i;
    if(tmp->getHostaddr() == ip) {
      ret = tmp->getStatus();
      break;
    }
  }
  unlock();
  return ret;
}

extern void periodicPolling();

// start periodic device polling process in separate thread
void
CoreDevice::startPeriodic() {
  if(::serverTrack.isActive()) { // if this bundle server active
    if(!getRunPeriodic()) { // if no polling thread active
      pthread_create(&thrvar_,NULL,(void*(*)(void*))&periodicPolling,(void*)NULL);
      pthread_detach(thrvar_);
    }
  }
}

string
CoreDevice::getCommunity(ipaddr ip) {
  Device_data *tmp = 0;
  string out = "";
  lock();
  if(nas.size() > 0) {
    list<Device_data*>::const_iterator i;
    for(i=nas.begin();i!=nas.end();i++) {
      tmp = *i;
      if(tmp->compareHostaddr(ip)) {
        out = tmp->getCommunity();
        unlock();
        return out;
      }
    }
  }
  unlock();
  return out;
}

void
CoreDevice::setRunPeriodic(bool d) {
  lock();
  runPeriodic_ = d;
  unlock();
}

bool
CoreDevice::getRunPeriodic() {
  lock();
  bool ret = runPeriodic_;
  unlock();
  return ret;
}


///////////// periodicPolling ////////////////

void
periodicPolling() {
  usleep(5); // some delay before polling
  ::coreData->getDevice()->setRunPeriodic(true);
  Device_data *tmp = 0;
  SnmpDevice *sd = 0;
  DeviceUser_data dutmp;
  SnmpOut *stmp = 0;
  bool result = false;

  TipSystem tipSystem;
  tipSystem.clear_ut();
  tipSystem.clear_dt();

  // runs over all devices
  for(int i=0; (tmp = ::coreData->getDevice()->get(i)) != 0; i++) {
    if(tmp->getShutdown()) continue; // if shutdowned - switch to other device
    ipaddr nasip = tmp->getHostaddr();

    // work only if no time present in rest and polldelay > 0
    if(tmp->getDelay() == 0) tmp->setDelay(60);
    if(tmp->getRest() > 0) {
      int x = tmp->getRest();
      x--;
      tmp->setRest(x);
      continue;
    }
    tmp->setRest(tmp->getDelay());

    result = false;
    sd = 0;
    if(tmp->getSnmppolling()) { // if snmp polling enabled
      sd = ::snmpSystem.get(tmp); // request NAS
      if(sd) result = true;
      else result = false;
      startlog(LOG_SNMP)<<"SNMP: polling "<<nasip<<" result="<<(result?"true":"false")<<endlog;
    }
    if(tmp->getIcmppolling() && result==false) { // if icmp polling enabled
      Icmp *icmp_test = new Icmp;
      result = icmp_test->test(nasip);
      delete icmp_test;
      startlog(LOG_SNMP)<<"ICMP: polling "<<nasip<<" result="<<(result?"true":"false")<<endlog;
    }

//std::cerr<<"result after snmp/icmp "<<nasip.get()<<" polling ="<<result<<std::endl;

    if(!result) { // error
      if(tmp->getStatus()) {
        startlog(LOG_EVENT)<<"NAS "<<nasip<<" ("<<tmp->getDescr()<<") go Down"<<endlog;
        tipSystem.clear_dt();
        tipSystem.add_dt(nasip,false); // add field in dt packet
        if(::coreData->getBundle()->isBundle()) tipSystem.send_dt();
        tipSystem.clear_dt();
        // set status
        tmp->setStatus(false);
        // this NAS have low trusty, and all users on this nas too
        ::coreData->getDevice()->usr_del(nasip);
        // all users from this device will be deleted by remote tacppds too
      }
    } else {
      if(!tmp->getStatus()) {
        startlog(LOG_EVENT)<<"NAS "<<nasip<<" ("<<tmp->getDescr()<<") go Up"<<endlog;
        tipSystem.clear_dt();
        tipSystem.add_dt(nasip,true);  // add field in dt packet
        if(::coreData->getBundle()->isBundle()) tipSystem.send_dt();
        tipSystem.clear_dt();
        tmp->setStatus(true);
      }
      // if snmp polling enabled
      if(tmp->getSnmppolling()) {
        ::snmpSystem.store_traffic(tmp,sd); // store traffic information
        // run over return and add to queue
        tipSystem.clear_ut(); // clear usertrack packet
        tipSystem.clear_dt();
        // in first, check about new users which detected by
        // snmp but not present into UserTrack data
        string ph;
        ipaddr xip = UNKNOWN_IP; // unknown ip addr

        stmp = 0;
        for(int i=0;(stmp = sd->get(i)) != 0 ;i++) {
          string us = stmp->getUser();
          if(us.size() > 0) {
            string pr = utl::portConvert(stmp->getPort());
            // run around this device in UserTrack
//cerr<<"get from snmp - u="<<us<<" pr="<<pr<<endl;
            if(::userTrack.add(us,nasip,pr,ph,xip)) { // add/trust accepted users
              // prepare usertrack packet
              tipSystem.add_ut(us,nasip,xip,pr,ph,true);
            }
          }
        }
        // in second, check about users which is present
        // in UserTrack, but not present into snmp
        bool present;
        int iii = 0;
        while(true) {
          dutmp = ::coreData->getDevice()->getuser(nasip,iii);
          if(!dutmp.isPresent()) break;
          string dus = dutmp.getUsername();
          string dpr = dutmp.getPort();
          ipaddr dnas = dutmp.getNasAddr();
          ipaddr dua = dutmp.getUserAddr();
          string dph = dutmp.getPhone();
          if(nasip == dnas) {
            present = false;
            for(int j=0;(stmp = sd->get(j)) != 0;j++) {
              string us = stmp->getUser();
              if(us.size() > 0) {
                string pr = utl::portConvert(stmp->getPort());
                if(dus == us && dpr == pr) {
                  present = true;
                  break;
                }
              }
            }
            if(!present) {
              ::userTrack.del(dus,nasip,dpr); // del accepted users
              // prepare usertrack packet
              tipSystem.add_ut(dus,nasip,dua,dpr,dph,false);
              iii = 0;
            }
          }
          iii++;
//        usleep(1);
        }
        if(::coreData->getBundle()->isBundle()) tipSystem.send_ut(); // send usertrack packet to bundle
        tipSystem.clear_ut(); // clear usertrack packet
        tipSystem.clear_dt(); // for something case :)
      }
    }
    if(sd) delete sd; // free
    sd = 0;
  }
  ::coreData->getDevice()->setRunPeriodic(false);
  pthread_exit(0);
}

/////////////////////// that is all //////////////////
