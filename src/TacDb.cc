// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: TacDb.cc
// description: database low-level interface

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"


///////// user_avp methods

user_avp::user_avp() {
  pthread_mutex_init(&mutex_,0);
}

user_avp::~user_avp() {
  lock();
  service_.erase();
  protocol_.erase();
  avp_.erase();
  unlock();
  pthread_mutex_destroy(&mutex_);
}

void
user_avp::lock() {
  pthread_mutex_lock(&mutex_);
}

void
user_avp::unlock() {
  pthread_mutex_unlock(&mutex_);
}

string
user_avp::getService() {
  lock();
  string ret = service_;
  unlock();
  return ret;
}

void
user_avp::setService(const string s) {
  lock();
  service_ = s;
  unlock();
}

bool
user_avp::compareService(const string s) {
  lock();
  bool ret = false;
  if(service_ == s) ret = true;
  unlock();
  return ret;
}

string
user_avp::getProtocol() {
  lock();
  string ret = protocol_;
  unlock();
  return ret;
}

void
user_avp::setProtocol(const string s) {
  lock();
  protocol_ = s;
  unlock();
}

bool
user_avp::compareProtocol(const string s) {
  lock();
  bool ret = false;
  if(protocol_ == s) ret = true;
  unlock();
  return ret;
}

string
user_avp::getAvp() {
  lock();
  string ret = avp_;
  unlock();
  return ret;
}

void
user_avp::setAvp(const string s) {
  lock();
  avp_ = s;
  unlock();
}

void
user_avp::operator =(user_avp a) {
  lock();
  a.lock();
  service_ = a.service_;
  protocol_ = a.protocol_;
  avp_ = a.avp_;
  a.unlock();
  unlock();
}

bool
user_avp::operator ==(user_avp a) {
  lock();
  a.lock();
  bool ret = false;
  if(service_==a.service_ && protocol_==a.protocol_ &&
    avp_==a.avp_) ret = true;
  a.unlock();
  unlock();
  return ret;
}

bool
user_avp::operator !=(user_avp a) {
  lock();
  a.lock();
  bool ret = false;
  if(service_!=a.service_ || protocol_!=a.protocol_ || avp_!=a.avp_) ret = true;
  a.unlock();
  unlock();
  return ret;
}


///////// cmd_avp methods

cmd_avp::cmd_avp() {
  pthread_mutex_init(&mutex_,0);
}

cmd_avp::~cmd_avp() {
  pthread_mutex_destroy(&mutex_);
}

void
cmd_avp::lock() {
  pthread_mutex_lock(&mutex_);
}

void
cmd_avp::unlock() {
  pthread_mutex_unlock(&mutex_);
}

string
cmd_avp::getCmdperm() {
  lock();
  string ret = cmdperm_;
  unlock();
  return ret;
}

void
cmd_avp::setCmdperm(const string s) {
  lock();
  cmdperm_ = s;
  unlock();
}

string
cmd_avp::getCmddeny() {
  lock();
  string ret = cmddeny_;
  unlock();
  return ret;
}

void
cmd_avp::setCmddeny(const string s) {
  lock();
  cmddeny_ = s;
  unlock();
}

string
cmd_avp::getArgperm() {
  lock();
  string ret = argperm_;
  unlock();
  return ret;
}

void
cmd_avp::setArgperm(const string s) {
  lock();
  argperm_ = s;
  unlock();
}

string
cmd_avp::getArgdeny() {
  lock();
  string ret = argdeny_;
  unlock();
  return ret;
}

void
cmd_avp::setArgdeny(const string s) {
  lock();
  argdeny_ = s;
  unlock();
}

void
cmd_avp::operator =(cmd_avp a) {
  lock();
  a.lock();
  cmdperm_ = a.cmdperm_;
  cmddeny_ = a.cmddeny_;
  argperm_ = a.argperm_;
  argdeny_ = a.argdeny_;
  a.unlock();
  unlock();
}

bool
cmd_avp::operator ==(cmd_avp a) {
  lock();
  a.lock();
  bool ret = false;
  if(cmdperm_==a.cmdperm_ && cmddeny_==a.cmddeny_ && argperm_==a.argperm_ && argdeny_==a.argdeny_)
    ret = true;
  a.unlock();
  unlock();
  return ret;
}


///////// db_user_acc methods

db_user_acc::db_user_acc() {
  pthread_mutex_init(&mutex_,0);
}

db_user_acc::~db_user_acc() {
  pthread_mutex_destroy(&mutex_);
}

void
db_user_acc::lock() {
  pthread_mutex_lock(&mutex_);
}

void
db_user_acc::unlock() {
  pthread_mutex_unlock(&mutex_);
}


///////// db_user_acc

string
db_user_acc::getPhone() {
  lock();
  string ret = phone_;
  unlock();
  return ret;
}

void
db_user_acc::setPhone(const string s) {
  lock();
  phone_ = s;
  unlock();
}

string
db_user_acc::getNas() {
  lock();
  string res = nas_;
  unlock();
  return res;
}

void
db_user_acc::setNas(const string s) {
  lock();
  nas_ = s;
  unlock();
}

string
db_user_acc::getPort() {
  lock();
  string res = port_;
  unlock();
  return res;
}

void
db_user_acc::setPort(const string s) {
  lock();
  port_ = s;
  unlock();
}

string
db_user_acc::getTime() {
  lock();
  string ret = time_;
  unlock();
  return ret;
}

void
db_user_acc::setTime(const string s) {
  lock();
  time_ = s;
  unlock();
}

void
db_user_acc::operator =(db_user_acc a) {
  lock();
  a.lock();
  phone_ = a.phone_;
  nas_ = a.nas_;
  port_ = a.port_;
  time_ = a.time_;
  a.unlock();
  unlock();
}

bool
db_user_acc::operator ==(db_user_acc a) {
  lock();
  a.lock();
  bool ret = false;
  if(phone_==a.phone_ && nas_==a.nas_ && port_==a.port_ &&
      time_==a.time_) ret = true;
  a.unlock();
  unlock();
  return ret;
}


///////// db_bilres methods

db_bilres::db_bilres() {
  pthread_mutex_init(&mutex_,0);
}

db_bilres::~db_bilres() {
  pthread_mutex_destroy(&mutex_);
}

void
db_bilres::lock() {
  pthread_mutex_lock(&mutex_);
}

void
db_bilres::unlock() {
  pthread_mutex_unlock(&mutex_);
}

string
db_bilres::getAttribute() {
  lock();
  string ret = attribute_;
  unlock();
  return ret;
}

void
db_bilres::setAttribute(const string s) {
  lock();
  attribute_ = s;
  unlock();
}

string
db_bilres::getValue() {
  lock();
  string ret = value_;
  unlock();
  return ret;
}

void
db_bilres::setValue(const string s) {
  lock();
  value_ = s;
  unlock();
}

void
db_bilres::operator =(db_bilres a) {
  lock();
  a.lock();
  attribute_ = a.attribute_;
  value_ = a.value_;
  a.unlock();
  unlock();
}

bool
db_bilres::operator !=(db_bilres a) {
  lock();
  a.lock();
  bool ret = false;
  if(attribute_!=a.attribute_ || value_!=a.value_) ret = true;
  a.unlock();
  unlock();
  return ret;
}

bool
db_bilres::operator ==(db_bilres a) {
  lock();
  a.lock();
  bool ret = false;
  if(attribute_==a.attribute_ && value_==a.value_) ret = true;
  a.unlock();
  unlock();
  return ret;
}


///////// db_user methods

db_user::db_user() {
  pthread_mutex_init(&mutex_,0);
  lock();
  open_ = 0;
  close_ = 0;
  result_ = false;
  connect_ = false;
  maxsess_ = 0;
  bilmodid_ = 0;
  unlock();
}

db_user::~db_user() {
  lock();
  for(list<user_avp>::iterator i=avp_.begin();i!=avp_.end() && !avp_.empty();) {
    i = avp_.erase(i);
  }
  avp_.clear();
  for(list<cmd_avp>::iterator i=cmd_.begin();i!=cmd_.end() && !cmd_.empty();) {
    i = cmd_.erase(i);
  }
  cmd_.clear();
  for(list<db_user_acc>::iterator i=acc_.begin();i!=acc_.end() && !acc_.empty();) {
    i = acc_.erase(i);
  }
  acc_.clear();
  for(list<db_bilres>::iterator i=bilres_.begin();i!=bilres_.end() && !bilres_.empty();) {
    i = bilres_.erase(i);
  }
  bilres_.clear();
  unlock();
  pthread_mutex_destroy(&mutex_);
}

void
db_user::lock() {
  pthread_mutex_lock(&mutex_);
}

void
db_user::unlock() {
  pthread_mutex_unlock(&mutex_);
}

string
db_user::getUsername() {
  lock();
  string ret = username_;
  unlock();
  return ret;
}

void
db_user::setUsername(const string s) {
  lock();
  username_ = s;
  unlock();
}

string
db_user::getAuthsrc() {
  lock();
  string ret = authsrc_;
  unlock();
  return ret;
}

void
db_user::setAuthsrc(const string s) {
  lock();
  authsrc_ = s;
  unlock();
}

string
db_user::getPassword() {
  lock();
  string ret = password_;
  unlock();
  return ret;
}

void
db_user::setPassword(const string s) {
  lock();
  password_ = s;
  unlock();
}

string
db_user::getEnable() {
  lock();
  string ret = enable_;
  unlock();
  return ret;
}

void
db_user::setEnable(const string s) {
  lock();
  enable_ = s;
  unlock();
}

time_t
db_user::getOpen() {
  lock();
  time_t ret = open_;
  unlock();
  return ret;
}

void
db_user::setOpen(time_t s) {
  lock();
  open_ = s;
  unlock();
}

time_t
db_user::getClose() {
  lock();
  time_t ret = close_;
  unlock();
  return ret;
}

void
db_user::setClose(time_t s) {
  lock();
  close_ = s;
  unlock();
}

string
db_user::getAvpid() {
  lock();
  string ret = avpid_;
  unlock();
  return ret;
}

void
db_user::setAvpid(const string s) {
  lock();
  avpid_ = s;
  unlock();
}

string
db_user::getAccid() {
  lock();
  string ret = accid_;
  unlock();
  return ret;
}

void
db_user::setAccid(const string s) {
  lock();
  accid_ = s;
  unlock();
}

string
db_user::getAddid() {
  lock();
  string ret = addid_;
  unlock();
  return ret;
}

void
db_user::setAddid(const string s) {
  lock();
  addid_ = s;
  unlock();
}

string
db_user::getBilid() {
  lock();
  string ret = bilid_;
  unlock();
  return ret;
}

