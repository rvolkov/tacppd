// tacppd tcp/udp listener core layer
// (c) Copyright 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

#include "global.h"

////////////////// Listener_data //////////////////

// st = TCP or UDP
// mc - maximum open connections
Listener_data::Listener_data(unsigned int serverport, int st, int mc,
    void (*uf)(int,struct in_addr,int,char*),int a,pthread_mutex_t *tmutex) {
  pthread_mutex_init(&Mutex_,0);
  pthread_mutex_init(&mutex2_,0);
  lock();
  UsersFunction = uf; // setup ptr to user function
  port_ = serverport; // set port
  servertype_ = st; // set server type
  maxconnect_ = mc; // max connections
  acl_ = a; // access list
  created_ = true; //
  qDescrNum_ = 0; // 0 sessions
  lmutex_ = tmutex; // store global listeners mutex
  unlock();
}

bool
Listener_data::isCreated() {
  lock();
  bool ret = created_;
  unlock();
  return ret;
}

void
Listener_data::setCreated(bool d) {
  lock();
  created_ = d;
  unlock();
}

void
Listener_data::create() {
}

// destructor (void)
Listener_data::~Listener_data() {
  pthread_mutex_destroy(&Mutex_);
  pthread_mutex_destroy(&mutex2_);
}

// destroy server
void
Listener_data::destroy() {
  startlog(LOG_SERVER)<<"DESTROY SERVER"<<endlog;
  setCreated(false);
}

void
Listener_data::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
Listener_data::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

void
Listener_data::lock2() {
//cerr<<"before Listener_data::lock2()"<<endl;
  pthread_mutex_lock(&mutex2_);
//cerr<<"after Listener_data::lock2()"<<endl;
}

void
Listener_data::unlock2() {
//cerr<<"before Listener_data::unlock2()"<<endl;
  pthread_mutex_unlock(&mutex2_);
//cerr<<"after Listener_data::unlock2()"<<endl;
}

void
Listener_data::llock() {
  pthread_mutex_lock(lmutex_);
}

void
Listener_data::lunlock() {
  pthread_mutex_unlock(lmutex_);
}

int
Listener_data::add_session(Connect *NewConnect) {
  lock();
  qDescrNum_++;
  NewConnect->number = qDescrNum_;
  int ret = qDescrNum_;
  unlock();
  return ret;
}

void
Listener_data::del_session(int n) {
  lock();
  // delete n
  qDescrNum_--;
  if(qDescrNum_ < 0) qDescrNum_ = 0;
  unlock();
}

int
Listener_data::con_session() {
  lock();
  int ret = qDescrNum_;
  unlock();
  return ret;
}

void
Listener_data::setServname(string d) {
  lock();
  servname_ = d;
  unlock();
}

string
Listener_data::getServname() {
  lock();
  string ret = servname_;
  unlock();
  return ret;
}

void
Listener_data::setMaxconnect(int d) {
  lock();
  maxconnect_ = d;
  unlock();
}

int
Listener_data::getMaxconnect() {
  lock();
  int ret = maxconnect_;
  unlock();
  return ret;
}

void
Listener_data::setAcl(int d) {
  lock();
  acl_ = d;
  unlock();
}

int
Listener_data::getAcl() {
  lock();
  int ret = acl_;
  unlock();
  return ret;
}

void
Listener_data::setPort(unsigned int d) {
  lock();
  port_ = d;
  unlock();
}

unsigned int
Listener_data::getPort() {
  lock();
  unsigned int ret = port_;
  unlock();
  return ret;
}

void
Listener_data::setServertype(int d) {
  lock();
  servertype_ = d;
  unlock();
}

int
Listener_data::getServertype() {
  lock();
  int ret = servertype_;
  unlock();
  return ret;
}


////////////////// servers section ////////////////

// constructor
CoreListener::CoreListener(const string n,const string d) : Core_data(n,d,this) {
  pthread_mutex_init(&Mutex_,0);
  pthread_mutex_init(&LMutex_,0);
}

// destructor
CoreListener::~CoreListener() {
  Listener_data *tmp = 0;
  lock();
  for(list<Listener_data*>::iterator i=srv_.begin();i!=srv_.end() && !srv_.empty();) {
    tmp = *i;
    i = srv_.erase(i);
    tmp->destroy();
    sleep(2); // wait some for end thread ??? - can be very strange results
    delete tmp;
  }
  srv_.clear();
  unlock();
  pthread_mutex_destroy(&LMutex_);
  pthread_mutex_destroy(&Mutex_);
}

void
CoreListener::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
CoreListener::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

