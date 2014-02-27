// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: TacDb.h
// description: database adaptation layer

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

// all comments which start from /** - this is comments for KDoc */
//  for classes
//    @short [short description of the class]
//    @author [class author]
//    @version [class version]
//    @see [references to other classes or methods]
//  for methods
//    @see [references]
//    @return [sentence describing the return value]
//    @exception [list the exeptions that could be thrown]
//    @param [name id] [description]	- can be multiple

#ifndef __TACDB_H__
#define __TACDB_H__

#include "Snmp.h"       // it sometimes required
#include "global.h"

namespace tacpp {

// database table names (this is for version 1 tables)
const string T_DEV = "tacppd_v1_dev";      // network devices table
const string T_USR = "tacppd_v1_usr";      // user/passwords table
const string T_ACC = "tacppd_v1_acc";      // access control
const string T_AVP = "tacppd_v1_avp";      // authorization
const string T_CMD = "tacppd_v1_cmd";      // command authoriz
const string T_ADD = "tacppd_v1_add";      // additional user data
const string T_LOG = "tacppd_v1_log";      // log inform
const string T_TRF = "tacppd_v1_trf";      // intf traffic info
const string T_NF  = "tacppd_v1_nf";       // netflow traffic info
const string T_BIL = "tacppd_v1_res";      // resource group
const string T_BILRES = "tacppd_v1_resdata";    // resource group resources
const string T_NFUSR = "tacppd_v1_nfuser"; // user for collect netflow data

const string T_DEV_ID =     "DEvice";
const string T_USR_ID =     "USer";
const string T_ACC_ID =     "ACcess";
const string T_AVP_ID =     "AVp";
const string T_CMD_ID =     "COmmand";
const string T_ADD_ID =     "ADd";
const string T_LOG_ID =     "LOg";
const string T_TRF_ID =     "TRFInt";
const string T_BIL_ID =     "BILLIng";
const string T_BILRES_ID =  "BILLRes";
const string T_NFUSR_ID =  "NFUser";

#define AVP_SERVICE_LEN   15
#define AVP_PROTOCOL_LEN  15
#define AVP_VALUE_LEN     128

/**
@short authorization information peace
@version 1
@author rv
*/
class user_avp {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  service (ppp,shell,slip,...) */
  string service_;
  /**
  protocol (lcp,ip,...) */
  string protocol_;
  /**
  AV-pair */
  string avp_;
 public:
  /**
  service_ */
  string getService();
  /**
  set service */
  void setService(const string);
  /**
  compare services
  @return true if equal */
  bool compareService(const string);
  /**
  protocol_ */
  string getProtocol();
  /**
  set protocol */
  void setProtocol(const string);
  /**
  compare protocols */
  bool compareProtocol(const string);
  /**
  avp_ */
  string getAvp();
  /**
  set avp */
  void setAvp(const string);
  /**
  copy */
  void operator =(user_avp);
  /**
  compare */
  bool operator ==(user_avp);
  /**
  not compare */
  bool operator !=(user_avp);
  /**
  init */
  user_avp();
  /**
  free */
  ~user_avp();
};

const int CMD_AVP_LEN = 50;

/**
@short per-command authorization
@version 1
@author rv
*/
class cmd_avp {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  permit command */
  string cmdperm_;
  /**
  deny command */
  string cmddeny_;
  /**
  permit argument */
  string argperm_;
  /**
  deny argument */
  string argdeny_;
 public:
  /**
  get command perm */
  string getCmdperm();
  /**
  set command perm */
  void setCmdperm(const string);
  /**
  ger cmd deny */
  string getCmddeny();
  /**
  set cmd deny */
  void setCmddeny(const string);
  /**
  get permitted arg */
  string getArgperm();
  /**
  set arg perm */
  void setArgperm(const string);
  /**
  get argd */
  string getArgdeny();
  /**
  set arg deny */
  void setArgdeny(const string);
  /**
  copy */
  void operator =(cmd_avp);
  /**
  compare */
  bool operator ==(cmd_avp);
  /**
  init */
  cmd_avp();
  /**
  free */
  ~cmd_avp();
};

#define FROM_PHONE_LEN    64
#define FROM_NAS_LEN      64
#define FROM_PORT_LEN     64
#define FROM_TIME_LEN     64

/**
@short access from check structure
@version 1
@author rv
*/
class db_user_acc {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  permit from phone */
  string phone_;
  /**
  permit from nas */
  string nas_;
  /**
  permit from ports */
  string port_;
  /**
  permit in time (cron style) */
  string time_;
 public:
  /**
  phone_ */
  string getPhone();
  /**
  set phone */
  void setPhone(const string);
  /**
  nas_ */
  string getNas();
  /**
  set nas */
  void setNas(const string);
  /**
  port_ */
  string getPort();
  /**
  set port */
  void setPort(const string);
  /**
  time_ (cron style) */
  string getTime();
  /**
  set time */
  void setTime(const string);
  /**
  copy */
  void operator =(db_user_acc);
  /**
  compare */
  bool operator ==(db_user_acc);
  /**
  init */
  db_user_acc();
  /**
  free */
  ~db_user_acc();
};

/**
@short store billing user-or-contract resource specific information
@version 1
@author rv
*/
class db_bilres {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  attr */
  string attribute_;
  /**
  value */
  string value_;
 public:
  /**
  attribute_ */
  string getAttribute();
  /**
  set attr */
  void setAttribute(const string);
  /**
  value_ */
  string getValue();
  /**
  set value */
  void setValue(const string);
  /**
  copy */
  void operator =(db_bilres);
  /**
  uncompare :) */
  bool operator !=(db_bilres);
  /**
  compare */
  bool operator ==(db_bilres);
  /**
  init */
  db_bilres();
  /**
  free */
  ~db_bilres();
};

#define DB_USERNAME_LEN     32
#define DB_AUTHSRC_LEN      5
#define DB_PASSWORD_LEN     50
#define DB_ID_LEN           16
#define MAX_AVP             100
#define MAX_ACC             32
#define MAX_BILRES          32

/**
@short db peace - this is for exchange user-associated information
@version 1
@author rv
*/
class db_user {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  username */
  string username_;
  /**
  auth source */
  string authsrc_;
  /**
  password */
  string password_;
  /**
  enable password */
  string enable_;
  /**
  open time */
  time_t open_;
  /**
  close time */
  time_t close_;
  /**
  authorization group */
  string avpid_;
  /**
  access group */
  string accid_;
  /**
  additional group id */
  string addid_;
  /**
  billing identifier */
  string bilid_;
  /**
  billing module identifier */
  int bilmodid_;
  /**
  authorization data */
  list<user_avp> avp_;
  /**
  command authorization data */
  list<cmd_avp> cmd_;
  /**
  access from-list */
  list<db_user_acc> acc_;
  /**
  billing resources list */
  list<db_bilres> bilres_;
  /**
  process result */
  bool result_;
  /**
  connection to database */
  bool connect_;
  /**
  error string for result */
  string result_msg_;
  /**
  maxsess */
  int maxsess_;
 public:
  /**
  init */
  db_user();
  /**
  free */
  ~db_user();
  /**
  get username_ value */
  string getUsername();
  /**
  set username_ value */
  void setUsername(const string);
  /**
  get authsrc_ value */
  string getAuthsrc();
  /**
  set authsrc_ value */
  void setAuthsrc(const string);
  /**
  password_ */
  string getPassword();
  /**
  set password */
  void setPassword(const string);
  /**
  enable_ */
  string getEnable();
  /**
  set enable */
  void setEnable(const string);
  /**
  open_ */
  time_t getOpen();
  /**
  set open */
  void setOpen(time_t);
  /**
  for make different method for unsigned int */
  //void setOpen(unsigned int,int)
  /**
  close_ */
  time_t getClose();
  /**
  set close */
  void setClose(time_t);
  /**
  avpid_ */
  string getAvpid();
  /**
  set avpid */
  void setAvpid(const string);
  /**
  accid_ */
  string getAccid();
  /**
  set accid */
  void setAccid(const string);
  /**
  addid_ */
  string getAddid();
  /**
  set addid */
  void setAddid(const string);
  /**
  bilid_ */
  string getBilid();
  /**
  set bilid */
  void setBilid(const string);
  /**
  bilmodid_ */
  int getBilmodid();
  /**
  set bilmodid */
  void setBilmodid(int);
  /**
  avp_ */
  void avp_add(user_avp);
  /**
  get avp size */
  int avp_size();
  /**
  get avp */
  user_avp avp_get(int);
  /**
  set avp */
  void avp_set(int,user_avp);
  /**
  cmd_ */
  void cmd_add(cmd_avp);
  /**
  get cmd size */
  int cmd_size();
  /**
  get cmd */
  cmd_avp cmd_get(int);
  /**
  acc_ */
  void acc_add(db_user_acc);
  /**
  get acc size */
  int acc_size();
  /**
  get db user */
  db_user_acc acc_get(int);
  /**
  bilres_ */
  void bilres_add(db_bilres);
  /**
  get bilrez size */
  int bilres_size();
  /**
  get billing resources */
  db_bilres bilres_get(int);
  /**
  result_ */
  bool getResult();
  /**
  set res */
  void setResult(bool);
  /**
  connect_ */
  bool getConnect();
  /**
  set conn */
  void setConnect(bool);
  /**
  result_msg_ */
  string getResultMsg();
  /**
  set res msg */
  void setResultMsg(const string);
  /**
  maxsess_ */
  int getMaxsess();
  /**
  set maxs */
  void setMaxsess(int);
  /**
  copy */
  void operator =(db_user);
  /**
  compare */
  bool operator ==(db_user);
  /**
  uncompare :) */
  bool operator !=(db_user);
};

// network device information
#define DB_NAS_LEN              64
#define DESCRIPTION_LEN         50
#define SNMP_MODULE_LEN         100
#define SNMP_COMMUNITY_LEN      30
#define TACACS_KEY_LEN          50
#define LOGINSTRING_LEN         32
#define PWDSTRING_LEN           32
#define DEV_TYPE_LEN            32

/*
@short network device information
*
class db_device {
	DLoad *dl;						// snmp processor module
 public:
	bool result;					// request result
	bool connect;					// connect to database result
	mystring<200> result_msg;		// request result message

	db_device(ipaddr h) {		// constructor
		dl = 0;
		hostaddr = h;
		community<<"public"<<end();
		descr<<"unknown"<<end();
		module<<"none.so"<<end();
		tac_key<<"none"<<end();
		loginstring<<"login:"<<end();
		pwdstring<<"Password:"<<end();
		dev_type<<"host"<<end();
		trfcounter = false;
		snmppoll = false;
		icmppoll = false;
		delay = 60;								// sec
		shutdown = true;					// disabled as default
		def_author = false;				// no permit default authorization - security!
	}
	~db_device() {							// destructor
		if(dl) delete dl;
	}
	// information from config
	int delay;															// polling delay
	string dev_type;				// device type
	string descr;				// description
	string community;	// community
	ipaddr hostaddr;												// hostname/hostaddr
	string module;				// snmp module name
	bool trfcounter;												// store traffic or not
	bool snmppoll;													// poll or not by snmp
	bool icmppoll;													// poll or not by icmp
	bool shutdown;													// working position or not
	string tac_key;				// tac+ key
	string loginstring;	// login req str
	string pwdstring;			// passwd req str
	bool def_author;												// default authorization
	// internal information
	bool status;														// last status (on/off)
	int rest;																// polling delay rest
	char *load() {													// load module
		static char out[80];
		if(dl) delete dl;
		dl = new DLoad;
		mystring<400> str;
//std::cout<<"try to load module "<<module.get()<<std::endl;
		str<<dl->snmpload(module.get())<<end();
		if(strlen(str.c_str())>0) {
			//TLOG(LOG_ERROR,"snmp module: %s",str.c_str());
			// error in load module
			getnas = 0;
			dropnas = 0;
			return str.c_str();
		}
		getnas = (bool (*)(SnmpDevice*,snmp_d*(*)(char*,char*,char*),char*))
				dl->getfunc("getnas");
		dropnas = (bool (*)(SnmpDevice*,char*,snmp_d*(*)(char*,char*,char*),
						bool (*)(char*,char*,char*,char*,char),char*))
							dl->getfunc("dropnas");
		if(getnas == 0 || dropnas == 0) {
			str<<"Unknown snmp module "<<module.get()<<" format"<<end();
			return str.get();
		}
		return "";
	}
	// dynamically loaded functions for snmp processing
	bool (*getnas)(SnmpDevice*,snmp_d*(*)(char*,char*,char*),char*);
	bool (*dropnas)(SnmpDevice*,char*,
		snmp_d*(*)(char*,char*,char*),
		    bool(*)(char*,char*,char*,char*,char),char*);
	void operator =(db_device a) {
		delay = a.delay;
		dev_type = a.dev_type;
		descr = a.descr;
		community = a.community;
		hostaddr = a.hostaddr;
		module = a.module;
		trfcounter = a.trfcounter;
		snmppoll = a.snmppoll;
		icmppoll = a.icmppoll;
		shutdown = a.shutdown;
		tac_key = a.tac_key;
		loginstring = a.loginstring;
		pwdstring = a.pwdstring;
		def_author = a.def_author;
		status = a.status;
		rest = a.rest;
		// we do not load module here - do it manually
	}
};
*/

#define DB_H323_ORIGIN_LEN  15
#define DB_H323_TYPE_LEN    15

/**
@short logging information from nas
@version 1
@author rv
*/
class log_user {
 public:
  /**
  1-start 2-stop 3-update */
  int action;
  /**
  server, which generated entry */
  ipaddr server;
  /**
  access server task_id */
  int task_id;
  /**
  time from server */
  time_t servtime;
  /**
  time from cisco */
  time_t start_time;
  /**
  username */
  string username;
  /**
  from */
  string from;
  /**
  nas port */
  string port;
  /**
  connection elapsed time */
  unsigned int elapsed;
  /**
  client (ppp) ip addr */
  ipaddr ip;
  /**
  nas ip addr */
  ipaddr nas;
  /**
  service (ppp,shell, etc) */
  string service;
  /**
  protocol (ip,ipx,...) */
  string protocol;
  /**
  disconnect cause code */
  int disc_cause;
  /**
  ext disconnect cause code */
  int disc_cause_ext;
  /**
  bytes in to nas */
  unsigned int bytes_in;
  /**
  bytes out from nas */
  unsigned int bytes_out;
  /**
  connection rx speed */
  unsigned int rx_speed;
  /**
  connection tx speed */
  unsigned int tx_speed;
  // VoIP parameters
  /**
  answer/originate */
  string h323_call_origin;
  /**
  call type (Telephony) */
  string h323_call_type;
  /**
  discon */
  int h323_disconnect_cause;
  /**
  voice quality */
  int h323_voice_quality;
  /**
  remote h323 peer IP addr */
  ipaddr h323_remote_address;
  /**
   */
  time_t h323_connect_time;
  /**
   */
  time_t h323_disconnect_time;
  /**
   */
  log_user() {
    server = UNKNOWN_IP;
    nas = UNKNOWN_IP;
    ip = UNKNOWN_IP;
    action = 0;
    servtime   = 0;
    start_time = 0;
    elapsed = 0;
    disc_cause = 0;
    disc_cause_ext = 0;
    bytes_in   = 0;
    bytes_out  = 0;
    rx_speed   = 0;
    tx_speed   = 0;
    task_id    = 0;
    h323_connect_time = 0;
    h323_disconnect_time = 0;
    h323_disconnect_cause = 0;
    h323_voice_quality = 0;
  }
  /**
   */
  ~log_user() {}
};

/**
@short database tables list for store all tables in one list
@version 1
@author rv
*/
class DbList {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  database lists */
  list<DbTable*> dt_;
 public:
  /**
  add database */
  void add(const string,const string,const string);
  /**
  get table */
  DbTable *get(const string);
  /**
  get table */
  DbTable *get(int);
  /**
  init */
  DbList();
  /**
  free */
  ~DbList();
  /**
  ret size */
  int size();
};

/**
@short DbModule functions
*/
//class DbModule : public md5 {
class DbModule {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  connection ID */
  //int cid_;
  /**
  database name */
  string dbname_;
  /**
  database location */
  ipaddr location_;
  /**
  database port */
  unsigned int dbport_;
  /**
  database key */
  string cryptokey_;
  /**
  database username */
  string username_;
  /**
  database password */
  string password_;
  /**
  database module */
  string module_;
  /**
  status */
  bool shutdown_;
 public:
  /**
  location_ */
  void setLocation(ipaddr);
  /**
  get locat */
  ipaddr getLocation();
  /**
  dbname_ */
  void setDbname(string);
  /**
  get db name */
  string getDbname();
  /**
  dbport_ */
  void setDbport(unsigned int);
  /**
  get db port */
  unsigned int getDbport();
  /**
  cryptokey_ */
  void setCryptokey(string);
  /**
  get c key */
  string getCryptokey();
  /**
  username_ */
  void setUsername(const string);
  /**
  get usnam */
  string getUsername();
  /**
  password_ */
  void setPassword(const string);
  /**
  get pwd */
  string getPassword();
  /**
  module_ */
  void setModule(const string);
  /**
  get module */
  string getModule();
  /**
  shutdown_ */
  void setShutdown(bool);
  /**
  get shutd */
  bool getShutdown();
  /**
  set database engine id */
  void setdbid(const char*,const char*,int);
  /**
  get db engine id string */
  string getdbid();
  /**
  data encryption */
  string encrypt(char*,char*);
  /**
  data decryption */
  string decrypt(char*,char*);
  /**
  create hash */
  void create_md5_hash(unsigned char*, unsigned char*);
  /**
  do xor */
  void md5_xor(unsigned char*,unsigned char*);