void
db_user::setBilid(const string s) {
  lock();
  bilid_ = s;
  unlock();
}

int
db_user::getBilmodid() {
  lock();
  int ret = bilmodid_;
  unlock();
  return ret;
}

void
db_user::setBilmodid(int s) {
  lock();
  bilmodid_ = s;
  unlock();
}

void
db_user::avp_add(user_avp a) {
  lock();
  avp_.push_back(a);
  unlock();
}

int
db_user::avp_size() {
  lock();
  int ret = avp_.size();
  unlock();
  return ret;
}

user_avp
db_user::avp_get(int s) {
  lock();
  int count = 0;
  user_avp ret;
  for(list<user_avp>::const_iterator i=avp_.begin();i!=avp_.end();i++,count++) {
    if(count == s) {
      ret = *i;
      break;
    }
  }
  unlock();
  return ret;
}

void
db_user::avp_set(int s,user_avp a) {
  lock();
  int count = 0;
  user_avp ret;
  for(list<user_avp>::iterator i=avp_.begin();i!=avp_.end();i++,count++) {
    if(count == s) {
      i = avp_.erase(i);
      break;
    }
  }
  avp_.push_back(a);
  unlock();
}

void
db_user::cmd_add(cmd_avp a) {
  lock();
  cmd_.push_back(a);
  unlock();
}

int
db_user::cmd_size() {
  lock();
  int ret = cmd_.size();
  unlock();
  return ret;
}

cmd_avp
db_user::cmd_get(int s) {
  lock();
  int count = 0;
  cmd_avp ret;
  for(list<cmd_avp>::const_iterator i=cmd_.begin();i!=cmd_.end();i++,count++) {
    if(count == s) {
      ret = *i;
      break;
    }
  }
  unlock();
  return ret;
}

void
db_user::acc_add(db_user_acc a) {
  lock();
  acc_.push_back(a);
  unlock();
}

int
db_user::acc_size() {
  lock();
  int ret = acc_.size();
  unlock();
  return ret;
}

db_user_acc
db_user::acc_get(int s) {
  lock();
  int count = 0;
  db_user_acc ret;
  for(list<db_user_acc>::const_iterator i=acc_.begin();i!=acc_.end();i++,count++) {
    if(count == s) {
      ret = *i;
      break;
    }
  }
  unlock();
  return ret;
}

void
db_user::bilres_add(db_bilres a) {
  lock();
  bilres_.push_back(a);
  unlock();
}

int
db_user::bilres_size() {
  lock();
  int ret = bilres_.size();
  unlock();
  return ret;
}

db_bilres
db_user::bilres_get(int s) {
  lock();
  int count = 0;
  db_bilres ret;
  for(list<db_bilres>::const_iterator i=bilres_.begin();i!=bilres_.end();i++,count++) {
    if(count == s) {
      ret = *i;
      break;
    }
  }
  unlock();
  return ret;
}

bool
db_user::getResult() {
  lock();
  bool ret = result_;
  unlock();
  return ret;
}

void
db_user::setResult(bool s) {
  lock();
  result_ = s;
  unlock();
}

bool
db_user::getConnect() {
  lock();
  bool ret = connect_;
  unlock();
  return ret;
}

void
db_user::setConnect(bool s) {
  lock();
  connect_ = s;
  unlock();
}

string
db_user::getResultMsg() {
  lock();
  string ret = result_msg_;
  unlock();
  return ret;
}

void
db_user::setResultMsg(const string s) {
  lock();
  result_msg_ = s;
  unlock();
}

int
db_user::getMaxsess() {
  lock();
  int ret = maxsess_;
  unlock();
  return ret;
}

void
db_user::setMaxsess(int s) {
  lock();
  maxsess_ = s;
  unlock();
}

void
db_user::operator =(db_user a) {
  lock();
  a.lock();
  username_ = a.username_;
  authsrc_ = a.authsrc_;
  password_ = a.password_;
  enable_ = a.enable_;
  open_ = a.open_;
  close_ = a.close_;
  avpid_ = a.avpid_;
  accid_ = a.accid_;
  addid_ = a.addid_;
  bilid_ = a.bilid_;
  bilmodid_ = a.bilmodid_;
  avp_ = a.avp_;
  cmd_ = a.cmd_;
  acc_ = a.acc_;
  bilres_ = a.bilres_;
  result_ = a.result_;
  connect_ = a.connect_;
  result_msg_ = a.result_msg_;
  maxsess_ = a.maxsess_;
  a.unlock();
  unlock();
}

bool
db_user::operator ==(db_user a) {
  lock();
  a.lock();
  bool ret = false;
  if(username_ == a.username_) ret = true;
  a.unlock();
  unlock();
  return ret;
}

bool
db_user::operator !=(db_user a) {
  lock();
  a.lock();
  bool ret = false;
  if(username_ != a.username_) ret = true;
  a.unlock();
  unlock();
  return ret;
}


/////////////////////

const string TDB_DEV =  "device";
const string TDB_TYPE =       "devtype";
const string TDB_DESCR =      "description";
const string TDB_TACKEY =     "tacacskey";
const string TDB_COMMUN =     "community";
const string TDB_SNMPPOL =    "snmppolling";
const string TDB_ICMPPOL =    "icmppolling";
const string TDB_INTFCOUNT =  "inttrfcount";
const string TDB_POLLDELAY =  "polldelay";
const string TDB_DEFAUTHOR =  "defauthorization";
const string TDB_SNMPMOD =    "snmpmodule";
const string TDB_SHUT =       "shutdowned";
const string TDB_LOGINS =     "loginreq";
const string TDB_PASSWDS =    "passwdreq";

const string TDB_USR =        "username";
const string TDB_PWD =        "password";
const string TDB_OPEN =       "dopen";
const string TDB_CLOSE =      "dclose";
const string TDB_AVPID =      "avpid";
const string TDB_ACCID =      "accid";
const string TDB_BILID =      "bilid";
const string TDB_ADDID =      "addid";
const string TDB_MAXS =       "maxsess";

const string TDB_UID =        "unid";

const string TDB_AUTHSRC =    "authsrc";
const string TDB_ENABLE =     "enable";

const string TDB_PHONE =      "phone";
const string TDB_NAS =        "nas";
const string TDB_IP =         "ipaddres";
const string TDB_PORT =       "port";
const string TDB_TIME =       "time";

const string TDB_SERVICE =    "service";
const string TDB_PROTOCOL =   "protocol";
const string TDB_AVP =        "avp";

const string TDB_CMDPERM =    "cmdperm";
const string TDB_CMDDENY =    "cmddeny";
const string TDB_ARGPERM =    "argperm";
const string TDB_ARGDENY =    "argdeny";

const string TDB_BILMODID =   "bilmodid";
const string TDB_ATTRIBUTE =  "attribute";
const string TDB_VALUE =      "value";

const string TDB_DCHECK =     "dcheck";

const string TDB_USERIP	= "usernetip";
const string TDB_COMMENT = "comment";

//// DbList

