// tacppd database core layer
// (c) Copyright 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

#include "global.h"

/////////////////////// database list //////////////////

// constructor
CoreDb::CoreDb(const string n,const string d) : Core_data(n,d,this) {
  pthread_mutex_init(&Mutex_,0);
}

// destructor
CoreDb::~CoreDb() {
  lock();
  // delete all from list
  TacDb *tmp = 0;
  for(list<TacDb*>::iterator i=tdb_.begin(); i!=tdb_.end() && !tdb_.empty();) {
    tmp = *i;
    i = tdb_.erase(i);
    delete tmp;
  }
  tdb_.clear();
  unlock();
  pthread_mutex_destroy(&Mutex_);
}

void
CoreDb::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
CoreDb::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

// get user entry
db_user
CoreDb::get(const string us) {
  db_user d;
  // get from database
  TacDb *tmp = 0;
  d.setConnect(false);
  d.setResult(false);
  lock();
  for(list<TacDb*>::const_iterator i=tdb_.begin(); i!=tdb_.end() && !tdb_.empty(); i++) {
    tmp = *i;
    if(!tmp->getShutdown()) {
      unlock();
      d = tmp->get(us);
      lock();
    }
    if(d.getConnect()) {
      break;
    }
  }
  unlock();
  return d;
}

// add log
void
CoreDb::add_log(log_user *l) {
  TacDb *tmp = 0;
  string out;
  lock();
  for(list<TacDb*>::const_iterator i=tdb_.begin(); i!=tdb_.end() && !tdb_.empty();i++) {
    tmp = *i;
    if(!tmp->getShutdown()) {
      unlock();
      out = tmp->add_log(l);
      lock();
      if(out.size() > 0) {
        startlog(LOG_ERROR)<<"msg from TacDb::add_log "<<tmp->getdbid()<<": "<<out<<endlog;
        continue;
      }
      break;
    }
  }
  unlock();
}

// trf log
//void
//CoreDb::add_trf(char *name,SnmpOut *d) {
//  TacDb *tmp = 0;
//  string out;
//  lock();
//  for(list<TacDb*>::iterator i=tdb_.begin(); i!=tdb_.end();i++) {
//    tmp = *i;
//    if(!tmp->getShutdown()) {
//      out = tmp->add_trf(d);
//      if(out.size() > 0) {
//        startlog(LOG_ERROR)<<"msg from TacDb::add_trf "<<tmp->getdbid()<<": "<<out<<endlog;
//        continue;
//      }
//      break;
//    }
//  }
//  unlock();
//}

int
CoreDb::open_nf() {
  TacDb *tmp = 0;
  int cid = 0;
  lock();
  list<TacDb*>::const_iterator i=tdb_.begin();
  tmp = *i;
  unlock();
  if(tmp) {
    if(!tmp->getShutdown()) {
      cid = tmp->open_nf();
      if(cid == 0) {
        startlog(LOG_ERROR)<<"msg from TacDb::open_nf "<<tmp->getdbid()<<": cid=0"<<endlog;
      }
    }
  }
  return cid;
}

void
CoreDb::close_nf(int cid) {
  TacDb *tmp = 0;
  lock();
  list<TacDb*>::const_iterator i=tdb_.begin();
  tmp = *i;
  unlock();
  if(tmp) {
    if(!tmp->getShutdown()) {
      tmp->close_nf(cid);
    }
  }
}

// nf log
void
CoreDb::add_nf(int cid,Bit32 dv,Bit32 sa,Bit32 da,time_t startt,time_t stopt,unsigned long octets,unsigned long packets,int ifin) {
  TacDb *tmp = 0;
  string out;
  lock();
  list<TacDb*>::const_iterator i=tdb_.begin();
  tmp = *i;
  unlock();
  if(tmp) {
    if(!tmp->getShutdown()) {
      out = tmp->add_nf(cid,dv,sa,da,startt,stopt,octets,packets,ifin);
      if(out.size() > 0) {
        startlog(LOG_ERROR)<<"msg from TacDb::add_nf "<<tmp->getdbid()<<": "<<out<<endlog;
      }
    }
  }
}

// direct sql query to database system
string
CoreDb::direct_function_query(const char *q) {
  string str = "";
  TacDb *tmp = 0;
  lock();
  for(list<TacDb*>::const_iterator i=tdb_.begin(); i!=tdb_.end() && !tdb_.empty();i++) {
    tmp = *i;
    unlock();
    str = tmp->direct_function_query((char*)q);
    lock();
    if(str.size() > 0) {
      break;
    }
  }
  unlock();
  return str;
}

// database control specific users

