// MySQL support module
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#ifdef DB_MYSQL

#ifndef _PTHREADS
#define   _PTHREADS   // enable STL pthread-safe code
#endif //_PTHREADS

#ifndef _REENTRANT
#define       _REENTRANT    // Pthread safe stuff in the stdlibs
#endif //_REENTRANT     // or you should compile with -pthread key

#include "ipaddr.h"
#include "db_module.h"
#include <cstdlib>
#include <mysql.h>
#include <iostream>

extern "C" {
  void db_init(); // init database
  void db_destroy(); // destroy database data
  int db_open(char*,int,char*,char*,char*,char*); // open database
  bool db_close(int);                     // close database
  bool db_query(int,DbTable*,char*);      // send query
  char *db_function(int,char*,char*);     // request SQL function (only for SQL!)
  bool db_get(int,DbTable*,int,char*);    // get information
  bool db_create(int,DbTable*,char*);     // create table
  bool db_create_idx(int,DbTable*,char*); // create table idx
  bool db_add(int,DbTable*,char*);        // add information to db table
  bool db_update(int,DbTable*,char*);     // update db table
  bool db_del(int,DbTable*,char*);        // delete entry from database
}

// connection information (database engine specific)
struct connection_struct {
  int num; // connect id
  MYSQL *SQL_conn;
  MYSQL_RES *SQL_res;
};

list<connection_struct*> con; // list of open connections

pthread_mutex_t mutex_; // thread locker

// convert time() to ISO datetime format (which is default for mysql)
string
time2iso(time_t t) {
  struct tm *ttt;
  struct tm res;
  ttt = localtime_r(&t,&res);
  char str[64];
  bzero(str,sizeof(str));
  snprintf(str,sizeof(str)-1,"%04d-%02d-%02d %02d:%02d:%02d",
    1900+ttt->tm_year,ttt->tm_mon+1,ttt->tm_mday,
      ttt->tm_hour,ttt->tm_min,ttt->tm_sec);
  string sout = str;
  return sout;
}

// convert ISO datetime format to time() value
time_t
iso2time(char *iso) {
  char str[20];
  struct tm ttt;

  // read year
  int k = 0;
  for(int i=0; i<4; i++,k++) str[k] = iso[i];
  str[k] = 0;
  ttt.tm_year = atoi(str) - 1900;

  // read month
  k = 0;
  for(int i=5; i<7; i++,k++) str[k] = iso[i];
  str[k] = 0;
  ttt.tm_mon = atoi(str) - 1;

  // read day
  k = 0;
  for(int i=8; i<10; i++,k++) str[k] = iso[i];
  str[k] = 0;
  ttt.tm_mday = atoi(str);

  // read hour
  k = 0;
  for(int i=11; i<13; i++,k++) str[k] = iso[i];
  str[k] = 0;
  ttt.tm_hour = atoi(str);

  // read minutes
  k = 0;
  for(int i=14; i<16; i++,k++) str[k] = iso[i];
  str[k] = 0;
  ttt.tm_min = atoi(str);

  // read seconds
  k = 0;
  for(int i=17; i<19; i++,k++) str[k] = iso[i];
  str[k] = 0;
  ttt.tm_sec = atoi(str);

  return mktime(&ttt);
}

void
lock() {
  pthread_mutex_lock(&mutex_);
}

void
unlock() {
  pthread_mutex_unlock(&mutex_);
}

void
db_init() {
  pthread_mutex_init(&mutex_,0);
}

void
db_destroy() {
  pthread_mutex_destroy(&mutex_);
}

// disconnect from server
bool
db_close(int num) {
  lock();
  connection_struct *ctmp = 0;
  bool res = false;
  for(list<connection_struct*>::iterator i=con.begin();i!=con.end();i++) {
    ctmp = *i;
    if(ctmp->num == num) {
      res = true;
      if(ctmp->SQL_res) mysql_free_result(ctmp->SQL_res);
      if(ctmp->SQL_conn) mysql_close(ctmp->SQL_conn);
      i = con.erase(i);
      delete ctmp;
      break;
    }
  }
  unlock();
  return res;
}