DbList::DbList() {
  pthread_mutex_init(&mutex_,0);

  DbTable *dbtmp;

  ////// this is version 1 database tables /////

  /*
  // network devices table
  add(T_DEV,T_DEV_ID,"network devices");
  get(T_DEV)->add(TDB_DEV,"network device name or address","",T_STR_TYPE,DB_NAS_LEN,T_UNIQUE);
  get(T_DEV)->add(TDB_TYPE,"network device type","",T_STR_TYPE,DEV_TYPE_LEN);
  get(T_DEV)->add(TDB_DESCR,"network device description","",T_STR_TYPE,DESCRIPTION_LEN);
  get(T_DEV)->add(TDB_TACKEY,"tacacs+ key","",T_STR_TYPE,TACACS_KEY_LEN);
  get(T_DEV)->add(TDB_COMMUN,"snmp community","public",T_STR_TYPE,SNMP_COMMUNITY_LEN);
  get(T_DEV)->add(TDB_SNMPPOL,"permit snmp polling","false",T_BOOL_TYPE);
  get(T_DEV)->add(TDB_ICMPPOL,"permit icmp polling","false",T_BOOL_TYPE);
  get(T_DEV)->add(TDB_INTFCOUNT,"permit interfaces traffic counter","false",T_BOOL_TYPE);
  get(T_DEV)->add(TDB_POLLDELAY,"polling delay","60",T_INT_TYPE);
  get(T_DEV)->add(TDB_DEFAUTHOR,"permit default authorization","false",T_BOOL_TYPE);
  get(T_DEV)->add(TDB_SNMPMOD,"snmp module","none.so",T_STR_TYPE,SNMP_MODULE_LEN);
  get(T_DEV)->add(TDB_SHUT,"out of operation","true",T_BOOL_TYPE);
  get(T_DEV)->add(TDB_LOGINS,"login req string","login:",T_STR_TYPE,LOGINSTRING_LEN);
  get(T_DEV)->add(TDB_PASSWDS,"password req string","Password:",T_STR_TYPE,PWDSTRING_LEN);
  */

  // users table
  add(T_USR,T_USR_ID,"user information");
  dbtmp = get(T_USR);
  dbtmp->add("dupdate","last update time","",T_DATE_TYPE);
  dbtmp->add(TDB_USR,"user name","",T_STR_TYPE,DB_USERNAME_LEN,T_UNIQUE);
  dbtmp->add(TDB_PWD,"password","",T_STR_TYPE,DB_PASSWORD_LEN);
  dbtmp->add(TDB_OPEN,"open date","0",T_DATE_TYPE);
  dbtmp->add(TDB_CLOSE,"close date","0",T_DATE_TYPE);
  dbtmp->add(TDB_AVPID,"authorization group","",T_STR_TYPE,DB_ID_LEN);
  dbtmp->add(TDB_ACCID,"access group","",T_STR_TYPE,DB_ID_LEN);
  dbtmp->add(TDB_BILID,"resource group","",T_STR_TYPE,DB_ID_LEN);
  dbtmp->add(TDB_ADDID,"additional data group","",T_STR_TYPE,DB_ID_LEN);
  dbtmp->add(TDB_MAXS,"max sessions","1",T_INT_TYPE);

  // additional user data group
  add(T_ADD,T_ADD_ID,"additional user data");
  dbtmp = get(T_ADD);
  dbtmp->add(TDB_ADDID,"additional data group","",T_STR_TYPE,DB_ID_LEN,T_UNIQUE);
  dbtmp->add(TDB_AUTHSRC,"authentication source","db",T_STR_TYPE,DB_AUTHSRC_LEN);
  dbtmp->add(TDB_ENABLE,"enable access password","",T_STR_TYPE,DB_PASSWORD_LEN);

  // user access rights group
  add(T_ACC,T_ACC_ID,"user access rights");
  dbtmp = get(T_ACC);
  dbtmp->add(TDB_UID,"unique id","",T_INT_TYPE,T_UNIQUE);
  dbtmp->add(TDB_ACCID,"access group name","",T_STR_TYPE,DB_ID_LEN);
  dbtmp->add(TDB_PHONE,"access by phone","",T_STR_TYPE,FROM_PHONE_LEN);
  dbtmp->add(TDB_NAS,"from specifyed NAS","",T_STR_TYPE,FROM_NAS_LEN);
  dbtmp->add(TDB_PORT,"from specifyed port","",T_STR_TYPE,FROM_PORT_LEN);
  dbtmp->add(TDB_TIME,"restricted access time","",T_STR_TYPE,FROM_TIME_LEN);
//  dbtmp->add(TDB_AD,"access domain","",T_STR_TYPE,ACC_DOM_LEN);

  // access domains descriptors
//  add(T_AD,T_AD_ID,"access domain");
//  dbtmp = get(T_AD);
//  dbtmp->add(TDB_UID,"unique id","",T_INT_TYPE,T_UNIQUE); // for web system
//  dbtmp->add(TDB_AD,"access domain id","",T_STR_TYPE,);
//  dbtmp->add(TDB_NAS,"device ip access string","",T_STR_TYPE,FROM_NAS_LEN);

  // authorization group
  add(T_AVP,T_AVP_ID,"authorization");
  dbtmp = get(T_AVP);
  dbtmp->add(TDB_UID,"unique id","",T_INT_TYPE,T_UNIQUE);
  dbtmp->add(TDB_AVPID,"author group id","",T_STR_TYPE,DB_ID_LEN);
  dbtmp->add(TDB_SERVICE,"service","",T_STR_TYPE,AVP_SERVICE_LEN);
  dbtmp->add(TDB_PROTOCOL,"protocol","",T_STR_TYPE,AVP_PROTOCOL_LEN);
  dbtmp->add(TDB_AVP,"attribute=value pair","",T_STR_TYPE,AVP_VALUE_LEN);

  // command authorization group
  add(T_CMD,T_CMD_ID,"per-command authorization");
  dbtmp = get(T_CMD);
  dbtmp->add(TDB_UID,"unique id","",T_INT_TYPE,T_UNIQUE);
  dbtmp->add(TDB_AVPID,"author group id","",T_STR_TYPE,DB_ID_LEN);
  dbtmp->add(TDB_CMDPERM,"permitted commands","",T_STR_TYPE,CMD_AVP_LEN);
  dbtmp->add(TDB_CMDDENY,"denied commands","",T_STR_TYPE,CMD_AVP_LEN);
  dbtmp->add(TDB_ARGPERM,"permitted arguments","",T_STR_TYPE,CMD_AVP_LEN);
  dbtmp->add(TDB_ARGDENY,"denied arguments","",T_STR_TYPE,CMD_AVP_LEN);

  // resource group
  add(T_BIL,T_BIL_ID,"resource group");
  dbtmp = get(T_BIL);
  dbtmp->add(TDB_BILID,"resource group id","",T_STR_TYPE,DB_ID_LEN,T_UNIQUE);
  dbtmp->add(TDB_BILMODID,"resource module number","",T_INT_TYPE);

  // resource group resources
  add(T_BILRES,T_BILRES_ID,"resource system resources");
  dbtmp = get(T_BILRES);
  dbtmp->add(TDB_BILID,"resource group id","",T_STR_TYPE,DB_ID_LEN,T_INDEX);
  dbtmp->add(TDB_ATTRIBUTE,"attribute name","",T_STR_TYPE,32);
  dbtmp->add(TDB_VALUE,"attribute value","",T_STR_TYPE,32);

  // users ip for netflow accounting
  add(T_NFUSR,T_NFUSR_ID,"users ip");
  dbtmp = get(T_NFUSR);
  dbtmp->add(TDB_USERIP,"user net ip","",T_IP_TYPE,T_INDEX);
  dbtmp->add(TDB_USR,"user name or id","",T_STR_TYPE,32);
  dbtmp->add(TDB_COMMENT,"comment","",T_STR_TYPE,48);
}

void
DbList::lock() {
  pthread_mutex_lock(&mutex_);
}

void
DbList::unlock() {
  pthread_mutex_unlock(&mutex_);
}

void
DbList::add(const string tname,const string tmask,const string tdescr) {
  DbTable *tmp;
  tmp = 0;
  lock();
  for(list<DbTable*>::const_iterator i=dt_.begin();i!=dt_.end();i++) {
    tmp = *i;
    if(tname == tmp->getName()) {
      unlock();
      return;
    }
  }
//  unlock();
  tmp = new DbTable((char*)tname.c_str(),(char*)tmask.c_str(),(char*)tdescr.c_str());
//  lock();
  dt_.push_back(tmp);
  unlock();
}

DbTable*
DbList::get(const string tname) {
  DbTable *tmp;
  tmp = 0;
  lock();
  for(list<DbTable*>::const_iterator i=dt_.begin();i!=dt_.end(); i++) {
    tmp = *i;
    if(tname == tmp->getName()) {
      unlock();
      return tmp;
    }
  }
  unlock();
  return 0;
}

DbTable*
DbList::get(int n) {
  DbTable *tmp;
  tmp = 0;
  lock();
  int dtsize = dt_.size();
  unlock();
  if(dtsize > 0 && dtsize > n) {
    int count=0;
    list<DbTable*>::const_iterator i;
    lock();
    for(i=dt_.begin();i!=dt_.end(); i++,count++) {
      tmp = *i;
      if(count == n) {
        break;
      }
    }
    unlock();
  }
  return tmp;
}

DbList::~DbList() {
  lock();
  DbTable *tmp;
  tmp = 0;
  for(list<DbTable*>::iterator i=dt_.begin();i!=dt_.end() && !dt_.empty();) {
    tmp = *i;
    i = dt_.erase(i);
    delete tmp;
  }
  unlock();
  pthread_mutex_destroy(&mutex_);
}

int
DbList::size() {
  lock();
  int ret = dt_.size();
  unlock();
  return ret;
}


///////////////// DbModule ///////////////////////

// constructor for DbSql class
//  loads functions from .so module
DbModule::DbModule(const char *mod,const char *key,const char *loc,int p,const char *uname, const char *pwd,const char *db) {
  pthread_mutex_init(&mutex_,0);
  lock();
  dbname_ = db;
  location_.fill(loc);
  dbport_ = p;
  cryptokey_ = key;
  username_ = uname;
  password_ = pwd;
  module_ = mod;
  shutdown_ = true;
  unlock();
  startlog(LOG_EVENT)<<"create Database Module "<<getdbid()<<endlog;
}

DbModule::~DbModule() {
  pthread_mutex_destroy(&mutex_);
}

void
DbModule::lock() {
  pthread_mutex_lock(&mutex_);
}
void
DbModule::unlock() {
  pthread_mutex_unlock(&mutex_);
}

void
DbModule::setdbid(const char *d,const char *l,int p) {
  lock();
  dbname_ = d;
  location_.fill(l);
  dbport_ = p;
  unlock();
}

string
DbModule::getdbid() {
  lock();
  string str = dbname_ + "@" + location_.getn() + ":" + utl::toString(dbport_);
  unlock();
  return str;
}

void
DbModule::setLocation(ipaddr d) {
  lock();
  location_ = d;
  unlock();
}

ipaddr
DbModule::getLocation() {
  lock();
  ipaddr ret = location_;
  unlock();
  return ret;
}

void
DbModule::setDbname(string d) {
  lock();
  dbname_ = d;
  unlock();
}

string
DbModule::getDbname() {
  lock();
  string ret = dbname_;
  unlock();
  return ret;
}

void
DbModule::setDbport(unsigned int d) {
  lock();
  dbport_ = d;
  unlock();
}

unsigned int
DbModule::getDbport() {
  lock();
  unsigned int ret = dbport_;
  unlock();
  return ret;
}

void
DbModule::setCryptokey(string d) {
  lock();
  cryptokey_ = d;
  unlock();
}

string
DbModule::getCryptokey() {
  lock();
  string ret = cryptokey_;
  unlock();
  return ret;
}

void
DbModule::setUsername(const string d) {
  lock();
  username_ = d;
  unlock();
}

string
DbModule::getUsername() {
  lock();
  string ret = username_;
  unlock();
  return ret;
}

void
DbModule::setPassword(const string d) {
  lock();
  password_ = d;
  unlock();
}

string
DbModule::getPassword() {
  lock();
  string ret = password_;
  unlock();
  return ret;
}

void
DbModule::setModule(const string d) {
  lock();
  module_ = d;
  unlock();
}

string
DbModule::getModule() {
  lock();
  string ret = module_;
  unlock();
  return ret;
}

void
DbModule::setShutdown(bool d) {
  lock();
  shutdown_ = d;
  unlock();
}

bool
DbModule::getShutdown() {
  lock();
  bool ret = shutdown_;
  unlock();
  return ret;
}

// data crypt - the main thing - change some symbols
// which can be created during encryption, for SQL server
string
DbModule::encrypt(char *str,char *varkey) {
  if(cryptokey_ == "none") {
    char ret[64];
    bzero(ret,sizeof(ret));
    for(int i=0,j=0; i<(int)strlen(str) && i<32 && j<64; i++,j++) {
      ret[j] = str[i];
      // symbol '\' should be changed to '\\'
      if(ret[j] == '\\') ret[++j] = '\\';
      // symbol ''' should be changed to '\''
      if(ret[j] == '\'') { ret[j] = '\\'; ret[++j] = '\''; }
      // symbol '"' should be changed to '\"'
      if(ret[j] == '\"') { ret[j] = '\\'; ret[++j] = '\"'; }
    }
    string out = ret;
    return out;
  }
  char s[32];
  bzero(s,sizeof(s));
  for(int i=0; i < (int)strlen(str) && i < 32; i++) s[i] = str[i];
  md5_xor((unsigned char*)s,(unsigned char*)varkey);
  // now we have in str crypted string.... Good,
  char ret[64];
  bzero(ret,sizeof(ret));
  for(int i=0; i < (int)strlen(s) && i < 64; i++) ret[i] = s[i];
  // move to base64
  string sout;
  sout = utl::base64Encode(ret);
//cerr<<"encrypted password ="<<sout<<"end"<<endl;
  return sout;
}