// get database table name
string
CoreDb::get_table_name(const int n) {
  TacDb *tmp = 0;
  string out = "";
  lock();
  list<TacDb*>::const_iterator i=tdb_.begin();
  tmp = *i;
  unlock();
  if(tmp) out = tmp->get_table_name(n);
  return out;
}

// get database name mask
string
CoreDb::get_table_mask(const int n) {
  TacDb *tmp = 0;
  string out = "";
  lock();
  list<TacDb*>::const_iterator i=tdb_.begin();
  tmp = *i;
  if(tmp) out = tmp->get_table_mask(n);
  unlock();
  return out;
}

// get database table description
string
CoreDb::get_table_descr(const int n) {
  TacDb *tmp = 0;
  string out = "";
  lock();
  list<TacDb*>::const_iterator i=tdb_.begin();
  tmp = *i;
  if(tmp) out = tmp->get_table_descr(n);
  unlock();
  return out;
}

// get field name
string
CoreDb::get_field_name(const int t,const int n) {
  TacDb *tmp = 0;
  string out = "";
  lock();
  list<TacDb*>::const_iterator i=tdb_.begin();
  tmp = *i;
  if(tmp) out = tmp->get_field_name(t,n);
  unlock();
  return out;
}

// get field descr
string
CoreDb::get_field_descr(const int t,const int n) {
  TacDb *tmp = 0;
  string out = "";
  lock();
  list<TacDb*>::const_iterator i=tdb_.begin();
  tmp = *i;
  if(tmp) out = tmp->get_field_descr(t,n);
  unlock();
  return out;
}

// get field default value
string
CoreDb::get_field_defv(const int t,const int n) {
  TacDb *tmp = 0;
  string out = "";
  lock();
  list<TacDb*>::const_iterator i=tdb_.begin();
  tmp = *i;
  if(tmp) out = tmp->get_field_defv(t,n);
  unlock();
  return out;
}

// get field type
int
CoreDb::get_field_type(const int t,const int n) {
  TacDb *tmp = 0;
  int out = 0;
  lock();
  list<TacDb*>::const_iterator i=tdb_.begin();
  tmp = *i;
  unlock();
  if(tmp) out = tmp->get_field_type(t,n);
  return out;
}

// get fields number
int
CoreDb::get_field_num(const int t) {
  TacDb *tmp = 0;
  int out = 0;
  lock();
  list<TacDb*>::const_iterator i=tdb_.begin();
  tmp = *i;
  unlock();
  if(tmp) out = tmp->get_field_num(t);
  return out;
}

string
CoreDb::create_all() {
  string ostr = "";
  TacDb *tmp = 0;
  lock();
  for(list<TacDb*>::const_iterator i=tdb_.begin(); i!=tdb_.end() && !tdb_.empty();i++) {
    tmp = *i;
    unlock();
    ostr = ostr + tmp->create();
    lock();
  }
  unlock();
  return ostr;
}

string
CoreDb::add_data(string table,list<string> argument,list<string> value) {
  string ostr = "";
  TacDb *tmp = 0;
  lock();
  for(list<TacDb*>::const_iterator i=tdb_.begin(); i!=tdb_.end() && !tdb_.empty();i++) {
    tmp = *i;
    unlock();
    ostr = ostr + tmp->add(table,argument,value);
    lock();
  }
  unlock();
  argument.clear();
  value.clear();
  return ostr;
}

// modify data in table
string
CoreDb::modify_data(string table,list<string> argument,list<string> value,list<string> argument2,list<string> value2) {
  string ostr = "";
  TacDb *tmp = 0;
  lock();
  for(list<TacDb*>::const_iterator i=tdb_.begin(); i!=tdb_.end() && !tdb_.empty();i++) {
    tmp = *i;
    unlock();
    ostr = ostr + tmp->modify(table,argument,value,argument2,value2);
    lock();
  }
  unlock();
  argument.clear();
  value.clear();
  return ostr;
}

string
CoreDb::del_data(string table,list<string> argument,list<string> value) {
  string ostr = "";
  TacDb *tmp = 0;
  lock();
  for(list<TacDb*>::const_iterator i=tdb_.begin(); i!=tdb_.end() && !tdb_.empty();i++) {
    tmp = *i;
    unlock();
    ostr = ostr + tmp->del(table,argument,value);
    lock();
  }
  unlock();
  argument.clear();
  value.clear();
  return ostr;
}

// stripped show
list<string>
CoreDb::sshow_data(string table,list<string> argument,list<string> value) {
  TacDb *tmp = 0;
  list<string> sout;
  if(tdb_.size() > 0) {
    lock();
    list<TacDb*>::const_iterator i=tdb_.begin();
    tmp = *i;
    unlock();
    sout = tmp->sshow(table,argument,value);
  }
  argument.clear();
  value.clear();
  return sout;
}