  // pointers to functions for dynamic loading

  /**
  init database driver */
  void (*dl_init)();
  /**
  destroy database driver */
  void (*dl_destroy)();
  /**
  connect to server */
  int (*dl_open)(char*,int,char*,char*,char*,char*);
  /**
  disconnect from server */
  bool (*dl_close)(int);
  /**
  send query */
  bool (*dl_query)(int,DbTable*,char*);
  /**
  function query */
  char* (*dl_function)(int,char*,char*);
  /**
  get data */
  bool (*dl_get)(int,DbTable*,int,char*);
  /**
  create table */
  bool (*dl_create)(int,DbTable*,char*);
  /**
  create table index */
  bool (*dl_create_idx)(int,DbTable*,char*);
  /**
  insert */
  bool (*dl_add)(int,DbTable*,char*);
  /**
  update */
  bool (*dl_update)(int,DbTable*,char*);
  /**
  delete */
  bool (*dl_del)(int,DbTable*,char*);
  /**
  we call to Db constructor in body */
  DbModule(const char*,const char*,const char*,int,const char*,const char*,const char*);
  /**
  connect to */
  bool db_open(int*);
  /**
  disconnect from */
  bool db_close(int);
  /**
  send query */
  bool db_query(int,DbTable*);
  /**
  send query to function */
  string db_function(int,char*);
  /**
  get data */
  bool db_get(int,DbTable*,int);
  /**
  create table */
  bool db_create(int,DbTable*,char*);
  /**
  create table index */
  bool db_create_idx(int,DbTable*,char*);
  /**
  insert to table */
  bool db_add(int,DbTable*);
  /**
  update */
  bool db_update(int,DbTable*);
  /**
  delete from table */
  bool db_del(int,DbTable*);
  /**
  free */
  ~DbModule();
};

/**
@short tacppd data database support
@version 1
@author rv
*/
class TacDb : public DbModule {
  /**
  thread locker */
  pthread_mutex_t Mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  our host ip */
  ipaddr our_ip_;
 public:
  /**
  for dynaloader */
  DLoad *dl;
  /**
  init */
  TacDb(const char*,const char*,const char*,int,const char*,const char*,const char*);
  /**
  free */
  ~TacDb();
  /**
  get user info */
  db_user get(const string);
  /**
  size */
  int size();
  /**
  direct query to database */
  string direct_function_query(char*);
  /**
  load */
  string load();
  /**
  create table in database */
  string create();