// data decrypt
string
DbModule::decrypt(char *str,char *varkey) {
  if(cryptokey_ == "none") {
    string out = str;
    return out;
  }
  char s[128];
  bzero(s,sizeof(s));
  for(int i=0; i < (int)strlen(str) && i < 128; i++) s[i] = str[i];

  char r[64];
  bzero(r,sizeof(r));
  string sss = utl::base64Decode(s);
  snprintf(r,sizeof(r)-1,"%s",sss.c_str());

  md5_xor((unsigned char*)r, (unsigned char*)varkey);
  char ret[32];
  bzero(ret,sizeof(ret));
  for(int i=0; i < (int)strlen(r) && i < 32; i++) ret[i] = r[i];

  string sout = ret;
  return sout;
}

// connect
bool
DbModule::db_open(int *cid) {
  char errmsg[DB_MODULE_ERRLEN+1];
  bool ret;
  ret = false;
  bzero(errmsg,sizeof(errmsg));
  *cid = dl_open((char*)location_.get().c_str(),dbport_,
      (char*)dbname_.c_str(),(char*)username_.c_str(),
        (char*)password_.c_str(),errmsg);
  if(strlen(errmsg)>0) startlog(LOG_ERROR)<<"DB connect: "<<errmsg<<endlog;
  if(*cid==0 || strlen(errmsg)>0) ret = false;
  else ret = true;
  return ret;
}

// disconnect
bool
DbModule::db_close(int cid) {
  bool ret = dl_close(cid);
  return ret;
}

// query
bool
DbModule::db_query(int cid,DbTable *q) {
  char errmsg[DB_MODULE_ERRLEN+1];
  bzero(errmsg,sizeof(errmsg));
  bool ret;
  ret = false;
  ret = dl_query(cid,q,errmsg);
  if(strlen(errmsg)>0) startlog(LOG_ERROR)<<"DB query: "<<errmsg<<endlog;
  return ret;
}

// function
string
DbModule::db_function(int cid,char *f) {
  char errmsg[DB_MODULE_ERRLEN+1];
  bzero(errmsg,sizeof(errmsg));
  string ostr;
  char *str;
  str = dl_function(cid,f,errmsg);
  if(strlen(errmsg) > 0) {
    startlog(LOG_ERROR)<<"DB function: "<<errmsg<<endlog;
//std::cerr<<"Error from DBfunc: "<<errmsg<<std::endl;
  }
  if(str && strlen(str)>0) {
    ostr = str;
  }
  if(str) delete [] str;
  return ostr;
}

// get data
bool
DbModule::db_get(int cid,DbTable *q,int a) {
  char errmsg[DB_MODULE_ERRLEN+1];
  bzero(errmsg,sizeof(errmsg));
  bool ret;
  ret = false;
  ret = dl_get(cid,q,a,errmsg);
  if(strlen(errmsg)>0) startlog(LOG_ERROR)<<"DB get: "<<errmsg<<endlog;
  return ret;
}

// create
bool
DbModule::db_create(int cid,DbTable *q,char *errmsg) {
  if(!errmsg) return false;
  bzero(errmsg,sizeof(char)*DB_MODULE_ERRLEN);
  bool ret;
  ret = false;
  ret = dl_create(cid,q,errmsg);
  if(errmsg && strlen(errmsg)>0) startlog(LOG_ERROR)<<"DB create: "<<errmsg<<endlog;
  return ret;
}

// create index
bool
DbModule::db_create_idx(int cid,DbTable *q,char *errmsg) {
  if(!errmsg) return false;
  bzero(errmsg,sizeof(char)*DB_MODULE_ERRLEN);
  bool ret;
  ret = false;
  ret = dl_create_idx(cid,q,errmsg);
  if(errmsg && strlen(errmsg)>0) startlog(LOG_ERROR)<<"DB create_idx: "<<errmsg<<endlog;
  return ret;
}

// add
bool
DbModule::db_add(int cid,DbTable *q) {
  char errmsg[DB_MODULE_ERRLEN+1];
  bzero(errmsg,sizeof(errmsg));
  bool ret;
  ret = false;
  ret = dl_add(cid,q,errmsg);
  if(strlen(errmsg) > 0) startlog(LOG_ERROR)<<"DB add: "<<errmsg<<endlog;
  return ret;
}

// update
bool
DbModule::db_update(int cid,DbTable *q) {
  char errmsg[DB_MODULE_ERRLEN+1];
  bzero(errmsg,sizeof(errmsg));
  bool ret;
  ret = false;
  ret = dl_update(cid,q,errmsg);
  if(strlen(errmsg) > 0) startlog(LOG_ERROR)<<"DB update: "<<errmsg<<endlog;
  return ret;
}

// delete
bool
DbModule::db_del(int cid,DbTable *q) {
  char errmsg[DB_MODULE_ERRLEN+1];
  bzero(errmsg,sizeof(errmsg));
  bool ret;
  ret = false;
  ret = dl_del(cid,q,errmsg);
  if(strlen(errmsg) > 0) startlog(LOG_ERROR)<<"DB del: "<<errmsg<<endlog;
  return ret;
}

// create_md5_hash(): create an md5 hash of the "the user's key",
// Write resulting hash into the array pointed to by "hash".
// The caller must allocate sufficient space for the resulting hash
// (which is 16 bytes long). The resulting hash can safely be used as
// input to another call to create_md5_hash, as its contents are copied
// before the new hash is generated.
void
DbModule::create_md5_hash(unsigned char* hash,unsigned char *varkey) {
  unsigned char *md_stream, *mdp;
  int md_len = 0;
  MD5_CTX mdcontext;
  char cryptokey[128];
  snprintf(cryptokey,sizeof(cryptokey)-1,"%s",cryptokey_.c_str());

  md_len = strlen(cryptokey)+strlen((char*)varkey);
  mdp = md_stream = new unsigned char[md_len];
  mdp = md_stream;
  bcopy(cryptokey, mdp, strlen(cryptokey));
  mdp += strlen(cryptokey);
  bcopy(varkey, mdp, strlen((char*)varkey));
  mdp += strlen((char*)varkey);

  MD5_Init(&mdcontext);
  MD5_Update(&mdcontext, md_stream, md_len);
  MD5_Final(hash, &mdcontext);

  delete [] md_stream;
  md_stream = 0;
  return;
}

// Overwrite input data with en/decrypted version by generating an MD5 hash
// and xor'ing data with it.
// When more than 16 bytes of hash is needed, the MD5 hash is performed
// again with the same values as before, but with the previous hash value
// appended to the MD5 input stream.
void
DbModule::md5_xor(unsigned char* data,unsigned char *varkey) {	
  char cryptokey[32];
  snprintf(cryptokey,sizeof(cryptokey)-1,"%s",cryptokey_.c_str());
  if(!utl::scomp(cryptokey,"none")) {
    int data_len = strlen((char*)data);
    unsigned char hash[MD5_LEN];       // the md5 hash
    bzero(hash, MD5_LEN);
    create_md5_hash(hash,varkey);
    for(int i = 0; i < data_len; i += 16) {
      for (int j = 0; j < 16 && (i+j)<data_len; j++) {
        data[i + j] ^= hash[j];
      }
    }
  }
  return;
}



//// TacDb

// constructor
TacDb::TacDb(const char *mod,const char *k,const char *l,int port,const char *u,const char *p,const char *d) :
    DbModule(mod,k,l,port,u,p,d) {
//std::cerr<<"XXXXXXXXXXXXXXXXX"<<std::endl;
  dl = 0;
  pthread_mutex_init(&Mutex_,0);
  char hostname[64];
  gethostname((char*)hostname,sizeof(hostname));
  our_ip_ = hostname;
}

TacDb::~TacDb() {
  lock();
  if(dl) {
    dl_destroy();
    delete dl;
  }
  unlock();
  pthread_mutex_destroy(&Mutex_);
}

void
TacDb::lock() {
  pthread_mutex_lock(&Mutex_);
}

void
TacDb::unlock() {
  pthread_mutex_unlock(&Mutex_);
}

int
TacDb::size() {
  DbList *dtl = new DbList;
  int ret = dtl->size();
  delete dtl;
  return ret;
}

// load module
string
TacDb::load() {
  lock();
  if(dl) {
    dl_destroy();
    delete dl;
  }
  dl = new DLoad;
  string str;
  str = dl->dbload((char*)getModule().c_str());
  if(str == "(null)") str.erase();
  if(str.size() > 0) return str;// if error
  dl_init = (void (*)())dl->getfunc("db_init");
  dl_destroy = (void (*)())dl->getfunc("db_destroy");
  dl_open = (int (*)(char*,int,char*,char*,char*,char*))
      dl->getfunc("db_open");
  dl_close = (bool (*)(int)) dl->getfunc("db_close");
  dl_query = (bool (*)(int,DbTable*,char*)) dl->getfunc("db_query");
  dl_function = (char* (*)(int,char*,char*)) dl->getfunc("db_function");
  dl_get = (bool (*)(int,DbTable*,int,char*)) dl->getfunc("db_get");
  dl_create = (bool (*)(int,DbTable*,char*)) dl->getfunc("db_create");
  dl_create_idx = (bool (*)(int,DbTable*,char*)) dl->getfunc("db_create_idx");
  dl_add = (bool (*)(int,DbTable*,char*)) dl->getfunc("db_add");
  dl_update = (bool (*)(int,DbTable*,char*)) dl->getfunc("db_update");
  dl_del = (bool (*)(int,DbTable*,char*)) dl->getfunc("db_del");
  unlock();
  if(dl_init==0 || dl_destroy==0 ||
      dl_open==0 || dl_close==0 || dl_query==0 || dl_get==0 ||
        dl_create==0 || dl_create_idx==0 || dl_add==0 ||
          dl_update==0 || dl_function==0)
    return "Error in detecting functions in module";
  dl_init(); // run init function in database module
  return str;
}

// create tables in database
string
TacDb::create() {
  string out;
  char emsg[DB_MODULE_ERRLEN+1];
  if(getShutdown()) return ""; // don't create tables on "closed" db
  DbList *dtl = new DbList;
  int cid;
  if(!db_open(&cid)) { // connect to db
    db_close(cid);
    delete dtl;
    out = out + "% can't connect to " + getdbid() + ", see error log for details\n";
    return out;
  }
  for(int i=0; i<dtl->size(); i++) {
    if(db_create(cid,dtl->get(i),emsg))   // call create
      out = out+"% table "+dtl->get(i)->getName()+" in database "+getdbid()+" created\n";
    else
      out = out+"% can't create table "+dtl->get(i)->getName()+" in database "+getdbid()+", message: "+emsg+"\n";
    if(db_create_idx(cid,dtl->get(i),emsg)) // call create index
      out = out+"% index "+dtl->get(i)->getName()+"_idx in database "+getdbid()+" created\n";
    else
      out = out+"% can't create index "+dtl->get(i)->getName()+"_idx in database "+getdbid()+", message: "+emsg+"\n";
  }
  db_close(cid);
  delete dtl;
  return out;
}

