// tacppd administrators core layer
// (c) Copyright 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

#include "global.h"

///////////////// monitor admin users //////////////////////

// constructor - store username/password/access-list
// no password encryption (from config we have it already encrypted)
User_data::User_data(const char *u,char *p,int a,char *t) {
  pthread_mutex_init(&Mutex_,0);
  username_ = u;
  password_ = p;
  acl_ = a;
  type_ = t;
}

User_data::~User_data() {
  pthread_mutex_destroy(&Mutex_);
}

void
User_data::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
User_data::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

string
User_data::getUsername() {
  lock();
  string ret = username_;
  unlock();
  return ret;
}

void
User_data::setUsername(string d) {
  lock();
  username_ = d;
  unlock();
}

string
User_data::getPassword() {
  lock();
  string ret = password_;
  unlock();
  return ret;
}

void
User_data::setPassword(string d) {
  lock();
  password_ = d;
  unlock();
}

string
User_data::getType() {
  lock();
  string ret = type_;
  unlock();
  return ret;
}

void
User_data::setType(string d) {
  lock();
  type_ = d;
  unlock();
}

int
User_data::getAcl() {
  lock();
  int ret = acl_;
  unlock();
  return ret;
}

void
User_data::setAcl(int d) {
  lock();
  acl_ = d;
  unlock();
}

// check only password
bool
User_data::check(const char *p) {
  unsigned char hash[MD5_LEN+1];
  // 1. cleartext password
  bzero(hash,sizeof(hash));
  create_md5_hash((unsigned char*)hash,(unsigned char*)p);
  hash[MD5_LEN] = 0;
//  char password[64];
//  bzero(password,sizeof(password));
  char ppp[64];
  bzero(ppp,sizeof(ppp));
  snprintf(ppp,sizeof(ppp)-1,"%s",password_.c_str()); // ppp - this is pwd

  char www[64];
  bzero(www,sizeof(www));
  string swww = utl::base64Encode((char*)hash);
  snprintf(www,sizeof(www)-1,"%s",swww.c_str());

//  string sss = utl::base64Decode((char*)ppp); // go from base64
//  snprintf(password,sizeof(password)-1,"%s",sss.c_str());

 // utl::base64Decode(password_.c_str(),password); // decode from base64
  //snprintf(password,sizeof(password)-1,"%s",password_.c_str());

//  if(utl::scomp((char*)ppp,(char*)www,MD5_LEN-1)) return true;
  bool res;
  res = false;
  if(utl::scomp((char*)ppp,(char*)www,MD5_LEN-1)) res = true;
//cerr<<"CoreManager compare p="<<ppp<<" with p1="<<www<<" and result="<<res<<endl;
  return res;

//  lock();
//  char password[200];
//  char cpassword[200];
//  snprintf(cpassword,sizeof(cpassword)-1,"%s",password_.c_str());
//  bzero(password,sizeof(password));
//  utl::base64Decode(password_.c_str(),password); // decode from base64
//  unlock();
//cerr<<"CoreManager compare p1="<<password_<<" with p2="<<p<<endl;
//  if(utl::scomp((char*)hash,(char*)password,strlen(password))) return true;
//  if(utl::scomp(p,cpassword)) return true;
//  return false;
}

// create_md5_hash(): create an md5 hash of the "the user's key",
//
// Write resulting hash into the array pointed to by "hash".
//
// The caller must allocate sufficient space for the resulting hash
// (which is 16 bytes long). The resulting hash can safely be used as
// input to another call to create_md5_hash, as its contents are copied
// before the new hash is generated.
void
User_data::create_md5_hash(unsigned char* hash,unsigned char *varkey) {
  unsigned char *md_stream, *mdp;
  int md_len = 0;
  MD5_CTX mdcontext;

  char username[32];
  lock();
  snprintf(username,sizeof(username)-1,"%s",username_.c_str());
  unlock();

  md_len = strlen(username)+strlen((char*)varkey);
  mdp = md_stream = new unsigned char[md_len];
  mdp = md_stream;
  bcopy(username, mdp, strlen(username));
  mdp += strlen(username);
  bcopy(varkey, mdp, strlen((char*)varkey));
  mdp += strlen((char*)varkey);

  MD5_Init(&mdcontext);
  MD5_Update(&mdcontext, md_stream, md_len);
  MD5_Final(hash, &mdcontext);

  delete [] md_stream;
  md_stream = 0;
  return;
}

////// User

// monitor user constructor
CoreManager::CoreManager(const string n,const string d) : Core_data(n,d,this) {
  pthread_mutex_init(&Mutex_,0);
}

// destructor
CoreManager::~CoreManager() {
  User_data *tmp = 0;
  lock();
  for(list<User_data*>::iterator i=usr_.begin();i!=usr_.end() & !usr_.empty();) {
    tmp = *i;
    i = usr_.erase(i);
    delete tmp;
  }
  usr_.clear();
  unlock();
  pthread_mutex_destroy(&Mutex_);
}

void
CoreManager::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
CoreManager::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

// add new entry to usr list
void
CoreManager::add(const char *u) {
  User_data *tmp;
  // add only if not have already
  lock();
  for(list<User_data*>::const_iterator i=usr_.begin();i!=usr_.end();i++) {
    tmp = *i;
    if(tmp->getUsername() == u) {
      unlock();
      return;
    }
  }
  User_data *x = new User_data(u,"nopassword",0,"terminal");
  usr_.push_back(x);
  unlock();
}