// init - start threads
void
CoreListener::init() {
  Listener_data *tmp = 0;
  lock();
  for(list<Listener_data*>::const_iterator i=srv_.begin();i!=srv_.end();i++) {
    tmp = *i;
    if(tmp->isCreated() && tmp->UsersFunction) {
      pthread_t threadvar_;
      pthread_attr_t tattr; // thread attribute
      pthread_attr_init(&tattr); // init thread attribute
      pthread_attr_setdetachstate(&tattr,PTHREAD_CREATE_DETACHED);
      if(pthread_create(&threadvar_,&tattr,
          (void*(*)(void*))&AcceptConnection, (void*)tmp) != 0) {
        startlog(LOG_ERROR)<<"Error create AcceptConnection thread"<<endlog;
        unlock(); //
        return;
      }
      std::cerr<<"Listener "<<tmp->getServname()<<" now configured at port "<<
        (tmp->getServertype()==TCP?"tcp":"udp")<<"/"<<tmp->getPort()<<endl;
      pthread_attr_destroy(&tattr);
    }
  }
  unlock();
}

// adding
void
CoreListener::add(unsigned int p,int m,const char *s,int a) {
  Listener_data *tmp;
  lock();
  for(list<Listener_data*>::const_iterator i=srv_.begin();i!=srv_.end();i++) {
    tmp = *i;
    if(tmp->getPort() == p) { // such port already occupyed
      //startlog(LOG_ERROR)<<"Listener: port "<<p<<" already occupyed"<<endlog;
      unlock();
      return;
    }
  }
  int t = 0;
  void (*f)(int,struct in_addr,int,char*) = 0;
  if(utl::scomp(s,"null")) {
    f = 0;
    t = TCP;
  } else if(utl::scomp(s,"tacacs")) {
    f = tacpp::tacacs_server;
    t = TCP;
  } else if(utl::scomp(s,"radius")) {
    f = tacpp::radius_server;
    t = UDP;
  } else if(utl::scomp(s,"radacct")) {
    f = tacpp::radacct_server;
    t = UDP;
  } else if(utl::scomp(s,"http")) {
    f = tacpp::http_server;
    t = TCP;
#ifdef WITH_XMLRPC
  } else if(utl::scomp(s,"xmlrpc")) {
    f = tacpp::xmlrpc_server;
    t = XMLRPC;
#endif //WITH_XMLRPC
  } else if(utl::scomp(s,"terminal")) {
    f = tacpp::terminal_server;
    t = TCP;
  } else if(utl::scomp(s,"netflow")) {
    f = tacpp::netflow_server;
    t = UDP;
  } else if(utl::scomp(s,"bundle")) {
    f = tacpp::bundle_server;
    t = UDP_MULTICAST;
  } else if(utl::scomp(s,"peer")) {
    f = tacpp::peer_server;
    t = TCP;
//  } else if(utl::scomp(s,"syslog")) {
//
  } else
//  if(utl::scomp(s,"traps")) {
//    f = tacpp::traps_server;
//    t = UDP;
//  }
//  if(utl::scomp(s,"snmp")) {
//    f = tacpp::snmp_server;
//    t = UDP;
//  }
  if(utl::scomp(s,"tci")) {
    f = tacpp::tci_server;
    t = TCP;
  } else {
    startlog(LOG_ERROR)<<"Cant create unknown server type "<<s<<endlog;
    unlock();
    return;
  }
  Listener_data *x = new Listener_data(p,t,m,f,a,&LMutex_);
  x->setServname(s);
  srv_.push_back(x);
  unlock();
}

// add new server
void
CoreListener::add(unsigned int p) {
  add(p,0,"null",0);
}

// delete
void
CoreListener::del(unsigned int p) {
  Listener_data *tmp = 0;
  lock();
  for(list<Listener_data*>::iterator i=srv_.begin();i!=srv_.end();) {
    tmp = *i;
    if(tmp->getPort() == p) {
      i = srv_.erase(i);
      tmp->destroy();
      sleep(1);
      delete tmp; // threads will be deleted by destructor
      continue;
    }
    i++;
  }
  unlock();
}

// modify server type
void
CoreListener::modify1(unsigned int p,const char *s) {
  Listener_data *tmp = 0;
  // should recreate server - technology require it
  lock();
  for(list<Listener_data*>::iterator i=srv_.begin();i!=srv_.end();) {
    tmp = *i;
    if(tmp->getPort() == p) {
      i = srv_.erase(i);
      int maxconnect = tmp->getMaxconnect();
      int acl = tmp->getAcl();
      tmp->destroy();
      unlock();
      sleep(1);
      lock();
      delete tmp; // delete current server
      unlock();
      add((unsigned int)p,maxconnect,s,acl); // create new
      return;
    }
    i++;
  }
  // if no server - create void
  unlock();
  add(p,0,s,0);
}