/*
// get db_device data and fill db_device structure
db_device
TacDb::get(ipaddr ip) {
  db_device d(ip);
  char str[400];

  lock();
  // work with device table
  dtl->get(T_DEV)->clear();
  dtl->get(T_DEV)->add(TDB_DEV,ip.get());		// set select value
  dtl->get(T_DEV)->set_request(TDB_DEV);

  // work with database
  if(!db_open()) {      // connect to db
    db_close();
    d.connect = false;
    d.result = false;
    d.result_msg<<"can not connect to database"<<end();
    unlock();
    return d;
  }
  d.connect = true;
  if(!db_query(dtl->get(T_DEV))) {
    db_close();         // disconnect from database
    d.result = false;   // if uncorrect device
    d.result_msg<<"unknown device "<<ip.get()<<" or can\'t do request"<<end();
    unlock();
    return d;
  }
  if(!db_get(dtl->get(T_DEV),0)) {	// we mean, that only one device
    db_close();           // disconnect from database
    d.result = false;     // if uncorrect device
    d.result_msg<<"unknown device "<<ip.get()<<" or can\'t get data"<<end();
    unlock();
    return d;
  }

  //// Ok, now work with results ////
  d.result = true;          // device was found



  // ok, we can stop
  db_close();             // disconnect from db
  d.result = true;
  unlock();
  return d;
}
*/

// get db_user data and fill db_user structure
db_user
TacDb::get(const string user) {
  db_user d;
  char str[400];
  string strn;
  bzero(str,sizeof(str));
  if(user.size() == 0) {  // some check
    d.setResult(false);
    d.setResultMsg("no username");
    return d;
  }
  if(getShutdown()) {
    d.setResult(false);
    d.setResultMsg("database shutdowned");
    return d;
  }
  DbList *dtl = new DbList;
  DbTable *dbtmp = 0;
  // work with user table
  dbtmp = dtl->get(T_USR);
  dbtmp->clear(); // clear data in table
  dbtmp->add(TDB_USR,user); // set SELECT value
  dbtmp->set_request(TDB_USR); // set this is as SELECT WHERE value

  // work with database
  int cid;
  cid = 0;

  if(!db_open(&cid)) { // connect to db
    db_close(cid);
    delete dtl;
    d.setConnect(false);
    d.setResult(false);
    d.setResultMsg("can not connect to database");
    return d;
  }
  d.setConnect(true);
  if(!db_query(cid,dbtmp)) {
    db_close(cid); // disconnect from database
    delete dtl;
    d.setResult(false); // if uncorrect username
    string sout;
    sout = sout + "can\'t do request for username " + user;
    d.setResultMsg(sout);
    return d;
  }
  if(!db_get(cid,dbtmp,0)) { // we mean, that only one user with this username
    db_close(cid);           // disconnect from database
    delete dtl;
    d.setResult(false);     // if uncorrect username
    string sout;
    sout = sout + "no username " + user;
    d.setResultMsg(sout);
    return d;
  }
  //// Ok, now work with results ////
  d.setResult(true); // user was found	
  time_t topen;
  topen = 0;
  time_t tclose;
  tclose = 0;
  dbtmp->get(TDB_OPEN,&topen);
  dbtmp->get(TDB_CLOSE,&tclose);
  d.setOpen(topen);
  d.setClose(tclose);
  d.setUsername(user); // store username
  d.setAuthsrc("db"); // authen source
  dbtmp->get(TDB_PWD,(char*)str);
  d.setPassword(decrypt(str,(char*)d.getUsername().c_str()));
  dbtmp->get(TDB_AVPID,&strn);
  d.setAvpid(strn); // authorization group
  dbtmp->get(TDB_ACCID,&strn);
  d.setAccid(strn); // access group
  dbtmp->get(TDB_BILID,&strn);
  d.setBilid(strn); // billing group
  int x;
  x = 0;
  dbtmp->get(TDB_MAXS,&x);
  d.setMaxsess(x);
//dtl->get(T_USR)->get("enable",(char*)str);
//d.enable<<decrypt(str,d.username.c_str())<<end();

  dbtmp->get(TDB_ADDID,&strn);
  d.setAddid(strn); // add group

  // work with additional group
  if(d.getAddid().size() > 0) {
    dbtmp = dtl->get(T_ADD);
    dbtmp->clear();
    dbtmp->add(TDB_ADDID,d.getAddid());
    dbtmp->set_request(TDB_ADDID);
    if(db_query(cid,dbtmp) && db_get(cid,dbtmp,0)) { // only one entry
      dbtmp->get(TDB_AUTHSRC,&strn);
      d.setAuthsrc(strn);
      dbtmp->get(TDB_ENABLE,(char*)str);
      d.setEnable(decrypt(str,(char*)d.getAddid().c_str())); // personal enable for each user
    }
  }

  // work with ACCESS table
  if(d.getAccid().size() > 0) {
    dbtmp = dtl->get(T_ACC);
    dbtmp->clear(); // clear table information
    dbtmp->add(TDB_ACCID,d.getAccid());
    dbtmp->set_request(TDB_ACCID);
    db_user_acc acc;
    if(db_query(cid,dbtmp) && db_get(cid,dbtmp,0)) {
      for(int k=0; db_get(cid,dbtmp,k); k++) {
        dbtmp->get(TDB_PHONE,&strn);
        acc.setPhone(strn);
        dbtmp->get(TDB_NAS,&strn);
        acc.setNas(strn);
        dbtmp->get(TDB_PORT,&strn);
        acc.setPort(strn);
        dbtmp->get(TDB_TIME,&strn);
        acc.setTime(strn);
        d.acc_add(acc);
      }
    }
  }

  // load av-pairs (authorization data)
  if(d.getAvpid().size() > 0) {
    dbtmp = dtl->get(T_AVP);
    dbtmp->clear();
    dbtmp->add(TDB_AVPID,d.getAvpid());
    dbtmp->set_request(TDB_AVPID);
    user_avp avp;
    if(db_query(cid,dbtmp) && db_get(cid,dbtmp,0)) {
      for(int k=0; db_get(cid,dbtmp,k); k++) {
        dbtmp->get(TDB_SERVICE,&strn);
        if(strn.size() == 0) continue;
        avp.setService(strn);
        dbtmp->get(TDB_PROTOCOL,&strn);
        avp.setProtocol(strn);
        dbtmp->get(TDB_AVP,&strn);
        avp.setAvp(strn);
        d.avp_add(avp);
      }
    }
  }

  // load per-command av-pairs
  if(d.getAvpid().size() > 0) {
    dbtmp = dtl->get(T_CMD);
    dbtmp->clear();
    dbtmp->add(TDB_AVPID,d.getAvpid());
    dbtmp->set_request(TDB_AVPID);
    cmd_avp cmd;
    if(db_query(cid,dbtmp) && db_get(cid,dbtmp,0)) {
      for(int k=0; db_get(cid,dbtmp,k); k++) {
        dbtmp->get(TDB_CMDPERM,&strn);
        cmd.setCmdperm(strn);
        dbtmp->get(TDB_CMDDENY,&strn);
        cmd.setCmddeny(strn);
        dbtmp->get(TDB_ARGPERM,&strn);
        cmd.setArgperm(strn);
        dbtmp->get(TDB_ARGDENY,&strn);
        cmd.setArgdeny(strn);
        d.cmd_add(cmd);
      }
    }
  }

  // load billing group
  if(d.getBilid().size() > 0) {
    dbtmp = dtl->get(T_BIL);
    dbtmp->clear();
    dbtmp->add(TDB_BILID,d.getBilid());
    dbtmp->set_request(TDB_BILID);
    if(db_query(cid,dbtmp) && db_get(cid,dbtmp,0)) {
      dbtmp->get(TDB_BILMODID,&x);
      d.setBilmodid(x);
    }
  }

  // load billing resource group
  if(d.getBilid().size() > 0) {
    dbtmp = dtl->get(T_BILRES);
    dbtmp->clear();
    dbtmp->add(TDB_BILID,d.getBilid());
    dbtmp->set_request(TDB_BILID);
    db_bilres bilres;
    if(db_query(cid,dbtmp) && db_get(cid,dbtmp,0)) {
      for(int k=0; db_get(cid,dbtmp,k); k++) {
        dbtmp->get(TDB_ATTRIBUTE,&strn);
        bilres.setAttribute(strn);
        dbtmp->get(TDB_VALUE,&strn);
        bilres.setValue(strn);
        d.bilres_add(bilres);
      }
    }
  }
  // ok, we can stop
  db_close(cid); // disconnect from db
  delete dtl;
  d.setResult(true);
  return d;
}

// direct function query to SQL server
string
TacDb::direct_function_query(char *f) {
  string str;
  int cid;
  if(!db_open(&cid)) { // connect to db
    db_close(cid);
    return str;
  }
  str = str + db_function(cid,f);
  db_close(cid);
  return str;
}

///// UNIVERSAL MANUPULATION FUNCTIONS //////

// add data into table, where fields=arguments
string
TacDb::add(string table,list<string> argument,list<string> value) {
  DbList *dtl = new DbList;
  DbTable *dbtmp;
  dbtmp = dtl->get(table);
  dbtmp->clear();

  list<string>::iterator agu = argument.begin();
  list<string>::iterator vlu = value.begin();

  char showkey[50];
  bzero(showkey,sizeof(showkey));
  if(value.size() > 0) {
    string showkeystr = *vlu;
    snprintf(showkey,sizeof(showkey),"%s",showkeystr.c_str());
  }

  // run over all entered data
  for(; agu!=argument.end() && vlu!=value.end() ; agu++, vlu++) {
    string argu = *agu;
    string valu = *vlu;
    int j = 0;
    for(j=0; !utl::match(argu.c_str(),dbtmp->get_name(j).c_str()) &&
      dbtmp->get_name(j).size() > 0; j++);

    switch(dbtmp->get_type(j)) {
     case 1:  // bool
      bool val;
      if(utl::match(valu.c_str(),"OPen") || utl::match(valu.c_str(),"TRue")) val = true;
      else val = false;
      dbtmp->add(argu,val);
      break;
     case 2:  // int
      dbtmp->add(argu,atoi(valu.c_str()));
      break;
     case 3:  // float
      dbtmp->add(argu,(float)atof(valu.c_str()));
      break;
     case 4:  // char*
      if((argu == TDB_PWD) || (argu == TDB_ENABLE)) {
        char aaa[100];
        char ccc[100];
        bzero(aaa,sizeof(aaa));
        bzero(ccc,sizeof(ccc));
        snprintf(aaa,sizeof(aaa),"%s",valu.c_str());
        snprintf(ccc,sizeof(ccc),"%s",encrypt(aaa,showkey).c_str());
        dbtmp->add(argu,ccc);
      } else dbtmp->add(argu,utl::toSQL(valu));
      break;
     case 5:  // time_t
      dbtmp->add(argu,utl::iso2time((char*)valu.c_str()));
      break;
     case 6:  // ipaddr
      dbtmp->add(argu,ipaddr(valu));
      break;
    }
  }
  string str;
  int cid;
  if(!db_open(&cid)) {      // connect to db
    str = str + "% Can\'t connect to " + getdbid() + "\n";
  } else {
    if(db_add(cid,dbtmp)) {  // add entry
      str = str + "% Entry added to " + getdbid() + "\n";
    } else {
      str = str + "% Can\'t add entry to " + getdbid() + "\n";
    }
  }
  db_close(cid);
  delete dtl;
  argument.clear();
  value.clear();
  return str;
}

