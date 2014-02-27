// tacppd logging code
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

#include "global.h"

// class LogData

// constructor
LogData::LogData(int n,string s) {
  pthread_mutex_init(&mutex_,0);
  lock();
  level = n;
  time_t t = time(0);
  struct tm res;
  struct tm *tt = localtime_r(&t,&res);
  char strd[60];
  bzero(strd,sizeof(strd));
  snprintf(strd,sizeof(strd)-1,"[%04d/%02d/%02d-%02d-%02d:%02d:%02d] ",
    1900+tt->tm_year,tt->tm_mon+1,tt->tm_mday,tt->tm_wday+1,
      tt->tm_hour,tt->tm_min,tt->tm_sec);
  char strt[30];
  bzero(strt,sizeof(strt));
  snprintf(strt,sizeof(strt)-1,"%s: ",utl::upper(coreData->getDebug()->fname(n)).c_str());
  if(strlen(strt)>2 && strlen(strt)<30) strcat(strd,strt);
  //int length = 0;
  if(s.size()>0) {
    //length = strlen(s) + strlen(strd) + 2;
    str_ = str_ + strd + s;
  } else {
    //char sss[30];
    //snprintf(sss,sizeof(sss)-1,"(void)");
    //length = strlen(sss) + strlen(strd) + 2;
    str_ = str_ + strd + "(void)";
  }
  is_written_ = false;
  unlock();
}

// destructor
LogData::~LogData() {
  pthread_mutex_destroy(&mutex_);
}

void
LogData::lock() {
  pthread_mutex_lock(&mutex_);
}

void
LogData::unlock() {
  pthread_mutex_unlock(&mutex_);
}

// get entry logging level
int
LogData::get_level() {
  int ret;
  lock();
  ret = level;
  unlock();
  return ret;
}

// get logging string
string
LogData::get_string() {
  string ret;
  lock();
  ret = str_;
  unlock();
  return ret;
}

bool
LogData::isWritten() {
  bool ret;
  lock();
  ret = is_written_;
  unlock();
  return ret;
}

void
LogData::setWritten(bool a) {
  lock();
  is_written_ = a;
  unlock();
}

//////// class LogList

// constructor
LogList::LogList(int n) {
  pthread_mutex_init(&mutex_,0);
  lock();
  num_ = n; // set id number
  unlock();
}

// destructor
LogList::~LogList() {
  lock();
  LogData *tmp;
  for(list<LogData*>::iterator i=logs.begin();i!=logs.end() && !logs.empty();) {
    tmp = *i;
    i = logs.erase(i);
    delete tmp;
  }
  logs.clear();
  unlock();
  pthread_mutex_destroy(&mutex_);
}

void
LogList::lock() {
  pthread_mutex_lock(&mutex_);
}

void
LogList::unlock() {
  pthread_mutex_unlock(&mutex_);
}

int
LogList::getNum() {
  lock();
  int ret = num_;
  unlock();
  return ret;
}

// add entry to log list
void
LogList::add(int n,string s) {
  lock();
  LogData *tmp = new LogData(n, s); // create log entry
  logs.push_back(tmp); // add it to list
  unlock();
}

string
LogList::del(int n) {
  lock();
  LogData *tmp;
  string stmp;
  for(list<LogData*>::iterator i=logs.begin();i!=logs.end();i++) {
    tmp = *i;
    if(tmp->get_level() == n) {
      i = logs.erase(i);
      stmp = tmp->get_string();
      delete tmp;
      unlock();
      return stmp;
    }
  }
  unlock();
  return stmp;
}

LogData*
LogList::getEntry() {
  lock();
  LogData *tmp;
  tmp = 0;
  if(!logs.empty()) {
    list<LogData*>::iterator i=logs.begin();
    tmp = *i;
    i = logs.erase(i);
  }
  unlock();
  return tmp;
}

int
LogList::size() {
  int ret;
  lock();
  ret = logs.size();
  unlock();
  return ret;
}

//////// class Logging

// constructor
Logging::Logging() {
  pthread_mutex_init(&Mutex_, 0);
  pthread_mutex_init(&Mutex2_, 0);
  present = false;
}

