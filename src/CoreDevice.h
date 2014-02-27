// tacppd network device related information
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// all comments which start from /** - this is comments for KDoc
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

#ifndef __CORE_DEVICE_H__
#define __CORE_DEVICE_H__

#include "global.h"

#define DESCRIPTION_LEN       50
#define SNMP_MODULE_LEN       100
#define SNMP_COMMUNITY_LEN    30
#define TACACS_KEY_LEN        50
#define RADIUS_KEY_LEN        50
#define LOGINSTRING_LEN       32
#define PWDSTRING_LEN         32
//#define AUTHENSOURCE_LEN    10

namespace tacpp {

/**
@short entry about one active user on device
@author rv
@version 1
*/
class DeviceUser_data {
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
  active user or not */
  bool present_;
  /**
  device user username */
  string username_;
  /**
  device port name */
  string port_;
  /**
  from information (phone number) */
  string phone_;
  /**
  ip address of active user */
  ipaddr useraddr_;
  /**
  active flag - true-active, false-inactive */
  bool active_;
  /**
  log information from accounting entries */
  log_user log_;
  /**
  time of last data comes */
  time_t lastdatatime_;
  /**
  nas ip addr */
  ipaddr nasaddr_;
 public:
  /**
  store information */
  DeviceUser_data(const string,const string,const string,const ipaddr,
    const ipaddr,const db_user);
  /**
  set default information */
  DeviceUser_data();
  /**
  free user information */
  ~DeviceUser_data();
  /**
  update user information */
  void update(const string,const string,const ipaddr);
  /**
  update last time of entry */
  void update();
  /**
  set active flag */
  void setActive();
  /**
  is this user active? */
  bool isActive();
  /**
  is this user expired */
  bool isExpired();
  /**
  set present flag */
  void setPresent(const bool);
  /**
  get present flag */
  bool isPresent();
  /**
  get username information */
  string getUsername();
  /**
  compare username with this */
  bool compareUsername(const string);
  /**
  get port information */
  string getPort();
  /**
  compare portnames */
  bool comparePort(const string);
  /**
  get from phone information */
  string getPhone();
  /**
  get user ip addr information */
  ipaddr getUserAddr();
  /**
  get database data stored inside */
  db_user getDbData();
  /**
  get network device address */
  ipaddr getNasAddr();
  /**
  compare device addresses */
  bool compareNasAddr(const ipaddr);
  /**
  is permitted user work time ? */
  bool match_time(const time_t);
  /**
  copy constructor */
  void operator =(DeviceUser_data);
};

/**
@short objects for network devices
@version 1
@author rv
*/
class Device_data {
  /**
  snmp module functions for this device */
  DLoad *dl_;
  /**
  polling delay (config) */
  int delay_;
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
  device description (config) */
  string descr_;
  /**
  network device community (config) */
  string community_;
  /**
  users on this device */
  list<DeviceUser_data*> cusr_;
  /**
  device name/address */
  ipaddr hostaddr_;
  /**
  snmp device module name */
  string module_;
  /**
  store per-interface snmp traffic counters or not */
  bool counter_;
  /**
  set device as active or close it */
  bool shutdown_;
  /**
  tacacs+ key (config) */
  string tac_key_;
  /**
  login request string (config) */
  string loginstring_;
  /**
  password request string (config) */
  string pwdstring_;
  /**
  enable/disable default authorization (config) */
  bool def_author_;
  /**
  permit/deny snmp polling (config) */
  bool snmppolling_;
  /**
  permit/deny icmp polling (config) */
  bool icmppolling_;
  /**
  device status from polling - on/off (internal) */
  bool status_;
  /**
  polling delay wait in seconds (internal) */
  int rest_;
 public:
  /**
  set data for device */
  Device_data(const ipaddr);
  /**
  set data by default */
  Device_data();
  /**
  destructor */
  ~Device_data();
  /**
  det DeviceUser_data */
  DeviceUser_data gets(const int);
  /**
  get DeviceUser_data if ipaddr match with user ip addr (for nf log) */
  DeviceUser_data gets(ipaddr);
  /**
  erase entry */
//  DeviceUser_data *erase(const int);
  /**
  erase entry with matched data */
//  DeviceUser_data *erase(const string,const string,const ipaddr);
  /**
  add entry */
  bool add(DeviceUser_data*);
  /**
  delete expired entryes */
  void usr_expire();
  /**
  delete user from port */
  bool usr_del(const string);
  /**
  delete all users from device */
  void usr_del();
  /**
  activate user or delete if something mismatch */
  bool usr_activate(const string,const string,const string,const ipaddr);
  /**
  add user */
  bool usr_add(const ipaddr,const string,const string,const string,const ipaddr,const db_user);
  /**
  periodic checking */
  void usr_periodic(const time_t);
  /**
  numsess on our device */
  int numsess(const string);
  /**
  cusr_ size */
  int size();
  /**
  get polling delay value */
  int getDelay();
  /**
  set polling delay value */
  void setDelay(const int);
  /**
  set device description */
  void setDescr(const string);
  /**
  get device description */
  string getDescr();
  /**
  set network device community */
  void setCommunity(const string);
  /**
  get network device community */
  string getCommunity();
  /**
  get device name/address */
  ipaddr getHostaddr();
  /**
  compare this ip addr with given
  @return true if equal */
  bool compareHostaddr(const ipaddr);
  /**
  set device name/address */
  void setHostaddr(const ipaddr);
  /**
  set device module name */
  void setModule(const string);
  /**
  get device module name */
  string getModule();
  /**
  set store per-interface snmp traffic counters or not */
  void setCounter(const bool);
  /**
  get store per-interface snmp traffic counters or not */
  bool getCounter();
  /**
  set device as active or close it (config) */
  void setShutdown(const bool);
  /**
   */
  bool getShutdown();
  /**
  tacacs+ key (config) */
  string getTacKey();
  /**
  set tacacs+ key */
  void setTacKey(const string);
  /**
  login request string (config) */
  string getLoginstring();
  /**
   */
  void setLoginstring(const string);
  /**
  password request string (config) */
  string getPwdstring();
  /**
  store pwd str */
  void setPwdstring(const string);
  /**
  enable/disable default authorization (config) */
  bool getDefAuthor();
  /**
  set default author value */
  void setDefAuthor(const bool);
  /**
  permit/deny snmp polling (config) */
  bool getSnmppolling();
  /**
  set snmp polling */
  void setSnmppolling(const bool);
  /**
  permit/deny icmp polling (config) */
  bool getIcmppolling();
  /**
  set icmp polling */
  void setIcmppolling(const bool);
  /**
  device status from polling - on/off (internal) */
  bool getStatus();
  /**
  set dev stat */
  void setStatus(const bool);
  /**
  polling delay wait in seconds (internal) */
  int getRest();
  /**
  set */
  void setRest(const int);
  /**
  load module */
  string load();
  /**
  dynamically loaded function - process snmp information for device */
  bool (*getnas)(SnmpDevice*,SnmpD*(*)(char*,char*,char*),char*);
  /**
  dynamically loaded function - drop user from device port */
  bool (*dropnas)(SnmpDevice*,char*,
    SnmpD*(*)(char*,char*,char*),
        bool(*)(char*,char*,char*,char*,char),char*);
  /**
  returns configuration strings for this device */
  string buildcfstr();
};

/**
@short network devices manager
@author rv
@see Core_data Device_data
@version 1
*/
class CoreDevice : public Core_data {
  /**
  thread id for periodic polling process */
  pthread_t thrvar_;
  /**
  thread locker */
  pthread_mutex_t Mutex_;
  /**
  set thread lock */
  void lock();
  /**
  remove thread lock */
  void unlock();
  /**
  get object by ipaddr without locking  */
  Device_data *get_(const ipaddr);
  /**
  network devices list */
  list<Device_data*> nas;
  /**
  flag which indicate run state of periodic snmp polling process */
  bool runPeriodic_;
 public:
  /**
  init */
  CoreDevice(const string,const string);
  /**
  free */
  ~CoreDevice();
  /**
  get object by ipaddr with locking  */
  Device_data *get(const ipaddr);
  /**
  add device */
  void add(const ipaddr);
  /**
  delete device */
  void del(const ipaddr);
  /**
  expire and delete expired users */
  void usr_expire();
  /**
  delete user from list by nas and port
  @return if user was deleted */
  bool usr_del(const ipaddr,const string);
  /**
  delete all users from device */
  void usr_del(const ipaddr);
  /**
  activate user on device */
  bool usr_activate(const ipaddr,const string,const string,const string,const ipaddr);
  /**
  add user with data
  @return true if user added */
  bool usr_add(const ipaddr,const string,const string,const string,const ipaddr,const db_user);
  /**
  periodic user checking */
  void usr_periodic(const time_t);
  /**
  number of user sessions */
  int numsess(const string);
  /**
  run periodic polling thread */
  void startPeriodic();
  /**
  set community */
  void modifyCommunity(const ipaddr,const string);
  /**
  set/unset interface traffic counter */
  void modifyCounter(const ipaddr,const bool);
  /**
  set/modify device polling delay */
  void modifyDelay(ipaddr,int);
  /**
  set/modify device description */
  void modifyDescr(ipaddr,const string);
  /**
  set/modify device snmp module */
  void modifyModule(ipaddr,const string);
  /**
  set/modify tacacs+ key */
  void modifyKey(ipaddr,const string);
  /**
  set/modify login request string */
  void modifyLogin(ipaddr,const string);
  /**
  set/modify pwd request string */
  void modifyPwd(ipaddr,const string);
  /**
  set/modify default authorization */
  void modifyDef(ipaddr,bool);
  /**
  set/modify device shutdown or no */
  void modifyShut(ipaddr,bool);
  /**
  enable/disable snmp polling */
  void modifySnmp(ipaddr,bool);
  /**
  enable/disable icmp polling */
  void modifyIcmp(ipaddr,bool);
  /**
  for snmp polling, get object by number */
  Device_data *get(int);
  /**
  get device ip by number */
  ipaddr getip(int);
  /**
  get user by number */
  DeviceUser_data getuser(int);
  /**
  get user by device ip and number */
  DeviceUser_data getuser(ipaddr,int);
  /**
  get username from known ipv4 addr for nf */
  DeviceUser_data getuser4ip(ipaddr);
  /**
  load/reload module */
  string load(const ipaddr);
  /**
  returns snmp module name by device ip */
  string getmodulename(const ipaddr);
  /**
  apply config */
  string applycf(char * const*mask,const string cmdname,const list<string>paramList,Command *cmd=NULL)throw (CmdErr);
  /**
  build config for one object by number */
  string buildcfstr(int);
  /**
  returns number of device objects */
  int size();
  /**
  set device status */
  void setDeviceStatus(ipaddr,bool);
  /**
  get device status */
  bool get_device_status(ipaddr);
  /**
  snmp community */
  string getCommunity(ipaddr);
  /**
   */
  void setRunPeriodic(bool);
  /**
   */
  bool getRunPeriodic();
};

};

#endif // __CORE_NAS_H__