// modify for argument/value and set argument2/value2 for it
string
TacDb::modify(string table,list<string> argument,list<string> value,list<string> argument2,list<string> value2) {
  DbList *dtl = new DbList;
  DbTable *dbtmp = dtl->get(table);
  dbtmp->clear();

  list<string>::iterator agu = argument.begin();
  list<string>::iterator vlu = value.begin();
  list<string>::iterator agu2 = argument2.begin();
  list<string>::iterator vlu2 = value2.begin();
  char showkey[50];
  bzero(showkey,sizeof(showkey));
  if(value.size() > 0) {
    string showkeystr = *vlu;
    snprintf(showkey,sizeof(showkey),"%s",showkeystr.c_str());
  }

  for(; agu2!=argument2.end() && vlu2!=value2.end() ; agu2++, vlu2++) {
    string argu2 = *agu2;
    string valu2 = *vlu2;

    // set new values
    int j = 0;
    for(j=0; !utl::match(argu2.c_str(),dbtmp->get_name(j).c_str()) &&
      dbtmp->get_name(j).size() > 0; j++);
    switch(dbtmp->get_type(j)) {
     case 1:      // bool
      bool val;
      if(utl::match(valu2.c_str(),"OPen") || utl::match(valu2.c_str(),"TRue")) val = true;
      else val = false;
      dbtmp->add(argu2,val);
      break;
     case 2: // int
      dbtmp->add(argu2,atoi(valu2.c_str()));
      break;
     case 3: // float
      dbtmp->add(argu2,(float)atof(valu2.c_str()));
      break;
     case 4: // char*
      if(argu2==TDB_PWD || argu2==TDB_ENABLE) {
        char aaa[100];
        bzero(aaa,sizeof(aaa));
        snprintf(aaa,sizeof(aaa),"%s",valu2.c_str());
        dbtmp->add(argu2,encrypt(aaa,showkey));
      } else dbtmp->add(argu2,valu2);
      break;
     case 5: // time_t
      dbtmp->add(argu2,utl::iso2time((char*)valu2.c_str()));
      break;
     case 6: // ipaddr
      dbtmp->add(argu2,ipaddr(valu2));
      break;
    }
    dbtmp->set_update(argu2);
  }
  for(; agu!=argument.end() && vlu!=value.end() ; agu++, vlu++) {
    string argu = *agu;
    string valu = *vlu;
    // set request
    int j = 0;
    for(j=0; !utl::match(argu.c_str(),dbtmp->get_name(j).c_str()) &&
      dbtmp->get_name(j).size() > 0; j++);
    switch(dbtmp->get_type(j)) {
     case 1:      // bool
      bool val;
      if(utl::match(valu.c_str(),"OPen") || utl::match(valu.c_str(),"TRue")) val = true;
      else val = false;
      dbtmp->add(argu,val);
      break;
     case 2:      // int
      dbtmp->add(argu,atoi(valu.c_str()));
      break;
     case 3:      // float
      dbtmp->add(argu,(float)atof(valu.c_str()));
      break;
     case 4: // char*
      if(argu==TDB_PWD || argu==TDB_ENABLE) {
        char aaa[100];
        bzero(aaa,sizeof(aaa));
        snprintf(aaa,sizeof(aaa),"%s",valu.c_str());
        dbtmp->add(argu,decrypt(aaa,showkey));
      } else dbtmp->add(argu,valu);
      break;
     case 5: // time_t
      dbtmp->add(argu,utl::iso2time((char*)valu.c_str()));
      break;
     case 6: // ipaddr
      dbtmp->add(argu,ipaddr(valu));
      break;
    }
    dbtmp->set_request(argu);
  }
  string str;
  int cid;
  if(!db_open(&cid))      // connect to db
    str = str + "% Can't connect to " + getdbid() + "\n";
  if(!db_update(cid,dtl->get(table)))   // select
    str = str + "% Can't process update request for " + getdbid() + "\n";
  else
    str = str + "% Update request for " + getdbid() + " completed\n";
  db_close(cid);
  delete dtl;
  argument.clear();
  value.clear();
  argument2.clear();
  value2.clear();
  return str;
}

// del data for arguments/values
string
TacDb::del(string table,list<string> argument,list<string> value) {
  DbList *dtl = new DbList;
  DbTable *dbtmp = dtl->get(table);
  dbtmp->clear();
  list<string>::iterator agu = argument.begin();
  list<string>::iterator vlu = value.begin();
  char showkey[50];
  bzero(showkey,sizeof(showkey));
  if(value.size() > 0) {
    string showkeystr = *vlu;
    snprintf(showkey,sizeof(showkey),"%s",showkeystr.c_str());
  }
  for(; agu!=argument.end() && vlu!=value.end() ; agu++, vlu++) {
    string argu = *agu;
    string valu = *vlu;
    int j = 0;
    for(j=0; !utl::match(argu.c_str(),dbtmp->get_name(j).c_str()) &&
      dbtmp->get_name(j).size() > 0; j++);
//std::cerr<<"field type["<<j<<"]="<<dtl->get(table)->get_type(j)<<std::endl;
    switch(dbtmp->get_type(j)) {
     case 1: // bool
      bool val;
      if(utl::match(valu.c_str(),"OPen") || utl::match(valu.c_str(),"TRue")) val = true;
      else val = false;
      dbtmp->add(argu,val);
      break;
     case 2: // int
      dbtmp->add(argu,atoi(valu.c_str()));
      break;
     case 3: // float
      dbtmp->add(argu,(float)atof(valu.c_str()));
      break;
     case 4: // char*
      if(argu==TDB_PWD || argu==TDB_ENABLE) {
        char aaa[100];
        char ccc[100];
        bzero(aaa,sizeof(aaa));
        bzero(ccc,sizeof(ccc));
        snprintf(aaa,sizeof(aaa),"%s",valu.c_str());
        snprintf(ccc,sizeof(ccc),"%s",encrypt(aaa,showkey).c_str());
        dbtmp->add(argu,ccc);
      } else dbtmp->add(argu,valu);
      break;
     case 5: // time_t
      dbtmp->add(argu,utl::iso2time((char*)valu.c_str()));
      break;
     case 6: // ipaddr
      dbtmp->add(argu,ipaddr(valu));
      break;
    }
    dbtmp->set_request(argu);
  }
  string str;
  int cid;
  if(!db_open(&cid)) { // connect to db
    str = str + "% Can't connect to " + getdbid() + "\n";
  } else if(db_del(cid,dbtmp)) // del entry
    str = str + "% Entry deleted from " + getdbid() + "\n";
  else
    str = str + "% Can't delete entry from " + getdbid() + "\n";
  db_close(cid);
  delete dtl;
  argument.clear();
  value.clear();
  return str;
}

// stripped show
list<string>
TacDb::sshow(string table,list<string> argument,list<string> value) {
  list<string> sout;
  DbList *dtl = new DbList;
  DbTable *dbtmp = dtl->get(table);
  dbtmp->clear();
  list<string>::iterator agu = argument.begin();
  list<string>::iterator vlu = value.begin();
  char showkey[50];
  bzero(showkey,sizeof(showkey));
  if(value.size() > 0) {
    string showkeystr = *vlu;
    snprintf(showkey,sizeof(showkey),"%s",showkeystr.c_str());
  }
  for(; agu!=argument.end() && vlu!=value.end() ; agu++, vlu++) {
    string argu = *agu;
    string valu = *vlu;
    int j = 0;
    for(j=0; j<dbtmp->size() && !utl::match(argu.c_str(),dbtmp->get_name(j).c_str());j++);
    switch(dbtmp->get_type(j)) {
     case 1: // bool
      bool val;
      if(utl::match(valu.c_str(),"OPen") || utl::match(valu.c_str(),"TRue")) val = true;
      else val = false;
      dbtmp->add(argu,val);
      break;
     case 2: // int
      dbtmp->add(argu,atoi(valu.c_str()));
      break;
     case 3: // float
      dbtmp->add(argu,(float)atof(valu.c_str()));
      break;
     case 4: // char*
      if((argu == TDB_PWD) || (argu == TDB_ENABLE)) {
        char aaa[100];
        char ccc[100];
        bzero(aaa,sizeof(aaa));
        bzero(ccc,sizeof(ccc));
        snprintf(aaa,sizeof(aaa),"%s",valu.c_str());
        snprintf(ccc,sizeof(ccc),"%s",encrypt(aaa,showkey).c_str());
        dbtmp->add(argu,ccc);
      } else dbtmp->add(argu,valu);
      break;
     case 5: // time_t
      dbtmp->add(argu,utl::iso2time((char*)valu.c_str()));
      break;
     case 6: // ipaddr
      dbtmp->add(argu,ipaddr(valu));
      break;
    }
    dbtmp->set_request(argu);
  }
  argument.clear();
  value.clear();

  char tmps[100];
  bool tmpb;
  tmpb = false;
  int tmpi;
  tmpi = 0;
  float tmpf;
  tmpf = 0;
  time_t tmpt;
  tmpt = 0;
  ipaddr tmpip;
  int cid;
  string tmps2;
  string fieldname;
  string showvarkey;
  if(!db_open(&cid)) { // connect to db
    db_close(cid);
    string sss;
    sss = sss + "% cant connect to " + getdbid();
    sout.push_back(sss);
    delete dtl;
    return sout;
  }
  if(!db_query(cid,dbtmp)) { // select
    db_close(cid);
    string sss;
    sss = sss + "% cant send request to " + getdbid();
    sout.push_back(sss);
    delete dtl;
    return sout;
  }
  if(!db_get(cid,dbtmp,0)) { // get
    db_close(cid);
    string sss;
    sss = sss + "% cant get data from " + getdbid();
    sout.push_back(sss);
    delete dtl;
    return sout;
  }
  for(int i=0; i < dbtmp->size(); i++) {
    fieldname = dbtmp->get_name(i);
    switch(dbtmp->get_type(i)) {
     case 1: // bool
      dbtmp->get(fieldname,&tmpb);
      snprintf(tmps,sizeof(tmps),"%s",tmpb ? "open" : "close");
      break;
     case 2: // int
      dbtmp->get(fieldname,&tmpi);
      snprintf(tmps,sizeof(tmps),"%d",tmpi);
      break;
     case 3: // float
      dbtmp->get(fieldname,&tmpf);
      snprintf(tmps,sizeof(tmps),"%f",tmpf);
      break;
     case 4: // char*
      dbtmp->get(fieldname,&tmps2);
      if(i == 0) showvarkey = tmps2;
      if(fieldname==TDB_PWD || fieldname==TDB_ENABLE) {
        snprintf(tmps,sizeof(tmps),"%s",decrypt((char*)tmps2.c_str(),(char*)showvarkey.c_str()).c_str());
      } else snprintf(tmps,sizeof(tmps),"%s",tmps2.c_str());
      break;
     case 5: // time_t
      dbtmp->get(fieldname,&tmpt);
      snprintf(tmps,sizeof(tmps),"%s",utl::time2iso(tmpt).c_str());
      break;
     case 6: // ipaddr
      dbtmp->get(fieldname,&tmpip);
      snprintf(tmps,sizeof(tmps),"%s",tmpip.get().c_str());
      break;
    }
    sout.push_back(tmps);
  }
  db_close(cid);
  delete dtl;
  return sout;
}