// destructor
Logging::~Logging() {
  LogList *tmp;
  lock();
  for(list<LogList*>::iterator i=tacpp_loglist.begin();
          i!=tacpp_loglist.end() && !tacpp_loglist.empty();) {
    tmp = *i;
    i = tacpp_loglist.erase(i);
    delete tmp;
  }
  tacpp_loglist.clear();
  unlock();
  pthread_mutex_destroy(&Mutex_);
  pthread_mutex_destroy(&Mutex2_);
}

// get LogList for special id
LogList*
Logging::getl(int n) {
  LogList *tmp;
  lock();
  for(list<LogList*>::const_iterator i=tacpp_loglist.begin();i!=tacpp_loglist.end();i++) {
    tmp = *i;
    if(tmp->getNum() == n) {
      unlock();
      return tmp;
    }
  }
  unlock();
  return 0;
}

// set lock
void
Logging::lock() {
  pthread_mutex_lock(&Mutex_);
}

// unset lock
void
Logging::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

// set lock
void
Logging::lock2() {
  pthread_mutex_lock(&Mutex2_);
}

// unset lock
void
Logging::unlock2() {
  pthread_mutex_unlock(&Mutex2_);
}

// add id to output list
int
Logging::add() {
  LogList *tmp;
  lock();
  // find free number for id
  int num = 1;
  for(list<LogList*>::iterator i=tacpp_loglist.begin();i!=tacpp_loglist.end();i++) {
    tmp = *i;
    if(tmp->getNum() == num) {
      num++;
      i = tacpp_loglist.begin();
      continue;
    }
  }
  // here free number for num
  tmp = new LogList(num);
  tacpp_loglist.push_back(tmp);
  unlock();
  return num;
}

// del id from output list
void
Logging::del(int n) {
  LogList *tmp;
  lock();
  for(list<LogList*>::iterator i=tacpp_loglist.begin();i!=tacpp_loglist.end();) {
    tmp = *i;
    if(tmp->getNum() == n) {
      i = tacpp_loglist.erase(i);
      delete tmp;
      continue;
    }
    i++;
  }
  unlock();
}

// out msg to output buffer (copy to all active outputs)
void
Logging::out_msg(int n,string s) {
  if(s.size()==0) return;
  LogList *tmp;
  for(list<LogList*>::const_iterator i=tacpp_loglist.begin();i!=tacpp_loglist.end();i++) {
    tmp = *i;
    tmp->add(n,s);
  }
}

// out message to output buffer for high-level netflow traffic
void
Logging::out_netflow(string s) {
  LogList *tmp;
  lock();
  for(list<LogList*>::const_iterator i=tacpp_loglist.begin();i!=tacpp_loglist.end();i++) {
    tmp = *i;
    tmp->add(LOG_NETFLOWLOG,s);
  }
  unlock();

/*
  char spath[150];
  // create log/debug system
  snprintf(spath,sizeof(spath)-1,"%s/log",::MAINDIR);

  char logn[255]; // netflow log file name
  time_t t = time(0);
  struct tm res;
  struct tm *tt = localtime_r(&t,&res);
  snprintf(logn,sizeof(logn)-1,"%s/nf%04d%02d%02d.log",spath,
      1900+tt->tm_year,tt->tm_mon+1,tt->tm_mday);
  int lognfd = -1;


  char strd[60];
  bzero(strd,sizeof(strd));
  snprintf(strd,sizeof(strd)-1,"[%04d/%02d/%02d-%02d-%02d:%02d:%02d] ",
    1900+tt->tm_year,tt->tm_mon+1,tt->tm_mday,tt->tm_wday+1,
      tt->tm_hour,tt->tm_min,tt->tm_sec);

  string out;

  out = out + strd + s;

  lognfd = open(logn, O_CREAT | O_WRONLY | O_APPEND, 0600);
  if(lognfd >= 0) {
    write(lognfd, out.c_str(), out.size());
    write(lognfd, "\n", 1);
    close(lognfd);
  }
*/
}

// out message to output buffer for high-level logging traffic
void
Logging::out_logging(string s) {
  LogList *tmp;
  lock();
  for(list<LogList*>::const_iterator i=tacpp_loglist.begin();i!=tacpp_loglist.end();i++) {
    tmp = *i;
    tmp->add(LOG_LOGGING,s);
  }
  unlock();
}