//  connection to SQL server...
//  false - reject, true - success
int
db_open(char *host,int port,char *dbase,char *login,char *pwd,char *derr) {
  int num = 1;    // connection id
  lock();
  connection_struct *tmp;
  for(list<connection_struct*>::iterator i=con.begin();i!=con.end();i++) {
    tmp = *i;
    if(tmp->num == num) {
      num++;
      i = con.begin();
    }
  }
  tmp = new connection_struct;
  tmp->num = num;
  tmp->SQL_res = 0;
  tmp->SQL_conn = 0;

  tmp->SQL_conn = mysql_init(0);
  if(!mysql_real_connect(tmp->SQL_conn,host,login,pwd,dbase,port,NULL,0)) {
    snprintf(derr,DB_MODULE_ERRLEN,"Connection to database %s failed: %s",
      dbase, mysql_error(tmp->SQL_conn));
    mysql_close(tmp->SQL_conn); // free
    delete tmp; // free
    unlock();
    return 0;
  }
  con.push_back(tmp); // store connection in list
  unlock();
  return num;
}

//  send query to SQL server
//  true - success, false - reject
MYSQL_RES*
sdb_query(MYSQL *SQL_conn,char *query,char *derr) {
  MYSQL_RES *SQL_res = 0;
  if(mysql_query(SQL_conn,query)) {
    snprintf(derr,DB_MODULE_ERRLEN,"%s",mysql_error(SQL_conn));
    return 0;
  }
  if(!(SQL_res=mysql_store_result(SQL_conn))) {
    snprintf(derr,DB_MODULE_ERRLEN,"cant get result (%s)",mysql_error(SQL_conn));
    return 0;
  }
  return SQL_res;
}

//  getting data
// return data if success
// NULL - othervise
string
sdb_get(MYSQL *SQL_conn,MYSQL_RES *SQL_res,int i,int j) {
  //
  MYSQL_ROW row = 0;
  char str[300];
  string sout;
  int r = mysql_num_rows(SQL_res);
  if(i > r) return sout; // wrong parameter

  // next code changed due to Mjl's investigation
  // from
//  for(int k=0; k <= i; k++)
//    row = mysql_fetch_row(SQL_res);
  // to
  mysql_data_seek(SQL_res,(unsigned long) i);
  row = mysql_fetch_row(SQL_res);
  // it looks like much better and no buggy, thanks!

  strncpy(str,row[j],sizeof(str)-1);
  sout = str;
  return sout;
}

// number of rows in response
int
sdb_rows(MYSQL *SQL_conn,MYSQL_RES *SQL_res) {
  return(mysql_num_rows(SQL_res));
}

