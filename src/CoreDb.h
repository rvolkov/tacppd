// tacppd database system support
// (c) Copyright 2000-2007 by Roman Volkov and contributors
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

#ifndef __CORE_DB_H__
#define __CORE_DB_H__

#include "global.h"

namespace tacpp {

////////////////////////////////////////////
//
// databases list
//

// data fragment big, and it outside

/**
@short database manager, use md5 for crypt/encrypt passwords field
@version 1
@author rv
*/
//class CoreDb : public md5, public Core_data {
class CoreDb : public Core_data {
  /**
  databases list */
  list<TacDb*> tdb_;
  /**
  thread locker */
  pthread_mutex_t Mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
 public:
  /**
  init data */
  CoreDb(const string,const string);
  /**
  destroy data */
  ~CoreDb();
  /**
  get information from database
  @return user data */
  db_user get(const string);
  /**
  direct query to server */
  string direct_function_query(const char*);
  /**
  get database table name */
  string get_table_name(const int);
  /**
  get database table mask */
  string get_table_mask(const int);
  /**
  get db table description */
  string get_table_descr(const int);
  /**
  get field name */
  string get_field_name(const int,const int);
  /**
  get field descr */
  string get_field_descr(const int,const int);
  /**
  get field def value */
  string get_field_defv(const int,const int);
  /**
  get field type */
  int get_field_type(const int,const int);
  /**
  get field num */
  int get_field_num(const int);
  /**
  add database to list */
  void add(const char*,unsigned int,const char*);
  /**
  delete database from list */
  void del(const char*,unsigned int,const char*);
  /**
  reload module */
  string load(const char*,unsigned int,const char*);
  /**
  get module name */
  string getmodulename(const char*,unsigned int,const char*);
  /**
  set/modify module */
  void modify_module(const char*,unsigned int,const char*,const char*);
  /**
  set/modify cryptokey */
  void modify2(const char*,unsigned int,const char*,const char*);
  /**
  set/modify username */
  void modify4(const char*,unsigned int,const char*,const char*);
  /**
  set/modify password */
  void modify5(const char*,unsigned int,const char*,const char*);
  /**
  set/mod shutdown flag */
  void modify_shutdown(const char*,unsigned int,const char*,bool);
  /**
  build configuration */
  string buildcfstr(int);
  /**
  apply config */
  string applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd=NULL)throw (CmdErr);
  /**
  create datatables in all databases */
  string create_all();
  /**
  delete data from table */
  string del_data(string,list<string>,list<string>);
  /**
  show data in table */
  string show_data(string,list<string>,list<string>);
  /**
  show stripped data in table */
  list<string> sshow_data(string,list<string>,list<string>);
  /**
  add data to table */
  string add_data(string,list<string>,list<string>);
  /**
  modify data in table */
  string modify_data(string,list<string>,list<string>,list<string>,list<string>);
  /**
  write log information to all databases log table */
  void add_log(log_user*);
  /**
  add traffic info from snmp counters */
  //void add_trf();
  /**
  open process nf packet */
  int open_nf();
  /**
  close process nf packet */
  void close_nf(int);
  /**
  add nf data */
  void add_nf(int,Bit32,Bit32,Bit32,time_t,time_t,unsigned long,unsigned long,int);//NfData*);
};

};

#endif // __CORE_DB_H__