// delete user with name
void
CoreManager::del(const char *u) {
  User_data *tmp = 0;
  // delete entries from usr with username u
  lock();
  for(list<User_data*>::iterator i=usr_.begin();i!=usr_.end();) {
    tmp = *i;
    if(tmp->getUsername() == u) {
      i = usr_.erase(i);
      delete tmp;
      continue;
    }
    i++;
  }
  unlock();
}

// modify parms
// do encryption - set password
void
CoreManager::modify(const char *u,const char *p) {
  unsigned char hash[MD5_LEN+1];
  //char password[256];
  bzero(hash,MD5_LEN);
  User_data *tmp;
Usr_mdf1_Again:;
  lock();
  for(list<User_data*>::const_iterator i=usr_.begin();i!=usr_.end();i++) {
    tmp = *i;
    if(tmp->getUsername() == u) {
//cerr<<"clear password = "<<p<<endl;
      bzero(hash,MD5_LEN);
      tmp->create_md5_hash((unsigned char*)hash,(unsigned char*)p);
      hash[MD5_LEN] = 0;
//cerr<<"hash = "<<hash<<endl;
      //bzero(password,sizeof(password));
      tmp->setPassword(utl::base64Encode((char*)hash)); // go to base64
//cerr<<"base64 = "<<utl::base64Encode((char*)hash)<<endl;
      unlock();
      return;
    }
  }
  // no such. Add and repeat.
  unlock();
  add(u);
  goto Usr_mdf1_Again;
}

// do modify - but password already encrypted
void
CoreManager::cmodify(const char *u,const char *p) {
  User_data *tmp;
Usr_mdf3_Again:;
  lock();
  for(list<User_data*>::const_iterator i=usr_.begin();i!=usr_.end();i++) {
    tmp = *i;
    if(tmp->getUsername() == u) {
      tmp->setPassword(p);
      //snprintf(tmp->password,
      //  sizeof(tmp->password),"%s",p);
      unlock();
      return;
    }
  }
  // no such - add and repeate
  unlock();
  add(u);
  goto Usr_mdf3_Again;
}

// set acl for monitor user
void
CoreManager::modify(const char *u,int a) {
  User_data *tmp;
Usr_mdf2_Again:;
  lock();
  for(list<User_data*>::const_iterator i=usr_.begin();i!=usr_.end();i++) {
    tmp = *i;
    if(tmp->getUsername() == u) {
      tmp->setAcl(a);
      unlock();
      return;
    }
  }
  // add if not here
  unlock();
  add(u);
  goto Usr_mdf2_Again;
}

// set type
void
CoreManager::modify2(const char *u,const char *t) {
  User_data *tmp;
  lock();
  for(list<User_data*>::const_iterator i=usr_.begin();i!=usr_.end();i++) {
    tmp = *i;
    if(tmp->getUsername() == u) {
      tmp->setType(t);
      unlock();
      return;
    }
  }
  // no add if not present
  unlock();
}

// check user with access list and with password
bool
CoreManager::check(const char *u,const char *p,const char *a,const char *t) {
  lock();
  User_data *tmp;
  bool res;
  res = false;
  for(list<User_data*>::const_iterator i=usr_.begin();i!=usr_.end();i++) {
    tmp = *i;
    if(tmp->getUsername() == u) {
//cerr<<"tmp->check(p)="<<tmp->check(p)<<endl;
//cerr<<"::coreData->getAccess()->check(tmp->getAcl(),a)="<<::coreData->getAccess()->check(tmp->getAcl(),a)<<endl;
//cerr<<"tmp->getType()="<<tmp->getType()<<endl;
      if(tmp->check(p) && ::coreData->getAccess()->check(tmp->getAcl(),a) &&
          utl::lower(tmp->getType()) == t) {
        res = true;
      }
      break;
    }
  }
  unlock();
//cerr<<"CoreManager::check="<<(res?"true":"false")<<endl;
  return res;
}

// build only config string
string
CoreManager::buildcfstr(int n) {
  string str;
  lock();
  if((int)usr_.size() > n) {
    User_data *tmp = 0;
    int count = 0;
    list<User_data*>::const_iterator i;
    for(i=usr_.begin();i!=usr_.end() && count!=n;i++,count++);
    if(count == n) tmp = *i;
    if(tmp) {
      str = str + " manager " + tmp->getUsername() + "\n";
      str = str + "  cpassword \'" + tmp->getPassword() + "\'\n";
      str = str + "  type " + tmp->getType() + "\n";
      str = str + "  acl " + utl::toString(tmp->getAcl()) + "\n";
      str = str + "  exit";
    }
  }
  unlock();
  return str;
}

// config applyer
string
CoreManager::applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd)throw (CmdErr) {
 string str;
//  cerr<<"---------------cmdName="<<cmdname<<endl;
  if(cmdname=="confManagerNo") {
      del(paramList.begin()->c_str());
  }
  else if(cmdname=="confManagerNode") {
      add(paramList.begin()->c_str());
  }
  else if(cmdname=="confManagerPwd") {
    modify(cmd->upNode->paramList.begin()->c_str(),paramList.begin()->c_str());
  }
  else if(cmdname=="confManagerCPwd") {
    cmodify(cmd->upNode->paramList.begin()->c_str(),paramList.begin()->c_str());
  }
  else if(cmdname=="confManagerAcl") {
    modify(cmd->upNode->paramList.begin()->c_str(),atoi(paramList.begin()->c_str()));
  }
  else if(cmdname=="confManagerHttp" ||cmdname=="confManagerTerm" ||
  cmdname=="confManagerTci") {
//cerr<<"POINT X="<<mask[1]<<endl;
    modify2(cmd->upNode->paramList.begin()->c_str(),utl::lower(mask[1]).c_str());
  }
  return str;
}

//////////////// end of users ////////////////////