// send query to database
bool
db_query(int num, DbTable *d, char *derr) {
  lock();
  connection_struct *ctmp = 0;
  bool res = false;
  for(list<connection_struct*>::iterator i=con.begin();i!=con.end();i++) {
    ctmp = *i;
    if(ctmp->num == num) {
      res = true;
      break;
    }
  }
  if(!res) {
    snprintf(derr,DB_MODULE_ERRLEN,"No connection open");
    unlock();
    return false;
  }
  string fff;
  string ppp;
  // create list of required fields
  //bzero(fff,sizeof(fff));
  for(int i=0; i<d->size(); i++) {
    if(!d->get_request(i)) { // if this is not request
      fff = fff + d->get_name(i);
      fff = fff + ",";
    }
  }
  fff.erase(fff.size()-1,2); // drop last comma
  // create list of known fields for request
  char tmp[100];
  bool bool_type = false;
  int int_type = 0;
  float float_type = 0;
  char string_type[300];
  time_t date_type = 0;
  ipaddr ipaddr_type;
  char name[100];
  for(int i=0; i<d->size(); i++) {
    if(d->get_request(i)) { // if this is request
      snprintf(name,sizeof(name)-1,"%s",d->get_name(i).c_str());
      ppp = ppp + name; // name
      ppp = ppp + "="; // only '='
      switch(d->get_type(i)) {
       case 1: // bool type
        d->get(name,&bool_type);
        if(bool_type) ppp = ppp + "true";
        else ppp = ppp + "false";
        break;
       case 2: // int type
        d->get(name,&int_type);
        snprintf(tmp,sizeof(tmp)-1,"%u",(unsigned int)int_type);
        ppp = ppp + tmp;
        break;
       case 3: // float type
        d->get(name,&float_type);
        snprintf(tmp,sizeof(tmp)-1,"%f",float_type);
        ppp = ppp + tmp;
        break;
       case 4: // string type (varchar)
        bzero(string_type,sizeof(string_type));
        d->get(name,string_type);
        string_type[sizeof(tmp)-3] = 0;
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",string_type);
        ppp = ppp + tmp;
        break;
       case 5: // date type (datetime)
        d->get(name,&date_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",time2iso(date_type).c_str());
        ppp = ppp + tmp;
        break;
       case 6: // ipaddr type
        d->get(name,&ipaddr_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",ipaddr_type.get().c_str());
        ppp = ppp + tmp;
        break;
       default:
        // error - unknown field
        break;
      }
      ppp = ppp + " AND "; // we do not support OR
    }
  }
  if(ppp.size() > 5) ppp.erase(ppp.size()-5,5); // drop last ' AND '
  // create SQL request
  string req;
  if(ppp.size()>0)
    req = req+"SELECT "+fff+" FROM "+d->getName()+" WHERE "+ppp;
  else
    req = req+"SELECT "+fff+" FROM "+d->getName();
  // clear previous data

  // do request
  ctmp->SQL_res = sdb_query(ctmp->SQL_conn,(char*)req.c_str(),derr);
  if(ctmp->SQL_res) {
    unlock();
    return true;
  } else {
    unlock();
    return false;
  }
}

// send function query to database
char*
db_function(int num, char *func, char *derr) {
  lock();
  connection_struct *ctmp = 0;
  bool res = false;
  for(list<connection_struct*>::iterator i=con.begin();i!=con.end();i++) {
    ctmp = *i;
    if(ctmp->num == num) {
      res = true;
      break;
    }
  }
  if(!res) {
    snprintf(derr,DB_MODULE_ERRLEN,"No connection open");
    unlock();
    return 0;
  }
  // create SQL request
  char req[400];
  snprintf(req,sizeof(req)-1,"SELECT %s", func);
  // do request
  char *aaaa = new char[200];
  ctmp->SQL_res = sdb_query(ctmp->SQL_conn,req,derr);
  if(ctmp->SQL_res) {
    if(sdb_rows(ctmp->SQL_conn,ctmp->SQL_res) == 0) {
      unlock();
      return 0;
    }
    snprintf(aaaa,200-1,"%s",sdb_get(ctmp->SQL_conn,ctmp->SQL_res,0,0).c_str());
//printf("db_function req=%s result=%s\n",req,aaaa);
    unlock();
    return aaaa;
  } else {
    unlock();
    return 0;
  }
}

// get information from database to DbTable list
bool
db_get(int num,DbTable *d,int row,char *derr) {
  lock();
  connection_struct *ctmp = 0;
  bool res = false;
  for(list<connection_struct*>::iterator i=con.begin();i!=con.end();i++) {
    ctmp = *i;
    if(ctmp->num == num) {
      res = true;
      break;
    }
  }
  if(!res) {
    snprintf(derr,DB_MODULE_ERRLEN,"No connection open");
    unlock();
    return false;
  }
  if(sdb_rows(ctmp->SQL_conn,ctmp->SQL_res) <= row) {
    unlock();
    return false;
  }
  int k=0;
  char name[100];
  for(int i=0; i<d->size(); i++) {
    if(!d->get_request(i)) { // if this is not request
      snprintf(name,sizeof(name)-1,"%s",d->get_name(i).c_str());
      switch(d->get_type(i)) {
       case 1:  // bool type
        if(sdb_get(ctmp->SQL_conn,ctmp->SQL_res,row,k).c_str()[0] == 't')
          d->set(name,true);
        else
          d->set(name,false);
        break;
       case 2:  // int type
        d->set(name,atoi(sdb_get(ctmp->SQL_conn,
          ctmp->SQL_res,row,k).c_str()));
        break;
       case 3:  // float type
        d->set(name,(float)atof(sdb_get(ctmp->SQL_conn,
          ctmp->SQL_res,row,k).c_str()));
        break;
       case 4:  // string type
        d->set(name,sdb_get(ctmp->SQL_conn,ctmp->SQL_res,row,k));
        break;
       case 5:  // date type
        d->set(name,
          iso2time((char*)sdb_get(ctmp->SQL_conn,ctmp->SQL_res,row,k).c_str()));
        break;
       case 6:  // ipaddr type
        d->set(name,
          ipaddr((char*)sdb_get(ctmp->SQL_conn,ctmp->SQL_res,row,k).c_str()));
        break;
       default:
        // error
        break;
      }
      k++;
    }
  }
  unlock();
  return true;
}

// create table
bool
db_create(int num,DbTable *d, char *derr) {
  lock();
  connection_struct *ctmp = 0;
  bool res = false;
  for(list<connection_struct*>::iterator i=con.begin();i!=con.end();i++) {
    ctmp = *i;
    if(ctmp->num == num) {
      res = true;
      break;
    }
  }
  if(!res) {
    snprintf(derr,DB_MODULE_ERRLEN,"No connection open");
    unlock();
    return false;
  }
  string fff;
  char tmp[90];
  // create list of required fields
//  char string_type[300];
//  bzero(string_type,sizeof(string_type));
  char name[100];

  for(int i=0; i<d->size(); i++) {
    snprintf(name,sizeof(name)-1,"%s",d->get_name(i).c_str());
    fff = fff + name;
    switch(d->get_type(i)) {
     case 1:  // bool
      fff = fff + " bool";
      if(d->get_defv(i).size() > 0) {
        fff = fff + " DEFAULT ";
        if(d->get_defv(i).c_str()[0]=='t')
          fff = fff + "true";
        else
          fff = fff + "false";
      }
      fff = fff + ",";
      break;
     case 2:  // int
      fff = fff + " int";
      if(d->get_defv(i).size() > 0) {
        fff = fff + " DEFAULT ";
        fff = fff + d->get_defv(i);
      }
      fff = fff + ",";
      break;
     case 3:  // float
      fff = fff + " float";
      if(d->get_defv(i).size() > 0) {
        fff = fff + " DEFAULT ";
        fff = fff + d->get_defv(i);
      }
      fff = fff + ",";
      break;
     case 4:  // string
      snprintf(tmp,sizeof(tmp)-1," varchar(%d)",
        d->getslen(name));
      fff = fff + tmp;
      if(d->get_defv(i).size() > 0) {
        fff = fff + " DEFAULT ";
        fff = fff + "\'";
        fff = fff + d->get_defv(i);
        fff = fff + "\'";
      }
      fff = fff + ",";
      break;
     case 5:  // datetime
      fff = fff + " datetime,";
//      fff = fff + " timestamp,";
      break;
     case 6:  // ipaddr
      fff = fff + " varchar(16),";
      break;
     default:
      // error
      break;
    }
  }
  fff.erase(fff.size()-1,2); // drop last comma
  string req;
  req = req + "CREATE TABLE " + d->getName() + "(" + fff + ")";
//printf("CREATE: %s\n",req.c_str());
  ctmp->SQL_res = sdb_query(ctmp->SQL_conn,(char*)req.c_str(),derr);
  if(ctmp->SQL_res) {
    mysql_free_result(ctmp->SQL_res);
    ctmp->SQL_res = 0;
    unlock();
    return true;
  } else {
    unlock();
    return false;
  }
}

// create table index
bool
db_create_idx(int num, DbTable *d, char *derr) {
  lock();
  connection_struct *ctmp = 0;
  bool res = false;
  for(list<connection_struct*>::iterator i=con.begin();i!=con.end();i++) {
    ctmp = *i;
    if(ctmp->num == num) {
      res = true;
      break;
    }
  }
  if(!res) {
    snprintf(derr,DB_MODULE_ERRLEN,"No connection open");
    unlock();
    return false;
  }
  string req1;
  string req2;
  string fff1;
  string fff2;

  // create list of required fields
  for(int i=0; i<d->size(); i++) {
    if(d->get_idx(i) == T_UNIQUE) {
      fff1 = fff1 + d->get_name(i);
      fff1 = fff1 + ",";
    }
    if(d->get_idx(i) == T_INDEX) {
      fff2 = fff2 + d->get_name(i);
      fff2 = fff2 + ",";
    }
  }
  if(fff1.size() > 0) {
    fff1.erase(fff1.size()-1,2); // drop last comma
    req1=req1+"CREATE UNIQUE INDEX "+d->getName()+"_idx ON "+d->getName()+"("+fff1+")";
//cerr<<"CREATE UNI: "<<req1<<endl;
    ctmp->SQL_res = sdb_query(ctmp->SQL_conn,(char*)req1.c_str(),derr);
    if(!ctmp->SQL_res) {
      unlock();
      return false;
    }
    mysql_free_result(ctmp->SQL_res);
    ctmp->SQL_res = 0;
  }
  if(fff2.size() > 0) {
    fff2.erase(fff2.size()-1,2); // drop last comma
    req2=req2+"CREATE INDEX "+d->getName()+"_idx ON "+d->getName()+"("+fff2+")";
//cerr<<"CREATE IDX: "<<req2<<endl;
    ctmp->SQL_res = sdb_query(ctmp->SQL_conn,(char*)req2.c_str(),derr);
    if(!ctmp->SQL_res) {
      unlock();
      return false;
    }
    mysql_free_result(ctmp->SQL_res);
    ctmp->SQL_res = 0;
  }
  unlock();
  return true;
}

// insert entry to database/table
bool
db_add(int num, DbTable *d, char *derr) {
  lock();
  connection_struct *ctmp = 0;
  bool res = false;
  for(list<connection_struct*>::iterator i=con.begin();i!=con.end();i++) {
    ctmp = *i;
    if(ctmp->num == num) {
      res = true;
      break;
    }
  }
  if(!res) {	
    snprintf(derr,DB_MODULE_ERRLEN,"No connection open");
    unlock();
    return false;
  }
  string req;
  string fff;
  string vvv;
  char tmp[90];

  bool bool_type = false;
  int int_type = 0;
  float float_type = 0;
  char string_type[300];
  time_t date_type = 0;
  ipaddr ipaddr_type;
  char name[100];

  for(int i=0; i<d->size(); i++) {
    snprintf(name,sizeof(name)-1,"%s",d->get_name(i).c_str());
    snprintf(tmp,sizeof(tmp)-1,"%s,",name);
    fff = fff + tmp;
    switch(d->get_type(i)) {
     case 1:  // bool
      d->get(name,&bool_type);
      if(bool_type) snprintf(tmp,sizeof(tmp)-1,"true,");
      else snprintf(tmp,sizeof(tmp)-1,"false,");
      vvv = vvv + tmp;
      break;
     case 2:  // int
      d->get(name,&int_type);
      snprintf(tmp,sizeof(tmp)-1,"%d,",int_type);
      vvv = vvv + tmp;
      break;
     case 3:  // float
      d->get(name,&float_type);
      snprintf(tmp,sizeof(tmp)-1,"%f,",float_type);
      vvv = vvv + tmp;
      break;
     case 4:  // string
      bzero(string_type,sizeof(string_type));
      d->get(name,string_type);
      string_type[sizeof(tmp)-3] = 0;
      snprintf(tmp,sizeof(tmp)-1,"\'%s\',",string_type);
      vvv = vvv + tmp;
      break;
     case 5:  // datetime
      d->get(name,&date_type);
      if(date_type < 100) {
        snprintf(tmp,sizeof(tmp)-1,"NULL,");
      } else {
        snprintf(tmp,sizeof(tmp)-1,"\'%s\',",time2iso(date_type).c_str());
      }
      vvv = vvv + tmp;
      break;
     case 6:  // ipaddr
      d->get(name,&ipaddr_type);
      snprintf(tmp,sizeof(tmp)-1,"\'%s\',",ipaddr_type.get().c_str());
      vvv = vvv + tmp;
      break;
     default:
      // error
      break;
    }
  }
  fff.erase(fff.size()-1,2); // drop last comma
  vvv.erase(vvv.size()-1,2); // drop last comma
  req=req+"INSERT INTO "+d->getName()+"("+fff+") VALUES ("+vvv+")";
//printf("INSERT: %s\n",req.c_str());
  ctmp->SQL_res = sdb_query(ctmp->SQL_conn, (char*)req.c_str(), derr);
  if(ctmp->SQL_res) {
    mysql_free_result(ctmp->SQL_res);
    ctmp->SQL_res = 0;
    unlock();
    return true;
  } else {
    unlock();
    return false;
  }
}

// update fields into database
bool
db_update(int num, DbTable *d, char *derr) {
  lock();
  connection_struct *ctmp = 0;
  bool res = false;
  for(list<connection_struct*>::iterator i=con.begin();i!=con.end();i++) {
    ctmp = *i;
    if(ctmp->num == num) {
      res = true;
      break;
    }
  }
  if(!res) {
    snprintf(derr,DB_MODULE_ERRLEN,"No connection open");
    unlock();
    return false;
  }
  string req;
  string ppp;
  string vvv;
  char tmp[90];

  bool bool_type = false;
  int int_type = 0;
  float float_type = 0;
  char string_type[300];
  time_t date_type = 0;
  char name[100];
  ipaddr ipaddr_type;

  for(int i=0; i<d->size(); i++) {
    if(d->get_update(i)) { // if this is update request
      snprintf(name,sizeof(name)-1,"%s",d->get_name(i).c_str());
      vvv = vvv + name; // name
      vvv = vvv + "="; // only '='
      switch(d->get_type(i)) {
       case 1: // bool type
        d->get(name,&bool_type);
        if(bool_type) vvv = vvv + "true";
        else vvv = vvv + "false";
        break;
       case 2: // int type
        d->get(name,&int_type);
        snprintf(tmp,sizeof(tmp)-1,"%d",int_type);
        vvv = vvv + tmp;
        break;
       case 3: // float type
        d->get(name,&float_type);
        snprintf(tmp,sizeof(tmp)-1,"%f",float_type);
        vvv = vvv + tmp;
        break;
       case 4: // string type (varchar)
        bzero(string_type,sizeof(string_type));
        d->get(name,string_type);
        string_type[sizeof(tmp)-3] = 0;
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",string_type);
        vvv = vvv + tmp;
        break;
       case 5: // date type (datetime)
        d->get(name,&date_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",time2iso(date_type).c_str());
        vvv = vvv + tmp;
        break;
       case 6: // ipaddr
        d->get(name,&ipaddr_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",ipaddr_type.get().c_str());
        vvv = vvv + tmp;
        break;
       default:
        // error - unknown field
        break;
      }
      vvv = vvv + ",";
    }
  }
  vvv.erase(vvv.size()-1,2); // drop last comma
  for(int i=0; i<d->size(); i++) {
    if(d->get_request(i)) { // if this is request
      snprintf(name,sizeof(name)-1,"%s",d->get_name(i).c_str());
      ppp = ppp + name + "="; // name
      switch(d->get_type(i)) {
       case 1: // bool type
        d->get(name,&bool_type);
        if(bool_type) ppp = ppp + "true";
        else ppp = ppp + "false";
        break;
       case 2: // int type
        d->get(name,&int_type);
        snprintf(tmp,sizeof(tmp)-1,"%d",int_type);
        ppp = ppp + tmp;
        break;
       case 3: // float type
        d->get(name,&float_type);
        snprintf(tmp,sizeof(tmp)-1,"%f",float_type);
        ppp = ppp + tmp;
        break;
       case 4: // string type (varchar)
        bzero(string_type,sizeof(string_type));
        d->get(name,string_type);
        string_type[sizeof(tmp)-3] = 0;
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",string_type);
        ppp = ppp + tmp;
        break;
       case 5: // date type (datetime)
        d->get(name,&date_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",time2iso(date_type).c_str());
        ppp = ppp + tmp;
        break;
       case 6: // ipaddr
        d->get(name,&ipaddr_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",ipaddr_type.get().c_str());
        ppp = ppp + tmp;
        break;
       default:
        // error - unknown field
        break;
      }
      ppp = ppp + " AND "; // we do not support OR
    }
  }
  if(ppp.size() > 5) ppp.erase(ppp.size()-5,5); // drop last ' AND '
  req=req+"UPDATE "+d->getName()+" SET "+vvv+" WHERE "+ppp;
//cerr<<"UPDATE: "<<req<<endl;
  ctmp->SQL_res = sdb_query(ctmp->SQL_conn,(char*)req.c_str(),derr);
  if(ctmp->SQL_res) {
    mysql_free_result(ctmp->SQL_res);
    ctmp->SQL_res = 0;
    unlock();
    return true;
  } else {
    unlock();
    return false;
  }
}

// delete entry from database/table
bool
db_del(int num, DbTable *d, char *derr) {
  lock();
  connection_struct *ctmp = 0;
  bool res = false;
  for(list<connection_struct*>::iterator i=con.begin();i!=con.end();i++) {
    ctmp = *i;
    if(ctmp->num == num) {
      res = true;
      break;
    }
  }
  if(!res) {
    snprintf(derr,DB_MODULE_ERRLEN,"No connection open");
    unlock();
    return false;
  }
  string req;
  string ppp;
  char tmp[90];
  //bzero(ppp,sizeof(ppp));

  bool bool_type = false;
  int int_type = 0;
  float float_type = 0;
  char string_type[300];
  time_t date_type = 0;
  char name[100];
  ipaddr ipaddr_type;

  for(int i=0; i<d->size(); i++) {
    if(d->get_request(i)) { // if this is request
      snprintf(name,sizeof(name)-1,"%s",d->get_name(i).c_str());
      ppp = ppp + name + "=";  // name
      switch(d->get_type(i)) {
       case 1: // bool type
        d->get(name,&bool_type);
        if(bool_type) ppp = ppp + "true";
        else ppp = ppp + "false";
        break;
       case 2: // int type
        d->get(name,&int_type);
        snprintf(tmp,sizeof(tmp)-1,"%d",int_type);
        ppp = ppp + tmp;
        break;
       case 3: // float type
        d->get(name,&float_type);
        snprintf(tmp,sizeof(tmp)-1,"%f",float_type);
        ppp = ppp + tmp;
        break;
       case 4: // string type (varchar)
        bzero(string_type,sizeof(string_type));
        d->get(name,string_type);
        string_type[sizeof(tmp)-3] = 0;
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",string_type);
        ppp = ppp + tmp;
        break;
       case 5: // date type (datetime)
        d->get(name,&date_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",time2iso(date_type).c_str());
        ppp = ppp + tmp;
        break;
       case 6: // ipaddr
        d->get(name,&ipaddr_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",ipaddr_type.get().c_str());
        ppp = ppp + tmp;
        break;
       default:
        // error - unknown field
        break;
      }
      ppp = ppp + " AND "; // we do not support OR
    }
  }
  if(ppp.size() > 5) ppp.erase(ppp.size()-5,5); // drop last ' AND '
  req=req+"DELETE FROM "+d->getName()+" WHERE "+ppp;
//cerr<<"DELETE: "<<req<<endl;
  ctmp->SQL_res = sdb_query(ctmp->SQL_conn,(char*)req.c_str(),derr);
  if(ctmp->SQL_res) {
    mysql_free_result(ctmp->SQL_res);
    ctmp->SQL_res = 0;
    unlock();
    return true;
  } else {
    unlock();
    return false;
  }
}

#endif //DB_MYSQL

/////////////////////////////////////////////////////