// show
string
TacDb::show(string table,list<string> argument,list<string> value) {
  DbList *dtl = new DbList;
  DbTable *dbtmp = dtl->get(table);
  if(!dbtmp) {
    delete dtl;
    return "";
  }
  dbtmp->clear();
  int j;
  list<string>::iterator agu = argument.begin();
  list<string>::iterator vlu = value.begin();
  char showkey[50];
  bzero(showkey,sizeof(showkey));
  if(value.size() > 0) {
    string showkeystr = *vlu;
    snprintf(showkey,sizeof(showkey),"%s",showkeystr.c_str());
  }
  for(; agu!=argument.end() && vlu!=value.end() ; agu++, vlu++) {
    string argu = *agu;
    string valu = *vlu;
    j = 0;
    for(j=0; dbtmp->get_name(j).size()>0 &&
      !utl::match(argu.c_str(),dbtmp->get_name(j).c_str()); j++);
    if(!utl::match(argu.c_str(),dbtmp->get_name(j).c_str())) continue;
    int ftype = dbtmp->get_type(j);
    switch(ftype) {
     case 1: // bool
      bool val;
      if(utl::match(valu.c_str(),"OPen") || utl::match(valu.c_str(),"TRue")) val = true;
      else val = false;
      dbtmp->add(argu,val);
      break;
     case 2: // int
      dbtmp->add(argu,atoi(valu.c_str()));
      break;
     case 3: // float
      dbtmp->add(argu,(float)atof(valu.c_str()));
      break;
     case 4: // char*
      if(argu==TDB_PWD || argu==TDB_ENABLE) {
        char aaa[100];
        char ccc[100];
        bzero(aaa,sizeof(aaa));
        bzero(ccc,sizeof(ccc));
        snprintf(aaa,sizeof(aaa),"%s",valu.c_str());
        snprintf(ccc,sizeof(ccc),"%s",encrypt(aaa,showkey).c_str());
        dbtmp->add(argu.c_str(),ccc);
      } else dbtmp->add(argu,valu);
      break;
     case 5: // time_t
      dbtmp->add(argu,utl::iso2time(valu.c_str()));
      break;
     case 6: // ipaddr
      dbtmp->add(argu,ipaddr(valu));
      break;
    }
    dbtmp->set_request(argu);
  }
  argument.clear();
  value.clear();

  string str;
  string str2;
  char tmps[100];
  bool tmpb;
  tmpb = false;
  int tmpi;
  tmpi = 0;
  float tmpf;
  tmpf = 0;
  time_t tmpt;
  tmpt = 0;
  ipaddr tmpip;
  int cid;
  if(!db_open(&cid)) {      // connect to db
    db_close(cid);
    str = str + "% Can't connect to " + getdbid() + "\n";
    delete dtl;
    return str;
  }
  if(!db_query(cid,dbtmp)) { // select
    db_close(cid);
    str = str + "% Can't send request to " + getdbid() + "\n";
    delete dtl;
    return str;
  }
  if(!db_get(cid,dbtmp,0)) {  // get
    db_close(cid);
    str = str + "% Can't get data from " + getdbid() + "\n";
    delete dtl;
    return str;
  }
  // create header
  for(int i=0;i < dbtmp->size();i++) {
    str = str + " ";
    str = str + utl::lower(dbtmp->get_name(i));
    str = str + " |";
    for(int j=0; j<(int)dbtmp->get_name(i).size()+2; j++) str2 = str2 + "-";
    str2 = str2 + "+";
  }
  str = str + "\n" + str2 + "\n";
  string fieldname;
  string showvarkey;
  string tmps2;
  for(int k=0; db_get(cid,dbtmp,k); k++) {
    for(int i=0; i < dbtmp->size(); i++) {
      bzero(tmps,sizeof(tmps));
      fieldname = dbtmp->get_name(i);
      int ftype = dbtmp->get_type(i);
      switch(ftype) {
       case 1: // bool
        dbtmp->get(fieldname,&tmpb);
        snprintf(tmps,sizeof(tmps),"%s",tmpb ? "open" : "close");
        break;
       case 2: // int
        dbtmp->get(fieldname,&tmpi);
        snprintf(tmps,sizeof(tmps),"%d",tmpi);
        break;
       case 3: // float
        dbtmp->get(fieldname,&tmpf);
        snprintf(tmps,sizeof(tmps),"%f",tmpf);
        break;
       case 4: // char*
        dbtmp->get(fieldname,&tmps2);
        if(i == 0) showvarkey = tmps2;
        if(fieldname==TDB_PWD || fieldname==TDB_ENABLE) {
          char svk[100];
          snprintf(svk,sizeof(svk)-1,"%s",showvarkey.c_str());
          char crp[100];
          snprintf(crp,sizeof(crp)-1,"%s",tmps2.c_str());
          snprintf(tmps,sizeof(tmps),"%s",decrypt(crp,svk).c_str());
        } else snprintf(tmps,sizeof(tmps),"%s",tmps2.c_str());
        break;
       case 5: // time_t
        dbtmp->get(fieldname,&tmpt);
        snprintf(tmps,sizeof(tmps),"%s",utl::time2iso(tmpt).c_str());
        break;
       case 6: // ipaddr
        dbtmp->get(fieldname,&tmpip);
        snprintf(tmps,sizeof(tmps),"%s",tmpip.get().c_str());
        break;
      }
      str = str + tmps + " | ";
    }
    str = str + "\n";
  }
  db_close(cid);
  delete dtl;
  return str;
}

/////////////// add data to log
string
TacDb::add_log(log_user *lu) {
  if(lu->username.size() == 0) return "";
//  lock();
  char emsg[DB_MODULE_ERRLEN+1];
  ipaddr our_ip;
  char hostname[64];
  gethostname((char*)hostname,sizeof(hostname));
  our_ip = hostname;
  int cid = 0;
  time_t stop_time = lu->start_time + lu->elapsed;
  struct tm res;
  struct tm *tt = localtime_r(&stop_time,&res);
  char str[100];
  snprintf(str,sizeof(str)-1,"%04d%02d",1900+tt->tm_year,tt->tm_mon+1);
  string t_log_name;
  t_log_name = t_log_name + T_LOG + "_" + str;
  bool create = false;
  DbList *dtl;
  DbTable *dbtmp;

add_log_Again:;

  // log table
  dtl = new DbList;
  dtl->add(t_log_name,T_LOG_ID,"user logs");
  dbtmp = dtl->get(t_log_name);

  if(create) {
    dbtmp->add(TDB_DCHECK,"id","",T_DATE_TYPE);
    dbtmp->add(TDB_USR,"user name","",T_STR_TYPE,DB_USERNAME_LEN);
    dbtmp->add("server","server ip","",T_IP_TYPE);
//    dbtmp->add("servtime","time on server","",T_DATE_TYPE);
    dbtmp->add("start_time","session start time","",T_DATE_TYPE,T_UNIQUE);
    //
    dbtmp->add("stop_time","session stop time","",T_DATE_TYPE);
    //
    dbtmp->add(TDB_NAS,"network device","",T_IP_TYPE,T_UNIQUE);
    dbtmp->add(TDB_PORT,"port","",T_STR_TYPE,16,T_UNIQUE);
    dbtmp->add(TDB_SERVICE,"service","",T_STR_TYPE,10);
    dbtmp->add("cfrom","phone","",T_STR_TYPE,64);
    dbtmp->add("ip","ip addr","",T_IP_TYPE);
    dbtmp->add(TDB_PROTOCOL,"protocol","",T_STR_TYPE,10);
    dbtmp->add("elapsed","elapsed time","",T_UINT_TYPE);
    dbtmp->add("bytes_in","bytes in","",T_UINT_TYPE);
    dbtmp->add("bytes_out","bytes out","",T_UINT_TYPE);
    dbtmp->add("disc_cause","disconnect reason","",T_INT_TYPE);
    dbtmp->add("disc_cause_ext","ext disconnect reason","",T_INT_TYPE);
    dbtmp->add("rx_speed","rx speed","",T_UINT_TYPE);
    dbtmp->add("tx_speed","tx speed","",T_UINT_TYPE);
    dbtmp->add("task_id","task id","",T_INT_TYPE);
    dbtmp->add("h323_call_origin","","",T_STR_TYPE,10);
    dbtmp->add("h323_call_type","","",T_STR_TYPE,15);
    dbtmp->add("h323_disconnect_cause","","",T_INT_TYPE);
    dbtmp->add("h323_voice_quality","","",T_INT_TYPE);
    dbtmp->add("h323_remote_address","","",T_IP_TYPE);
    dbtmp->add("h323_connect_time","","",T_DATE_TYPE);
    dbtmp->add("h323_disconnect_time","","",T_DATE_TYPE);

    if(!db_open(&cid)) {      // connect to db
      db_close(cid);
      delete dtl;
      return "error during LOG connect";
    }
//cerr<<"try to create database"<<endl;
    if(db_create(cid,dbtmp,emsg)) {  // call create
      db_create_idx(cid,dbtmp,emsg);
//cerr<<"create database"<<endl;
    }
    db_close(cid);
  }
  dbtmp->add("server",our_ip);
//  dbtmp->add("servtime",lu->servtime);
  dbtmp->add("start_time",lu->start_time);
  dbtmp->add("stop_time",stop_time);
  dbtmp->add(TDB_USR,lu->username);
  dbtmp->add(TDB_NAS,lu->nas);
  dbtmp->add(TDB_PORT,lu->port);
//cerr<<"add_log: serv="<<our_ip.get()<<" user="<<lu->username<<" nas="<<lu->nas.get()<<endl;
  dbtmp->add(TDB_SERVICE,lu->service);
  dbtmp->add("cfrom",lu->from);
  dbtmp->add("ip",lu->ip);
  dbtmp->add(TDB_PROTOCOL,lu->protocol);
  dbtmp->add("elapsed",lu->elapsed);
  dbtmp->add("bytes_in",lu->bytes_in);
  dbtmp->add("bytes_out",lu->bytes_out);
  dbtmp->add("disc_cause",lu->disc_cause);
  dbtmp->add("disc_cause_ext",lu->disc_cause_ext);
  dbtmp->add("rx_speed",lu->rx_speed);
  dbtmp->add("tx_speed",lu->tx_speed);
  dbtmp->add("task_id",lu->task_id);
  dbtmp->add("h323_call_origin",lu->h323_call_origin);
  dbtmp->add("h323_call_type",lu->h323_call_type);
  dbtmp->add("h323_disconnect_cause",lu->h323_disconnect_cause);
  dbtmp->add("h323_voice_quality",lu->h323_voice_quality);
  dbtmp->add("h323_remote_address",lu->h323_remote_address);
  dbtmp->add("h323_connect_time",lu->h323_connect_time);
  dbtmp->add("h323_disconnect_time",lu->h323_disconnect_time);

  if(!db_open(&cid)) {      // connect to db
    db_close(cid);
    delete dtl;
    return "error during LOG connect";
  }
  if(!db_add(cid,dbtmp)) { // send query
    // may be table do not created, create it now
    if(!create) {
      create = true;
      db_close(cid);
      delete dtl;
      goto add_log_Again; // try to create table and add log again
    }
    db_close(cid);
    delete dtl;
    return "error during insert LOG query";
  }
  db_close(cid);
  delete dtl;
  return "";
}

