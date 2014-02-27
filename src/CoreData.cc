// tacppd Core* classes common things
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

#include "global.h"

//////////////////// Core_data //////////////////////

// zero all
void
Core_data::zero_all() {
  coreAccess = 0;
  coreDevice = 0;
  coreDebug = 0;
  coreManager = 0;
  corePool = 0;
  coreDb = 0;
  coreListener = 0;
  coreBundle = 0;
  coreBilling = 0;
  corePeer = 0;
}

void
Core_data::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
Core_data::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

Core_data::Core_data(const string n,const string d,CoreAccess *a) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  name = n;
  descr = d;
  zero_all();
  coreAccess = a;
  unlock();
}

Core_data::Core_data(const string n,const string d,CoreDevice *a) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  name = n;
  descr = d;
  zero_all();
  coreDevice = a;
  unlock();
}

Core_data::Core_data(const string n,const string d,CoreDebug *a) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  name = n;
  descr = d;
  zero_all();
  coreDebug = a;
  unlock();
}

Core_data::Core_data(const string n,const string d,CoreManager *a) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  name = n;
  descr = d;
  zero_all();
  coreManager = a;
  unlock();
}

Core_data::Core_data(const string n,const string d,CorePool *a) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  name = n;
  descr = d;
  zero_all();
  corePool = a;
  unlock();
}

Core_data::Core_data(const string n,const string d,CoreDb *a) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  name = n;
  descr = d;
  zero_all();
  coreDb = a;
  unlock();
}

Core_data::Core_data(const string n,const string d,CoreListener *a) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  name = n;
  descr = d;
  zero_all();
  coreListener = a;
  unlock();
}

Core_data::Core_data(const string n,const string d,CoreBundle *a) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  name = n;
  descr = d;
  zero_all();
  coreBundle = a;
  unlock();
}

Core_data::Core_data(const string n,const string d,CoreBilling *a) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  name = n;
  descr = d;
  zero_all();
  coreBilling = a;
  unlock();
}

Core_data::Core_data(const string n,const string d,CorePeer *a) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  name = n;
  descr = d;
  zero_all();
  corePeer = a;
  unlock();
}

// destructor
Core_data::~Core_data() {
  lock();
  if(coreAccess) delete coreAccess;
  if(coreDevice) delete coreDevice;
  if(coreDebug) delete coreDebug;
  if(coreManager) delete coreManager;
  if(corePool) delete corePool;
  if(coreDb) delete coreDb;
  if(coreListener) delete coreListener;
  if(coreBundle) delete coreBundle;
  if(coreBilling) delete coreBilling;
  if(corePeer) delete corePeer;
  unlock();
  pthread_mutex_destroy(&Mutex_);
}

// build cf for one object
string
Core_data::buildcf(int n,string d) {
  lock();
  string str;
  if(n == 0) str = str + "\n ! " + d + "\n";
  str = str + buildcfstr(n) + "\n";
  unlock();
  return str;
}

// build cf for this object, occupy memory by 'new'
string
Core_data::buildcf() {
  string out;
  string stmp;
  stmp = stmp + buildcf(0,descr);
  for(int i=1; stmp.size()>5; i++) {
    out = out + stmp;
    stmp.erase();
    stmp = buildcf(i,descr);
  }
  return out;
}

/////////////////////// CoreData /////////////////////////

CoreData::CoreData() {
  pthread_mutex_init(&Mutex_,0);
  // fill queue with core objects (this order will be used in config file)
  lock();
  core_.push_back(new CoreDebug(DEBUG_TYPE,"debug"));
  core_.push_back(new CoreAccess(ACCESS_TYPE,"access list"));
  core_.push_back(new CoreManager(MANAGER_TYPE,"system manager"));
  core_.push_back(new CoreListener(LISTENER_TYPE,"tcp/udp listener"));
  core_.push_back(new CorePool(POOL_TYPE,"address pool"));
  core_.push_back(new CoreDevice(DEVICE_TYPE,"network access server"));
  core_.push_back(new CoreDb(DB_TYPE,"database"));
  core_.push_back(new CoreBundle(BUNDLE_TYPE,"tacppd bundle"));
  core_.push_back(new CoreBilling(BILLING_TYPE,"billing module"));
  core_.push_back(new CorePeer(PEER_TYPE,"roaming peer"));
  unlock();
}