// modify maxconnections
void
CoreListener::modify2(unsigned int p,int m) {
  Listener_data *tmp;
 srv_modify2_again:;
  lock();
  // no server recreate - only change parameter
  for(list<Listener_data*>::const_iterator i=srv_.begin();i!=srv_.end();i++) {
    tmp = *i;
    if(tmp->getPort() == p) {
      tmp->setMaxconnect(m);
      unlock();
      return;
    }
  }
  // if no server created, add void
  unlock();
  add(p,m,"null",0);
  goto srv_modify2_again;
}

// modify acl
void
CoreListener::modify3(unsigned int p,int a) {
  Listener_data *tmp;
 srv_modify3_again:;
  // only change
  lock();
  for(list<Listener_data*>::const_iterator i=srv_.begin();i!=srv_.end();i++) {
    tmp = *i;
    if(tmp->getPort() == p) {
      tmp->setAcl(a);
      unlock();
      return;
    }
  }
  // if no, create void
  unlock();
  add(p,0,"null",a);
  goto srv_modify3_again;
}

// check by acl
bool
CoreListener::check(unsigned int p,const char *a) {
  Listener_data *tmp;
  bool res = false;
  lock();
  for(list<Listener_data*>::const_iterator i=srv_.begin();i!=srv_.end();i++) {
    tmp = *i;
    if(tmp->getPort() == p) {
      if(::coreData->getAccess()->check(tmp->getAcl(), a)) {
        res = true;
        unlock();
        return res;
      }
    }
  }
  unlock();
  return res;
}

// build only config string
string
CoreListener::buildcfstr(int n) {
  string str = "";
  lock();
  if((int)srv_.size() > n) {
    int count=0;
    Listener_data *tmp = 0;
    list<Listener_data*>::const_iterator i;
    for(i=srv_.begin();i!=srv_.end() && count!=n;i++,count++);
    if(count == n) tmp = *i;
    if(tmp) {
      str = str + " listener " + utl::toString(tmp->getPort()) + "\n";
      str = str + "  type " + tmp->getServname() + "\n";
      str = str + "  maxconnect " + utl::toString(tmp->getMaxconnect()) + "\n";
      str = str + "  acl " + utl::toString(tmp->getAcl()) + "\n";
      str = str + "  exit";
    }
  }
  unlock();
  return str;
}

// apply configuration
string
CoreListener::applycf(char * const *mask,const string cmdname,const list<string>paramList,Command * cmd)throw (CmdErr) {
 string str = "";
//  cerr<<"---------------cmdName="<<cmdname<<endl;
  if(cmdname == "confListenerNo") {
    del(atoi(paramList.begin()->c_str()));
  } else if(cmdname=="confListenerNode") {
    add(atoi(paramList.begin()->c_str()));

//    cerr<<"Configure Listener on port "<<paramList.begin()->c_str()<<endl;

//  } else if(cmdname=="confListenerTac" || cmdname=="confListenerHttp" ||
//  cmdname == "confListenerTerm" || cmdname == "confListenerNetf" ||
//  cmdname == "confListenerBundle" || cmdname == "confListenerPeer" ||
//  cmdname == "confListenerTraps" || cmdname == "confListenerSnmp" ||
//  cmdname == "confListenerTci" || cmdname == "confListenerXml" ||
//  cmdname == "confListenerRad" || cmdname == "confListenerRadacc" ||
//  cmdname == "confListenerSyslog" ) {
////cerr<<"****** "<<atoi(cmd->upNode->paramList.begin()->c_str())<<" "<<mask[1]<<" //*****"<<endl;
//    modify1(atoi(cmd->upNode->paramList.begin()->c_str()), //utl::lower(mask[1]).c_str());

  } else if(cmdname=="confListenerMaxconnect") {
    modify2(atoi(cmd->upNode->paramList.begin()->c_str()), atoi(paramList.begin()->c_str()));
  } else if(cmdname=="confListenerAcl") {
    modify3(atoi(cmd->upNode->paramList.begin()->c_str()), atoi(paramList.begin()->c_str()));
  } else {
    modify1(atoi(cmd->upNode->paramList.begin()->c_str()), utl::lower(mask[1]).c_str());
  }
  return str;
}

///////////////// end of servers ////////////////
