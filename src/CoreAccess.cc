// tacppd Access lists core layer
// (c) Copyright 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

#include "global.h"

///////////// Acl_entry /////////////

// constructor
Acl_entry::Acl_entry(const string p,const string d) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  permit_ = p;
  deny_ = d;
  unlock();
}

Acl_entry::~Acl_entry() {
  pthread_mutex_destroy(&Mutex_);
}

void
Acl_entry::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
Acl_entry::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

string
Acl_entry::getPermit() {
  lock();
  string ret = permit_;
  unlock();
  return ret;
}

void
Acl_entry::setPermit(const string s) {
  lock();
  permit_ = s;
  unlock();
}

string
Acl_entry::getDeny() {
  lock();
  string ret = deny_;
  unlock();
  return ret;
}

void
Acl_entry::setDeny(const string s) {
  lock();
  deny_ = s;
  unlock();
}

// check given information with regex
bool
Acl_entry::checkPermit(const string st) {
  lock();
  bool filter = false;
  regex_t preg;
  size_t nmatch = 2;
  regmatch_t pmatch[2];
  char s[300];
  char permit[300];
  snprintf(s,sizeof(s)-1,"%s",st.c_str());
  snprintf(permit,sizeof(permit)-1,"%s",permit_.c_str());
  if(strlen(permit) > 0) {
    if(regcomp(&preg,permit,REG_EXTENDED) == 0) {
      if(regexec(&preg,s,nmatch,pmatch,0) == 0) filter = true;
      regfree(&preg);
    }
  }
  unlock();
  return filter;
}

bool
Acl_entry::checkDeny(const string st) {
  lock();
  bool filter = true;
  regex_t preg;
  size_t nmatch = 2;
  regmatch_t pmatch[2];
  char s[300];
  char deny[300];
  snprintf(s,sizeof(s)-1,"%s",st.c_str());
  snprintf(deny,sizeof(deny)-1,"%s",deny_.c_str());
  if(strlen(deny) > 0) {
    if(regcomp(&preg,deny,REG_EXTENDED) == 0) {
      if(regexec(&preg,s,nmatch,pmatch,0) == 0) filter = false;
      regfree(&preg);
    }
  }
  unlock();
  return filter;
}

///////////// Acl_data /////////////

// constructor
Acl_data::Acl_data(const int num) {
  pthread_mutex_init(&Mutex_,0);
  lock();
  number_ = num;
  unlock();
}

// destructor
Acl_data::~Acl_data() {
  Acl_entry *tmp;
  lock();
  for(list<Acl_entry*>::iterator i=acl_.begin();i!=acl_.end() && !acl_.empty();) {
    tmp = *i;
    i = acl_.erase(i);
    delete tmp;
  }
  acl_.clear();
  unlock();
  pthread_mutex_destroy(&Mutex_);
}

void
Acl_data::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
Acl_data::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

int
Acl_data::getNumber() {
  lock();
  int ret = number_;
  unlock();
  return ret;
}

// check given information with regex
bool
Acl_data::check(const char *s) {
  Acl_entry *tmp;
  lock();
  for(list<Acl_entry*>::const_iterator i=acl_.begin();i!=acl_.end();i++) {
    tmp = *i;
    if(!tmp->checkDeny(s)) {
      unlock();
      return false; // got in deny
    }
    if(tmp->checkPermit(s)) {
      unlock();
      return true; // got in permit
    }
  }
  unlock();
  return false;
}

// build cfg string
string
Acl_data::buildcfstr() {
  Acl_entry *tmp;
  lock();
  string str = " access " + utl::toString(number_) + "\n";
  for(list<Acl_entry*>::const_iterator i=acl_.begin();i!=acl_.end();i++) {
    tmp = *i;
    if(tmp->getPermit().size() > 0)
      str = str + "  permit " + tmp->getPermit() + "\n";
    if(tmp->getDeny().size() > 0)
      str = str + "  deny " + tmp->getDeny() + "\n";
  }
  str += "  exit";
  unlock();
  return str;
}