void
CoreData::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
CoreData::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

// destructor
CoreData::~CoreData() {
  Core_data *tmp = 0;
  lock();
  for(list<Core_data*>::iterator i=core_.begin(); i!=core_.end() && !core_.empty();) {
    tmp = *i;
    i = core_.erase(i);
    delete tmp;
  }
  core_.clear();
  unlock();
}

//
string
CoreData::getn(int num) {
  lock();
  Core_data *tmp = 0;
  string ret;
  if((int)core_.size() > num) {
    int count = 0;
    for(list<Core_data*>::const_iterator i=core_.begin(); i!=core_.end(); i++,count++) {
      if(count == num) {
        tmp = *i;
        ret = tmp->name;
        unlock();
        return ret;
      }
    }
  }
  unlock();
  return ret;
}

// build configuration for all objects, occupy memory by 'new'
string
CoreData::showcf(char *user) {
  lock();
  Core_data *tmp;
  string out;
  time_t t = time(0);
  char ct[40];
  ctime_r(&t,ct); // current datetime
  ct[strlen(ct)-1] = '\0';
  out = out + "! tacppd configuration file, build by " + user + " " + ct + " \nconfig\n";
  // go across all core objects
  for(list<Core_data*>::const_iterator i=core_.begin(); i!=core_.end() ;i++) {
    tmp = *i;
    out = out + tmp->buildcf();
  }
  out = out + "\nexit\n! end of config\n";
  unlock();
  return out;
}

char*
CoreData::writecf(char *file,char *user) {
  // rename current
  string fn1;      // filename
  string fn2;      // filename.old
  string fn3;      // filename
  time_t t = time(0);
  struct tm res;
  struct tm *tt = localtime_r(&t,&res);

  fn1 = file;
  fn2 = fn2 + file + ".old";
  char strd[40];
  snprintf(strd,sizeof(strd)-1,"%04d%02d%02d%02d%02d",1900+tt->tm_year,
    tt->tm_mon+1,tt->tm_mday,tt->tm_hour,tt->tm_min);
  fn3 = fn3 + file + "." + strd;
  rename(fn1.c_str(),fn2.c_str());
  // store
  std::ofstream cfstrw;
  string s = showcf(user);
  cfstrw.open(file,std::ios::out);    // file open	
  cfstrw<<s;                          // write config
  cfstrw.close();                     // close config file
  cfstrw.open(fn3.c_str(),std::ios::out);
  cfstrw<<s;                          // write config
  cfstrw.close();                     // close config file
  //delete [] s;
  return "";
}

// number of core objects
int
CoreData::size() {
  lock();
  int ret = core_.size();
  unlock();
  return ret;
}

// get object
Core_data*
CoreData::get(int n) {
  lock();
  Core_data *tmp = 0;
  if(core_.size() > 0 && (int)core_.size() > n) {
    int count = 0;
    list<Core_data*>::const_iterator i;
    for(i=core_.begin(); i!=core_.end(); i++,count++) {
      if(count==n) {
        tmp = *i;
        break;
      }
    }
  }
  unlock();
  return tmp;
}

// get object
Core_data*
CoreData::get(const string n) {
  lock();
  Core_data *tmp = 0;
  for(list<Core_data*>::const_iterator i=core_.begin(); i!=core_.end(); i++) {
    tmp = *i;
    if(utl::match((char*)n.c_str(),(char*)tmp->name.c_str())) {
      unlock();
      return tmp;
    }
  }
  unlock();
  return 0;
}

Core_data*
CoreData::get(const char *n) {
  const string aaa = n;
  return get(aaa);
}

// get help about all objects
string
CoreData::help() {
  string str;
  Core_data *tmp;
  lock();
  for(list<Core_data*>::const_iterator i=core_.begin(); i!=core_.end(); i++) {
    tmp = *i;
    str = str + " " + utl::lower(tmp->name);
    str = str + " ";
    for(int j=tmp->name.size(); j<20; j++) str = str + " ";
    str = str + tmp->descr + "\n";
  }
  unlock();
  return str;
}