string
CoreDb::show_data(string table,list<string> argument,list<string> value) {
  string ostr = "";
  TacDb *tmp = 0;
  if(tdb_.size() > 0) {
    lock();
    list<TacDb*>::const_iterator i=tdb_.begin();
    tmp = *i;
    unlock();
    ostr = tmp->show(table,argument,value);
  }
  argument.clear();
  value.clear();
  return ostr;
}

////// methods for all object work

// add database to list
void
CoreDb::add(const char *loc,unsigned int port,const char *dbname) {
  TacDb *tmp;
  lock();
  for(list<TacDb*>::const_iterator i=tdb_.begin(); i!=tdb_.end() && !tdb_.empty();i++) {
    tmp = *i;
    unlock();
    if(tmp->getLocation()==loc && tmp->getDbport()==port &&
        tmp->getDbname()==dbname) {
      // already have such
      return;
    }
    lock();
  }
  unlock();
  tmp = new TacDb("none.so","public",loc,port,
    "dbuser","dbpassword",dbname);
  lock();
  tdb_.push_back(tmp);
  unlock();
}

// delete database from list
void
CoreDb::del(const char *loc,unsigned int port,const char *dbname) {
  TacDb *tmp = 0;
  lock();
  for(list<TacDb*>::iterator i=tdb_.begin(); i!=tdb_.end() && !tdb_.empty();) {
    tmp = *i;
    if(tmp->getLocation()==loc && tmp->getDbport()==port &&
        tmp->getDbname()==dbname) {
      // have it
      i = tdb_.erase(i);
      delete tmp; // delete database
      continue;
    }
    i++;
  }
  unlock();
}

// set/modify module
void
CoreDb::modify_module(const char *a,unsigned int p,const char *d,const char *m) {
  TacDb *tmp;
  lock();
  for(list<TacDb*>::const_iterator i=tdb_.begin(); i!=tdb_.end() && !tdb_.empty(); i++) {
    tmp = *i;
    if(tmp->getLocation()==a && tmp->getDbport()==p &&
        tmp->getDbname()==d) {
      tmp->setModule(m);
    }
  }
  unlock();
}

// set/modify cryptokey
void
CoreDb::modify2(const char *a,unsigned int p,const char *d,const char *c) {
  TacDb *tmp;
  lock();
  for(list<TacDb*>::const_iterator i=tdb_.begin(); i!=tdb_.end() && !tdb_.empty(); i++) {
    tmp = *i;
    if(tmp->getLocation()==a &&
      tmp->getDbport()==p &&
        tmp->getDbname()==d) {
      tmp->setCryptokey(c);
    }
  }
  unlock();
}

// set/modify username
void
CoreDb::modify4(const char *a,unsigned int p,const char *d,const char *u) {
  TacDb *tmp;
  lock();
  for(list<TacDb*>::const_iterator i=tdb_.begin(); i!=tdb_.end() && !tdb_.empty(); i++) {
    tmp = *i;
    if(tmp->getLocation()==a &&
      tmp->getDbport()==p &&
        tmp->getDbname()==d) {
      tmp->setUsername(u);
    }
  }
  unlock();
}

// set/modify password
void
CoreDb::modify5(const char *a,unsigned int p,const char *d,const char *pw) {
  TacDb *tmp;
  lock();
  for(list<TacDb*>::const_iterator i=tdb_.begin(); i!=tdb_.end() && !tdb_.empty(); i++) {
    tmp = *i;
    if(tmp->getLocation()==a &&
      tmp->getDbport()==p &&
        tmp->getDbname()==d) {
      tmp->setPassword(pw);
    }
  }
  unlock();
}

// set/modify shutdown
void
CoreDb::modify_shutdown(const char *a,unsigned int p,const char *d,bool s) {
  TacDb *tmp;
  lock();
  for(list<TacDb*>::const_iterator i=tdb_.begin(); i!=tdb_.end() && !tdb_.empty(); i++) {
    tmp = *i;
    if(tmp->getLocation()==a &&
      tmp->getDbport()==p &&
        tmp->getDbname()==d) {
      tmp->setShutdown(s);
    }
  }
  unlock();
}

// load/reload database module
// 0 - success
string
CoreDb::load(const char *a,unsigned int p,const char *d) {
  string str = "";
  TacDb *tmp;
  lock();
  for(list<TacDb*>::const_iterator i=tdb_.begin(); i!=tdb_.end() && !tdb_.empty(); i++) {
    tmp = *i;
    if(tmp->getLocation()==a &&
      tmp->getDbport()==p &&
        tmp->getDbname()==d) {
std::cerr<<"host="<<a<<" port="<<p<<" db="<<d<<std::endl;
      str = tmp->load();
std::cerr<<"result="<<str<<std::endl;
      unlock();
      return str;
    }
  }
  unlock();
  return "";
}