  // universal manipulation functions
  /**
  del ent */
  string del(string,list<string>,list<string>);
  /**
  show data */
  string show(string,list<string>,list<string>);
  /**
  small/stripped show */
  list<string> sshow(string,list<string>,list<string>);
  /**
  add data */
  string add(string,list<string>,list<string>);
  /**
  mod data */
  string modify(string,list<string>,list<string>,list<string>,list<string>);
  /**
  add log information to file and to database */
  string add_log(log_user*);
  /**
  add traffic information from SNMP interface counters to database */
  string add_trf(char*,SnmpOut*);
  /**
  open database connect for netflow data */
  int open_nf();
  /**
  close database after netflow data */
  void close_nf(int);
  /**
  add traffic information from netflow aggregator */
  string add_nf(int,Bit32,Bit32,Bit32,time_t,time_t,unsigned long,unsigned long,int);//NfData*);
  /**
  get table name */
  string get_table_name(int);
  /**
  get table mask */
  string get_table_mask(int);
  /**
  get table description */
  string get_table_descr(int);
  /**
  get field name */
  string get_field_name(int,int);
  /**
  get field descr */
  string get_field_descr(int,int);
  /**
  get field def value */
  string get_field_defv(int,int);
  /**
  get field type */
  int get_field_type(int,int);
  /**
  get field number */
  int get_field_num(int);
};

};

#endif //__TACDB_H__
