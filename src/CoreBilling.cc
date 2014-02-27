// tacppd billing module methods
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

#include "global.h"


////// user_data

void
user_data::lock() {
  pthread_mutex_lock(&mutex_);
}

void
user_data::unlock() {
  pthread_mutex_unlock(&mutex_);
}

user_data::user_data(db_user d) {
  pthread_mutex_init(&mutex_,0);
  lock();
  ddata_ = d;
  numptr_ = 0;
  unlock();
}

user_data::~user_data() {
  pthread_mutex_destroy(&mutex_);
}

db_user
user_data::get() {
  lock();
  db_user ret = ddata_;
  unlock();
  return ret;
}

int
user_data::getNumPtr() {
  lock();
  int ret = numptr_;
  unlock();
  return ret;
}

void
user_data::incNumPtr() {
  lock();
  numptr_++;
  unlock();
}

void
user_data::decNumPtr() {
  lock();
  if(numptr_ > 0) numptr_--;
  unlock();
}


/////////////// Billing_data ////////////////////

// constr
Billing_data::Billing_data(int num) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  dl_ = 0;
  number_ = num;
  descr_ = "none";
  module_ = "none.so";
  shutdown_ = false;
  unlock();
}

// destructor
Billing_data::~Billing_data() {
  lock();
  if(dl_) delete dl_; // delete loaded module
  unlock();
  pthread_mutex_destroy(&Mutex_);
}

void
Billing_data::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
Billing_data::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

int
Billing_data::getNumber() {
  lock();
  int ret = number_;
  unlock();
  return ret;
}

void
Billing_data::setDescr(string d) {
  lock();
  descr_ = d;
  unlock();
}

string
Billing_data::getDescr() {
  lock();
  string ret = descr_;
  unlock();
  return ret;
}

void
Billing_data::setModule(string d) {
  lock();
  module_ = d;
  unlock();
}

string
Billing_data::getModule() {
  lock();
  string ret = module_;
  unlock();
  return ret;
}

void
Billing_data::setShutdown(bool d) {
  lock();
  shutdown_ = d;
  unlock();
}

bool
Billing_data::getShutdown() {
  lock();
  bool ret = shutdown_;
  unlock();
  return ret;
}

// load module
string
Billing_data::load() {
  lock();
  if(dl_) delete dl_;
  dl_ = new DLoad;
  string str;
//std::cout<<"try to load module "<<module.get()<<std::endl;
  str = str + dl_->bilload((char*)module_.c_str());
  if(str.size() > 0) {
    startlog(LOG_ERROR)<<"billing module: "<<str<<endlog;
    // error in load module
//getnas = 0;
//dropnas = 0;
    unlock();
    return str;
  }
//getnas = (bool (*)(SnmpDevice*,snmp_d*(*)(char*,char*,char*),char*))
//  dl->getfunc("getnas");
//dropnas = (bool (*)(SnmpDevice*,char*,snmp_d*(*)(char*,char*,char*),
//        bool (*)(char*,char*,char*,char*,char),char*))
//  dl->getfunc("dropnas");
//if(getnas == 0 || dropnas == 0) {
//  str<<"Unknown snmp module "<<module.get()<<" format"<<end();
//  return str.get();
//}
  unlock();
  return "";
}

string
Billing_data::buildcfstr() {
  string str;
  lock();
  str = str + " billing " + utl::toString(number_) + "\n";
  str = str + "  description \'" + descr_ + "\'\n";
  str = str + "  module " + module_ + "\n";
  //str = str + "  shutdown " + (shutdown?"yes":"no") + "\n";
  str = str + "  exit";
  unlock();
  return str;
}

/////////////// CoreBilling /////////////////////

CoreBilling::CoreBilling(const string n,const string d) : Core_data(n,d,this) {
  pthread_mutex_init(&Mutex,0);
}

// destr
CoreBilling::~CoreBilling() {
  lock();
  Billing_data *tmp;
  for(list<Billing_data*>::iterator i=bd.begin();i!=bd.end() && !bd.empty();) {
    tmp = *i;
    i = bd.erase(i);
    delete tmp;
  }
  bd.clear();
  user_data *tmp2;
  for(list<user_data*>::iterator i=ud_.begin();i!=ud_.end() && !ud_.empty();) {
    tmp2 = *i;
    i = ud_.erase(i);
    delete tmp2;
  }
  ud_.clear();
  unlock();
  pthread_mutex_destroy(&Mutex);
}

void
CoreBilling::lock() {
  pthread_mutex_lock(&Mutex);
}

void
CoreBilling::unlock() {
  pthread_mutex_unlock(&Mutex);
}

// load/reload module, 0-success
//char*
//CoreBilling::load(int num) {
//mystring<100> str;
//lock();
//for(int i=0; bd.size() > i; i++) {
//  if(bd.get(i)->number == num) {
////std::cout<<"try to load module "<<nas.get(i)->module.get()<<" for "<<nas.get(i)->hostaddr.get()<<std::endl;
//    str<<bd.get(i)->load()<<end();
//    unlock();
//    return str.c_str();
//  }
//}
//unlock();
//return "";
//}

// add void default module
void
CoreBilling::add(int num) {
  Billing_data *tmp;
  lock();
  for(list<Billing_data*>::const_iterator i=bd.begin();i!=bd.end() && !bd.empty(); i++) {
    tmp = *i;
    if(tmp->getNumber() == num) {
      unlock();
      return;
    }
  }
  tmp = new Billing_data(num);
  bd.push_back(tmp);
  unlock();
}