// add permit in this acl
bool
Acl_data::add_permit(const char *p) {
  if(p && strlen(p)) {
    Acl_entry *tmp;
    tmp = new Acl_entry(p,"");
    lock();
    acl_.push_back(tmp);
    unlock();
    return true;
  }
  return false;
}

// add deny in this acl
bool
Acl_data::add_deny(const char *d) {
  if(d && strlen(d)) {
    Acl_entry *tmp;
    tmp = new Acl_entry("",d);
    lock();
    acl_.push_back(tmp);
    unlock();
    return true;
  }
  return false;
}

// del permit from this acl
bool
Acl_data::del_permit(const char *p) {
  if(p && strlen(p)) {
    Acl_entry *tmp;
    lock();
    for(list<Acl_entry*>::iterator i=acl_.begin();i!=acl_.end();i++) {
      tmp = *i;
      if(tmp->getPermit() == p) {
        i = acl_.erase(i);
        delete tmp;
        unlock();
        return true;
      }
    }
    unlock();
  }
  return false;
}

// del deny from this acl
bool
Acl_data::del_deny(const char *d) {
  if(d && strlen(d)) {
    Acl_entry *tmp;
    lock();
    for(list<Acl_entry*>::iterator i=acl_.begin();i!=acl_.end();i++) {
      tmp = *i;
      if(tmp->getDeny() == d) {
        i = acl_.erase(i);
        delete tmp;
        unlock();
        return true;
      }
    }
    unlock();
  }
  return false;
}


//////////////// CoreAccess ///////////////////

CoreAccess::CoreAccess(const string n,const string d) : Core_data(n,d,this) {
  pthread_mutex_init(&Mutex_,0);
}

// destructor
CoreAccess::~CoreAccess() {
  Acl_data *tmp = 0;
  lock();
  for(list<Acl_data*>::iterator i=acl_.begin();i!=acl_.end() && !acl_.empty();) {
    tmp = *i;
    i = acl_.erase(i);
    delete tmp;
  }
  acl_.clear();
  unlock();
  pthread_mutex_destroy(&Mutex_);
}

void
CoreAccess::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
CoreAccess::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

// create new void access list
void
CoreAccess::add(int n) {
  Acl_data *tmp = 0;
  lock();
  for(list<Acl_data*>::const_iterator i=acl_.begin();i!=acl_.end() && !acl_.empty(); i++) {
    tmp = *i;
    if(tmp->getNumber() == n) { // such acl already created
      unlock();
      return;
    }
  }
  Acl_data *x = new Acl_data(n);
  acl_.push_back(x);
  unlock();
}

// add new entry to acl list
bool
CoreAccess::add_permit(int p,const char *r) {
  Acl_data *tmp;
  lock();
  for(list<Acl_data*>::const_iterator i=acl_.begin();i!=acl_.end() && !acl_.empty(); i++) {
    tmp = *i;
    if(tmp->getNumber() == p) {
      tmp->add_permit(r);
      unlock();
      return true;
    }
  }
  // if no created acl with this number - create it
  Acl_data *x = new Acl_data(p);
  x->add_permit(r);
  acl_.push_back(x);
  unlock();
  return true;
}

// add new entry to acl list
bool
CoreAccess::add_deny(int p,const char *r) {
  Acl_data *tmp;
  lock();
  for(list<Acl_data*>::const_iterator i=acl_.begin();i!=acl_.end() && !acl_.empty(); i++) {
    tmp = *i;
    if(tmp->getNumber() == p) {
      tmp->add_deny(r);
      unlock();
      return true;
    }
  }
  // if no created acl with this number - create it
  Acl_data *x;
  x = new Acl_data(p);
  x->add_deny(r);
  acl_.push_back(x);
  unlock();
  return true;
}