// get several types of objects
CoreAccess*
CoreData::getAccess() {
  Core_data *tmp;
  CoreAccess *ca;
  lock();
  for(list<Core_data*>::const_iterator i=core_.begin(); i!=core_.end(); i++) {
    tmp = *i;
    if(tmp->name == ACCESS_TYPE) {
      ca = tmp->coreAccess;
      unlock();
      return ca;
    }
  }
  unlock();
  return 0;
}

CoreDevice*
CoreData::getDevice() {
  Core_data *tmp;
  CoreDevice *cd;
  lock();
  for(list<Core_data*>::const_iterator i=core_.begin(); i!=core_.end(); i++) {
    tmp = *i;
    if(tmp->name == DEVICE_TYPE) {
      cd = tmp->coreDevice;
      unlock();
      return cd;
    }
  }
  unlock();
  return 0;
}

CoreDebug*
CoreData::getDebug() {
  Core_data *tmp;
  CoreDebug *cd;
  lock();
  for(list<Core_data*>::const_iterator i=core_.begin(); i!=core_.end(); i++) {
    tmp = *i;
    if(tmp->name == DEBUG_TYPE) {
      cd = tmp->coreDebug;
      unlock();
      return cd;
    }
  }
  unlock();
  return 0;
}

CoreManager*
CoreData::getManager() {
  Core_data *tmp;
  CoreManager *cm;
  lock();
  for(list<Core_data*>::const_iterator i=core_.begin(); i!=core_.end(); i++) {
    tmp = *i;
    if(tmp->name == MANAGER_TYPE) {
      cm = tmp->coreManager;
      unlock();
      return cm;
    }
  }
  unlock();
  return 0;
}

CorePool*
CoreData::getPool() {
  Core_data *tmp;
  CorePool *cp;
  lock();
  for(list<Core_data*>::const_iterator i=core_.begin(); i!=core_.end(); i++) {
    tmp = *i;
    if(tmp->name == POOL_TYPE) {
      cp = tmp->corePool;
      unlock();
      return cp;
    }
  }
  unlock();
  return 0;
}

// get native CoreDb object
CoreDb*
CoreData::getDb() {
  Core_data *tmp;
  CoreDb *cd;
  lock();
  for(list<Core_data*>::const_iterator i=core_.begin(); i!=core_.end(); i++) {
    tmp = *i;
    if(tmp->name == DB_TYPE) {
      cd = tmp->coreDb;
      unlock();
      return cd;
    }
  }
  unlock();
  return 0;
}

// get native CoreListener object
CoreListener*
CoreData::getListener() {
  Core_data *tmp;
  CoreListener *cl;
  lock();
  for(list<Core_data*>::const_iterator i=core_.begin(); i!=core_.end(); i++) {
    tmp = *i;
    if(tmp->name == LISTENER_TYPE) {
      cl = tmp->coreListener;
      unlock();
      return cl;
    }
  }
  unlock();
  return 0;
}

// get native CoreBundle object
CoreBundle*
CoreData::getBundle() {
  Core_data *tmp;
  CoreBundle *cb;
  lock();
  for(list<Core_data*>::const_iterator i=core_.begin(); i!=core_.end(); i++) {
    tmp = *i;
    if(tmp->name == BUNDLE_TYPE) {
      cb = tmp->coreBundle;
      unlock();
      return cb;
    }
  }
  unlock();
  return 0;
}

// get native CoreBilling object
CoreBilling*
CoreData::getBilling() {
  Core_data *tmp;
  CoreBilling *cb;
  lock();
  for(list<Core_data*>::const_iterator i=core_.begin(); i!=core_.end(); i++) {
    tmp = *i;
    if(tmp->name == BILLING_TYPE) {
      cb = tmp->coreBilling;
      unlock();
      return cb;
    }
  }
  unlock();
  return 0;
}

// get native CorePeer object
CorePeer*
CoreData::getPeer() {
  Core_data *tmp;
  CorePeer *cp;
  lock();
  for(list<Core_data*>::const_iterator i=core_.begin(); i!=core_.end(); i++) {
    tmp = *i;
    if(tmp->name == PEER_TYPE) {
      cp = tmp->corePeer;
      unlock();
      return cp;
    }
  }
  unlock();
  return 0;
}

///////////////////// that is all ///////////////////