// delete module
void
CoreBilling::del(int num) {
  Billing_data *tmp;
  lock();
  for(list<Billing_data*>::iterator i=bd.begin();i!=bd.end() && !bd.empty();) {
    tmp = *i;
    if(tmp->getNumber() == num) {
      i = bd.erase(i);
      delete tmp;
      unlock();
      return;
    }
    i++;
  }
  unlock();
}

// modify shutdown system
void
CoreBilling::modify_shut(int num, bool shut) {
  Billing_data *tmp;
  lock();
  for(list<Billing_data*>::const_iterator i=bd.begin();i!=bd.end() && !bd.empty();i++) {
    tmp = *i;
    if(tmp->getNumber() == num) {
      tmp->setShutdown(shut);
      unlock();
      return;
    }
  }
  unlock();
}

// modify description
void
CoreBilling::modify_des(int num, const char *des) {
  Billing_data *tmp;
  lock();
  for(list<Billing_data*>::const_iterator i=bd.begin();i!=bd.end() && !bd.empty();i++) {
    tmp = *i;
    if(tmp->getNumber() == num) {
      tmp->setDescr(des);
      unlock();
      return;
    }
  }
  unlock();
}

// set/modify billing module
void
CoreBilling::modify_mod(int num,const char *m) {
  Billing_data *tmp;
  lock();
  for(list<Billing_data*>::const_iterator i=bd.begin();i!=bd.end() && !bd.empty();i++) {
    tmp = *i;
    if(tmp->getNumber() == num) {
      tmp->setModule(m);
      break;
    }
  }
  unlock();
}

// load/reload module, 0-success
string
CoreBilling::load(int num) {
  Billing_data *tmp;
  string str;
  lock();
  for(list<Billing_data*>::const_iterator i=bd.begin();i!=bd.end() && !bd.empty();i++) {
    tmp = *i;
    if(tmp->getNumber() == num) {
//std::cout<<"try to load module "<<nas.get(i)->module.get()<<" for "<<nas.get(i)->hostaddr.get()<<std::endl;
      str = tmp->load();
      unlock();
      return str;
    }
  }
  unlock();
  return "";
}

// return modulename
string
CoreBilling::getmodulename(int num) {
  Billing_data *tmp;
  string str;
  lock();
  for(list<Billing_data*>::const_iterator i=bd.begin();i!=bd.end() && !bd.empty();i++) {
    tmp = *i;
    if(tmp->getNumber() == num) {
      str = tmp->getModule();
      unlock();
      return str;
    }
  }
  unlock();
  return "none.so";
}

// build config string
string
CoreBilling::buildcfstr(int num) {
  string str;
  lock();
  if(bd.size() > 0 && (int)bd.size() > num) {
    Billing_data *tmp;
    int count = 0;
    list<Billing_data*>::const_iterator i;
    for(i=bd.begin();i!=bd.end() && count<=num && !bd.empty();i++,count++) {
      if(count==num) {
        tmp = *i;
        if(tmp) str = tmp->buildcfstr();
      }
    }
  }
  unlock();
  return str;
}

// config applyer
string
CoreBilling::applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd)throw (CmdErr) {
  string str="";
  string modNam;
//  cerr<<"---------------cmdName="<<cmdname<<endl;
  if(cmdname=="confBillingNo") {
    del(atoi(paramList.begin()->c_str()));
  } else if (cmdname=="confBillingNode") {
    add(atoi(paramList.begin()->c_str()));
  } else if (cmdname=="confBillingDescr") {
    modify_des(atoi(cmd->upNode->paramList.begin()->c_str()),paramList.begin()->c_str());
  } else if (cmdname=="confBillingModule") {
    modNam=getmodulename(atoi(cmd->upNode->paramList.begin()->c_str()));
    modify_mod(atoi(cmd->upNode->paramList.begin()->c_str()),paramList.begin()->c_str());
    str = load(atoi(cmd->upNode->paramList.begin()->c_str()));
    if(str.size() > 0) {
      modify_mod(atoi(cmd->upNode->paramList.begin()->c_str()),modNam.c_str());
      load(atoi(cmd->upNode->paramList.begin()->c_str()));
    }

  }
  str="";
  return str;
}


db_user
CoreBilling::getUd(string u) {
  lock();
  user_data *tmp;
  db_user ret;
  for(list<user_data*>::const_iterator i=ud_.begin();i!= ud_.end() && !ud_.empty(); i++) {
    tmp = *i;
    if(tmp->get().getUsername() == u) {
      ret = tmp->get();
      break;
    }
  }
  unlock();
  return ret;
}

void
CoreBilling::addUd(db_user d) {
  lock();
  user_data *tmp;
  bool added = false;
  for(list<user_data*>::const_iterator i=ud_.begin();i!= ud_.end() && !ud_.empty(); i++) {
    tmp = *i;
    if(tmp->get().getUsername() == d.getUsername()) {
      tmp->incNumPtr();
      added = true;
      break;
    }
  }
  if(!added) {
    tmp = new user_data(d);
    ud_.push_back(tmp);
  }
  unlock();
}

void
CoreBilling::delUd(string u) {
  lock();
  user_data *tmp;
  for(list<user_data*>::iterator i=ud_.begin();i!= ud_.end() && !ud_.empty();) {
    tmp = *i;
    if(tmp->get().getUsername() == u) {
      if(tmp->getNumPtr() <= 1) {
        i = ud_.erase(i);
        delete tmp;
        continue;
      } else {
        tmp->decNumPtr();
      }
    }
    i++;
  }
  unlock();
}

////////////// that is all ////////////////////