// delete all from access-list number p
bool
CoreAccess::del(int p) {
  bool res = false;
  Acl_data *tmp;
  tmp = 0;
  // delete entries from acl with number a
  lock();
  for(list<Acl_data*>::iterator i=acl_.begin();i!=acl_.end() && !acl_.empty();) {
    tmp = *i;
    if(tmp->getNumber() == p) {
      i = acl_.erase(i);
      delete tmp;
      res = true;
    } else i++;
  }
  unlock();
  return res;
}

// delete rule from access list
bool
CoreAccess::del_permit(int p,const char *r) {
  bool res = false;
  Acl_data *tmp = 0;
  lock();
  for(list<Acl_data*>::const_iterator i=acl_.begin();i!=acl_.end() && !acl_.empty();i++) {
    tmp = *i;
    if(tmp->getNumber() == p) {
      tmp->del_permit(r);
      res = true;
      break;
    }
  }
  unlock();
  return res;
}

// delete rule from access list
bool
CoreAccess::del_deny(int p,const char *r) {
  bool res = false;
  Acl_data *tmp = 0;
  lock();
  for(list<Acl_data*>::const_iterator i=acl_.begin();i!=acl_.end() && !acl_.empty();i++) {
    tmp = *i;
    if(tmp->getNumber() == p) {
      tmp->del_deny(r);
      res = true;
      break;
    }
  }
  unlock();
  return res;
}

// check rule with access list
bool
CoreAccess::check(int p,const char *r) {
  bool res = false;
  Acl_data *tmp = 0;
  lock();
  for(list<Acl_data*>::const_iterator i=acl_.begin();i!=acl_.end() && !acl_.empty();i++) {
    tmp = *i;
    if(tmp->getNumber() == p) {
      res = tmp->check(r);
      if(res) {
        unlock();
        return res;
      }
    }
  }
  unlock();
  return res;
}

// build 1 cfg string
string
CoreAccess::buildcfstr(int n) {
  string str;
  lock();
  if(acl_.size() > 0 && (int)acl_.size() > n) {
    Acl_data *tmp = 0;
    int count = 0;
    list<Acl_data*>::const_iterator i;
    for(i=acl_.begin();i!=acl_.end() && count<=n;i++,count++) {
      if(count==n) {
        tmp = *i;
        if(tmp) str = tmp->buildcfstr();
      }
    }
  }
  unlock();
  return str;
}

// apply configuration
string
CoreAccess::applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd)throw (CmdErr) {
  string str;
//  cerr<<"---------------cmdName="<<cmdname<<endl;
  if(cmdname=="confAccessNo") {
      if(del(atoi(paramList.begin()->c_str()))) return "";
      str= "% can not delete acl # " + *(paramList.begin());
  }
  else if(cmdname=="confAccessNode") {
    add(atoi(paramList.begin()->c_str()));
    return "";
  }
  else if(cmdname=="confAccessIntNoDeny") {
          if(del_deny(atoi(cmd->upNode->paramList.begin()->c_str()),paramList.begin()->c_str()))return "";
          str="% can not delete deny rule "    +*(cmd->upNode->paramList.begin())+    "in acl # " + *(cmd->paramList.begin());
  }
  else if(cmdname=="confAccessIntNoPermit") {
          if(del_permit(atoi(cmd->upNode->paramList.begin()->c_str()),paramList.begin()->c_str())) return "";
          str="% can not delete permit rule "    +*(cmd->upNode->paramList.begin())+    "in acl # " + *(cmd->paramList.begin());
  }
  else if(cmdname=="confAccessDeny") {
      if (add_deny(atoi(cmd->upNode->paramList.begin()->c_str()),paramList.begin()->c_str()))return "";
      str="% can not add deny rule "   +*(cmd->upNode->paramList.begin())+    "in acl # " + *(cmd->paramList.begin());
    }
  else if(cmdname=="confAccessPermit") {
    if(add_permit(atoi(cmd->upNode->paramList.begin()->c_str()),paramList.begin()->c_str()))return "";
    str="% can not add permit rule "   +*(cmd->upNode->paramList.begin())+    "in acl # " + *(cmd->paramList.begin());
  }
  return str;
}

////////////////// end of access-lists ////////////////