// get module name
string
CoreDb::getmodulename(const char *a,unsigned int p,const char *d) {
  string str = "";
  TacDb *tmp;
  lock();
  for(list<TacDb*>::const_iterator i=tdb_.begin(); i!=tdb_.end() && !tdb_.empty(); i++) {
    tmp = *i;
    if(tmp->getLocation()==a &&
      tmp->getDbport()==p &&
        tmp->getDbname()==d) {
      str = tmp->getModule();
      unlock();
      return str;
    }
  }
  unlock();
  return "";
}

// build config string
string
CoreDb::buildcfstr(int n) {
  string str = "";
  lock();
  if((int)tdb_.size()>n && !tdb_.empty()) {
    TacDb *tmp = 0;
    list<TacDb*>::const_iterator it;
    int count = 0;
    for(it=tdb_.begin(); it!=tdb_.end() && (count<n) && !tdb_.empty(); it++,count++);
    if(count == n) {
      tmp = *it;
      if(tmp) {
        str = str + " database " + tmp->getdbid() + "\n";
        str = str + "  cryptokey \'" + tmp->getCryptokey() + "\'\n";
        str = str + "  login " + tmp->getUsername() + "\n";
        str = str + "  password " + tmp->getPassword() + "\n";
        str = str + "  module " + tmp->getModule() + "\n";
        str = str + "  shutdown " + (tmp->getShutdown() ? "yes" : "no") + "\n";
        str = str + "  exit";
      }
    }
  }
  unlock();
  return str;
}

// apply configuration
string
CoreDb::applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd)throw
(CmdErr) {
  string str = "";
  string dbDescr;
  // should be dbname@dbhost:port
  char dbname[64];
  char IpAddr[17];
  int port;
//cerr<<"---------------cmdName="<<cmdname<<endl;
  if(cmdname=="confDbNo" || cmdname=="confDbNode") dbDescr=*(paramList.begin());
  else dbDescr=*(cmd->upNode->paramList.begin());

  if(utl::splitId(dbDescr.c_str(),0).size()==0 || utl::splitId(dbDescr.c_str(),1).size()==0 ||
        utl::splitId(dbDescr.c_str(),2).size()==0) {
    str = "% wrong database descriptor /" + dbDescr+ "/ you must use databasename@ipaddress:port form";
    return str;
  }

  //std::cerr<<"v1="<<v1<<std::endl;
  //std::cerr<<"DbName="<<utl::splitId(v1,0)<<std::endl;
  //std::cerr<<"DbIp="<<utl::splitId(v1,1)<<std::endl;
  //std::cerr<<"DbPort="<<utl::splitId(v1,2)<<std::endl;
  snprintf(dbname,sizeof(dbname)-1,"%s",utl::splitId(dbDescr.c_str(),0).c_str());
  ipaddr dbhost;
  snprintf(IpAddr,sizeof(IpAddr)-1,"%s",utl::splitId(dbDescr.c_str(),1).c_str());
  dbhost = IpAddr;
  //cerr<<"IpAddr="<<IpAddr<<" dbhost="<<dbhost.get()<<endl;
  port = atoi(utl::splitId(dbDescr.c_str(),2).c_str());

//std::cerr<<"dbname="<<dbname<<" dbhost="<<dbhost.get()<<" port="<<port<<std::endl;

  if(cmdname=="confDbNo") {
    del(IpAddr,port,dbname);  // drop whole device
  } else if (cmdname=="confDbNode") {
    add(IpAddr,port,dbname);
    str = load(IpAddr,port,dbname);
  } else if (cmdname=="confDbCrypto") {
    modify2(IpAddr,port,dbname,paramList.begin()->c_str());
  } else if (cmdname=="confDbLogin") {
    modify4(IpAddr,port,dbname,paramList.begin()->c_str());
  } else if (cmdname=="confDbPasswd") {
    modify5(IpAddr,port,dbname,paramList.begin()->c_str());
  } else if (cmdname=="confDbModule") {
    dbDescr = getmodulename(IpAddr,port,dbname);
    modify_module(IpAddr,port,dbname,paramList.begin()->c_str());
    str = load(IpAddr,port,dbname);
    if(strlen(str.c_str()) > 0) {
      modify_module(IpAddr,port,dbname,dbDescr.c_str());
      load(IpAddr,port,dbname);
    }
  } else if (cmdname=="confDbShutdown") {
    modify_shutdown(IpAddr,port,dbname,utl::match(paramList.begin()->c_str(),"No") ? false : true);
  }
  return str;
}

/////////////////// end of database list //////////////