// end-user function
void
Logging::log(int num, char *fmt,...) {
  lock();
  char *msg = new char[MAX_LOG_STR_LEN]; // temporary str
  char *fp = 0, *bufp = 0, *charp = 0;
  int len = 0, m = 0, i = 0, n = 0;
  string stro;
  char digits[16];
  va_list ap;

  bzero(msg,sizeof(char)*MAX_LOG_STR_LEN);
  va_start(ap, fmt);
  // ensure that msg is never overwritten
  n = MAX_LOG_STR_LEN;
  fp = fmt;
  len = 0;
  msg[n-1] = '\0';    // limit
  bufp = msg;

  while(*fp) {
    if(*fp != '%') {
      if ((len+1) >= n)
        break;
      *bufp++ = *fp++;
      len++;
      continue;
    }
    // seen a '%'
    fp++;
    switch(*fp) {
      case 'c':
        fp++;
        i = va_arg(ap, uint);
        snprintf(digits,sizeof(digits)-1, "%c", i);
        m = strlen(digits);
        charp = digits;
        break;
      case 's':
        fp++;
        charp = va_arg(ap, char*);
        m = strlen(charp);
        break;
      case 'u':
        fp++;
        i = va_arg(ap, uint);
        snprintf(digits,sizeof(digits)-1, "%u", i);
        m = strlen(digits);
        charp = digits;
        break;
      case 'x':
        fp++;
        i = va_arg(ap, uint);
        snprintf(digits, sizeof(digits)-1, "%x", i);
        m = strlen(digits);
        charp = digits;
        break;
      case 'd':
        fp++;
        i = va_arg(ap, int);
        snprintf(digits, sizeof(digits)-1,"%d", i);
        m = strlen(digits);
        charp = digits;
        break;
    }
    if((len + m + 1) >= n) break;
    memcpy(bufp, charp, m);
    bufp += m;
    len += m;
    continue;
  }
  msg[len] = '\0';
  va_end(ap);
  out_msg(num, msg);
  delete [] msg;
  unlock();
  return;
}

// clear all entries from log/debug
void
Logging::clear() {
  LogList *tmpl;
  LogData *tmpd;
  lock();
  for(list<LogList*>::const_iterator i=tacpp_loglist.begin();i!=tacpp_loglist.end();i++) {
    tmpl = *i;
    while((tmpd = tmpl->getEntry()) != 0) {
        delete tmpd;
    }
  }
  unlock();
}

int
Logging::size() {
  int siz;
  siz = 0;
  lock();
  LogList *tmpl;
  for(list<LogList*>::const_iterator i=tacpp_loglist.begin();i!=tacpp_loglist.end();i++) {
    tmpl = *i;
    siz += tmpl->size();
  }
  unlock();
  return siz;
}

Logging_end
Logging::Logging_end_endlog() {
  Logging_end aaa;
  return aaa;
}

Logging_set
Logging::Logging_set_setlog(int a) {
  Logging_set aaa(a);
  return aaa;
}

//
Logging&
Logging::operator <<(const char *s) {
  sbuf = sbuf + s;
  return *this;
}

Logging&
Logging::operator <<(const int s) {
  sbuf = sbuf + utl::toString(s);
  return *this;
}

Logging&
Logging::operator <<(const unsigned int s) {
  sbuf = sbuf + utl::toString(s);
  return *this;
}

Logging&
Logging::operator <<(const string s) {
  sbuf = sbuf + s;
  return *this;
}

Logging&
Logging::operator <<(ipaddr s) {
  sbuf = sbuf + s.getn();
  return *this;
}

Logging&
Logging::operator <<(const float s) {
  sbuf = sbuf + utl::toString(s);
  return *this;
}

Logging&
Logging::operator <<(const double s) {
  sbuf = sbuf + utl::toString(s);
  return *this;
}

Logging&
Logging::operator <<(const bool s) {
  sbuf = sbuf + (s ? "true" : "false");
  return *this;
}

Logging&
Logging::operator <<(Logging_end aaa) {
  lock();
  if(present)
    out_msg(log2_level,(char*)sbuf.c_str());
  sbuf.erase();
  present = false;
  unlock();
  unlock2();
  return *this;
}

Logging&
Logging::operator <<(Logging_set aaa) {
  log2_level = aaa.level;
  lock2();
  if(::logFileSystem->is_debug_fc(log2_level)) present = true;
  else present = false;
  return *this;
}

///////////////// that's all /////////////////////