// add snmp traffic data to log
string
TacDb::add_trf(char *name,SnmpOut *d) {
  ipaddr our_addr;
  time_t ttt = time(0);
  char emsg[DB_MODULE_ERRLEN+1];
  ipaddr our_ip;
  char hostname[64];
  gethostname((char*)hostname,sizeof(hostname));
  our_ip = hostname;
  int cid = 0;
  struct tm res;
  struct tm *tt = localtime_r(&ttt,&res);
  char str[100];
  snprintf(str,sizeof(str)-1,"%04d%02d",1900+tt->tm_year,tt->tm_mon+1);
  string t_trf_name;
  t_trf_name = t_trf_name + T_TRF + "_" + str;
  bool create = false;

  DbList *dtl;
  DbTable *dbtmp;

add_trf_Again:;

  dtl = new DbList;
  dtl->add(t_trf_name,T_TRF_ID,"per interface traffic counters");
  dbtmp = dtl->get(t_trf_name);

  if(create) {
    // datatable
    dbtmp->add("dcheck","id","",T_DATE_TYPE);
    dbtmp->add(TDB_DEV,"network device","",T_IP_TYPE,T_INDEX);
    dbtmp->add("server","server","",T_IP_TYPE);
    dbtmp->add("servtime","current server time","",T_DATE_TYPE);
    dbtmp->add("port","port","",T_STR_TYPE,16,T_INDEX);
    //  dbtmp->add("intf","intf description","",T_STR_TYPE,50);
    dbtmp->add("bytein","bytes in","",T_UINT_TYPE);
    dbtmp->add("byteout","bytes out","",T_UINT_TYPE);

    if(db_create(cid,dbtmp,emsg)) {  // call create
      db_create_idx(cid,dbtmp,emsg);
    }
    db_close(cid);
  }
  dbtmp->add("server",our_addr);
  dbtmp->add("servtime",ttt);
  dbtmp->add("port",d->getPort());
  dbtmp->add(TDB_DEV,ipaddr(name));
//  dbtmp->add("intf",d->description);
  dbtmp->add("bytein",d->getIfIO());
  dbtmp->add("byteout",d->getIfOO());

  if(!db_open(&cid)) { // connect to db
    db_close(cid);
    delete dtl;
    return "error during TRAFFIC LOG connect";
  }
  if(!db_add(cid,dbtmp)) { // send query
    // may be table do not created, create it now
    if(!create) {
      create = true;
      delete dtl;
      goto add_trf_Again;
    }
    db_close(cid);
    delete dtl;
    return "error during insert TRF query, create table and try write log again";
  }
  db_close(cid);
  delete dtl;
  return "";
}

// open writing netflow data to database
int
TacDb::open_nf() {
  int cid = 0;
  if(!db_open(&cid)) { // connect to db
    db_close(cid);
    return 0;
  }
  return cid;
}

// close writing netflow data to database
void
TacDb::close_nf(int cid) {
  db_close(cid);
}

// add netflow traffic data to log
string
TacDb::add_nf(int cid,Bit32 dv,Bit32 sa,Bit32 da,time_t startt,time_t stopt,unsigned long octets,unsigned long packets,int ifin) {
//  lock(); // !!!!!
  time_t ttt = stopt; // flow stop time
  char emsg[DB_MODULE_ERRLEN+1];
//  ipaddr our_ip;
//  char hostname[64];
//  gethostname((char*)hostname,sizeof(hostname));
//  our_ip = hostname;
  struct tm res;
  struct tm *tt = localtime_r(&ttt,&res);
  char str[100];
  snprintf(str,sizeof(str)-1,"%04d%02d%02d%02d",1900+tt->tm_year,
    tt->tm_mon+1,tt->tm_mday,tt->tm_hour);
  string t_nf_name;
  t_nf_name = t_nf_name + T_NF + "_" + str;
  static bool create = false;

  DbList *dtl;
  DbTable *dbtmp;

add_nf_Again:;

  dtl = new DbList;
  dtl->add(t_nf_name,T_TRF_ID,"netflow traffic counter");
  dbtmp = dtl->get(t_nf_name);

  if(create) {
    // datatable
    dbtmp->add("server","server","",T_IP_TYPE);
//    dbtmp->add("servtime","current server time","",T_DATE_TYPE);
    dbtmp->add(TDB_DEV,"network device","",T_IP_TYPE);
    dbtmp->add("ifin","input interface","",T_UINT_TYPE);
    dbtmp->add("source","source address","",T_IP_TYPE);
    dbtmp->add("destination","destination address","",T_IP_TYPE);
    dbtmp->add("starttime","flow start time","",T_DATE_TYPE);
    dbtmp->add("stoptime","flow stop time","",T_DATE_TYPE);
    dbtmp->add("octets","bytes","",T_UINT_TYPE);
    dbtmp->add("packets","paks","",T_UINT_TYPE);
    dbtmp->add("srcuser","source username","",T_STR_TYPE,DB_USERNAME_LEN);
    dbtmp->add("dstuser","destination username","",T_STR_TYPE,DB_USERNAME_LEN);
    if(db_create(cid,dbtmp,emsg)) {  // call create
      db_create_idx(cid,dbtmp,emsg);
    }
    create = false;
    delete dtl;
    goto add_nf_Again;
  }
  dbtmp->add("server",our_ip_);
//  dbtmp->add("servtime",ttt);
  dbtmp->add(TDB_DEV,ipaddr(utl::sip(dv))); //->getDevAddr());
  dbtmp->add("ifin",ifin); //d->getIfIn());
  dbtmp->add("source",ipaddr(utl::sip(sa))); //d->getSource());
  dbtmp->add("destination",ipaddr(utl::sip(da))); //d->getDest());
  dbtmp->add("starttime",startt); //d->getStart());
  dbtmp->add("stoptime",stopt); //d->getStop());
  dbtmp->add("octets",octets); //d->getOctets());
  dbtmp->add("packets",packets); //d->getPackets());
  dbtmp->add("srcuser",::coreData->getDevice()->getuser4ip(ipaddr(utl::sip(sa))).getUsername());
  dbtmp->add("dstuser",::coreData->getDevice()->getuser4ip(ipaddr(utl::sip(da))).getUsername());

  if(!db_add(cid,dbtmp)) { // send query
    // may be table do not created, create it now
    if(!create) {
      create = true;
      delete dtl;
      goto add_nf_Again;
    }
    delete dtl;
//    unlock(); // !!!!!
    return "error during insert TRF query";
  }
  delete dtl;
//  unlock(); // !!!!!
  return "";
}

string
TacDb::get_table_name(int n) {
  string ret;
  DbList *dtl = new DbList;
  if(n < dtl->size()) ret = dtl->get(n)->getName();
//std::cerr<<"TacDb::get_table_name dtl->size = "<<dtl->size()<<std::endl;
  delete dtl;
  return ret;
}

string
TacDb::get_table_mask(int n) {
  string ret;
  DbList *dtl = new DbList;
  if(n < dtl->size()) ret = dtl->get(n)->getNamemask();
  delete dtl;
  return ret;
}

string
TacDb::get_table_descr(int n) {
  string ret;
  DbList *dtl = new DbList;
  if(n < dtl->size()) ret = dtl->get(n)->getDescription();
  delete dtl;
  return ret;
}

string
TacDb::get_field_name(int t,int n) {
  string ret;
  DbList *dtl = new DbList;
//cerr<<"TacDb::get_field_name point 1 t="<<t<<" n="<<n<<" dtl->size()="<<dtl->size()<<endl;
  if(t < dtl->size()) ret = dtl->get(t)->get_name(n);
//cerr<<"TacDb::get_field_name point 2"<<endl;
  delete dtl;
  return ret;
}

string
TacDb::get_field_descr(int t,int n) {
  string ret;
  DbList *dtl = new DbList;
  if(t < dtl->size()) ret = dtl->get(t)->get_descr(n);
  delete dtl;
  return ret;
}

string
TacDb::get_field_defv(int t,int n) {
  string ret;
  DbList *dtl = new DbList;
  if(t < dtl->size()) ret = dtl->get(t)->get_defv(n);
  delete dtl;
  return ret;
}

int
TacDb::get_field_type(int t,int n) {
  int ret = 0;
  DbList *dtl = new DbList;
  if(t < dtl->size()) ret = dtl->get(t)->get_type(n);
  delete dtl;
  return ret;
}

int
TacDb::get_field_num(int t) {
  int ret = 0;
  DbList *dtl = new DbList;
  if(t < dtl->size()) ret = dtl->get(t)->size();
  delete dtl;
  return ret;
}

///////////////////////// that's all //////////////////////////
