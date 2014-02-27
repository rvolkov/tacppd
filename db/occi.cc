// Oracle C++ database module
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// ORACLE datatype conversions
// 1-b CHAR(1) (Y/N)
// 2-i NUMBER(10) (if int will use 32 bit)
// 3-f FLOAT
// 4-s VARCHAR2(300)
// 5-t DATE
//
// You must set ORACLE_HOME environment variable before use it!

#ifdef DB_OCCI

#ifndef _PTHREADS
#define   _PTHREADS   // enable STL pthread-safe code
#endif //_PTHREADS

#ifndef _REENTRANT
#define       _REENTRANT    // Pthread safe stuff in the stdlibs
#endif //_REENTRANT     // or you should compile with -pthread key

#include "ipaddr.h"
#include "db_module.h"
#include <cstdlib>
#include <occi.h>
#include <iostream>

using namespace oracle::occi;
using namespace std;

extern "C" {
  void db_init(); // init database
  void db_destroy(); // destroy database data
  int db_open(char*,int,char*,char*,char*,char*); // open database
  bool db_close(int); // close database
  bool db_query(int,DbTable*,char*); // send query
  char *db_function(int,char*,char*); // request SQL function (only for SQL!)
  bool db_get(int,DbTable*,int,char*); // get information
  bool db_create(int,DbTable*,char*); // create table
  bool db_create_idx(int,DbTable*,char*); // create table index
  bool db_add(int,DbTable*,char*); // add information to db table
  bool db_update(int,DbTable*,char*); // update db table
  bool db_del(int,DbTable*,char*); // delete entry from database
}

// connection information (database engine specific)
struct connection_struct {
  int num; // connection id
  Environment *env;
  Connection *conn;
  Statement *stmt;
  ResultSet *rset;
};

list<connection_struct*> con; // list of open connections

pthread_mutex_t mutex_; // thread locker

#define ON_ERROR_STOP       0
#define ON_ERROR_CONTINUE   1
//int on_error_state = ON_ERROR_STOP;

// convert time() to ISO datetime format (which is default for postgresql)
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
  string ret(str);
  return ret;
}

// convert ISO datetime format to time() value
time_t
iso2time(char *in) {
  char iso[100];
  bzero(iso,sizeof(iso));
  snprintf(iso,sizeof(iso)-1,"%s",in);

  if(strcmp(iso,"NULL")==0) return 0;
  char str[20];
  struct tm ttt;
  bzero((void*)&ttt,sizeof(struct tm));

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

// send query to Oracle server
connection_struct*
sdb_query(connection_struct *SQL_conn,char *query,char *derr) {
  sprintf(derr,"");
  string sqlStmt = query;
  SQL_conn->stmt = SQL_conn->conn->createStatement(sqlStmt);
std::cerr<<"OCCI query: "<<query<<std::endl;
  SQL_conn->rset = SQL_conn->stmt->executeQuery();
  SQL_conn->conn->terminateStatement(SQL_conn->stmt);
  return SQL_res;
}

// send update/create/delete to Oracle server
connection_struct*
sdb_update(connection_struct *SQL_conn,char *query,char *derr) {
  sprintf(derr,"");
  string sqlStmt = query;
  SQL_conn->stmt = SQL_conn->conn->createStatement(sqlStmt);
std::cerr<<"OCCI update: "<<query<<std::endl;
  try {
  SQL_conn->stmt->executeUpdate();
  }catch(SQLException ex){
    snprintf(derr,DB_MODULE_ERRLEN,"%s",ex.getMessage());
  }
  SQL_conn->conn->terminateStatement(SQL_conn->stmt);
  return SQL_conn;
}

// getting data
// return data if success
// NULL - othervise
string
sdb_get(connection_struct *SQL_conn,int i,int j) {
  char s[300];
  bzero(s,sizeof(s));
  string out;
  if(!SQL_conn) return out;
  while(SQL_conn->rset->next()) {
  
  
  }
  snprintf(s,sizeof(s)-1,"%s",PQgetvalue(SQL_res,i,j));
  int k = strlen(s);
  while((s[k]==' ' || s[k]=='\0') && k>0) s[k--]='\0';
  out = s;

  SQL_conn->rset = SQL_conn->conn->; // return to begin
  return out;
}

// number of rows in response
int
sdb_rows(PGconn *SQL_conn,PGresult *SQL_res) {
  if(!SQL_res) return 0;
  return(PQntuples(SQL_res));
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
  lock();
  connection_struct *tmp;
  for(list<connection_struct*>::iterator i=con.begin();i!=con.end() && !con.empty();) {
    tmp = *i;
    i = con.erase(i);
    if(tmp->conn) {
      tmp->env->terminateConnection(tmp->conn);
    }
    if(tmp->env) {
      Environment::terminateEnvironment(tmp->env);
      tmp->env = 0;
    }
    delete tmp;
  }
  unlock();
  pthread_mutex_destroy(&mutex_);
}

// database system open
//  0 - fail, >=1 - connection id
int
db_open(char *host,int port,char *dbase,char *login,char *pwd,char *derr) {
//  if(port <= 0) port = 5432;
  int num;
  num = 1; // connection id
  lock();
  connection_struct *tmp;
  for(list<connection_struct*>::const_iterator i=con.begin();i!=con.end();i++) {
    tmp = *i;
    if(tmp->num == num) {
      num++;
      i = con.begin();
    }
  }
  tmp = new connection_struct;
  tmp->num = num;
  tmp->env = 0;
  tmp->conn = 0;
  tmp->stmt = 0;
  tmp->rset = 0;
  
  tmp->env = Environment::createEnvironment(Environment::DEFAULT);
  tmp->conn = tmp->env->createConnection(login, pwd, dbase);

  char rerr[DB_MODULE_ERRLEN+1];
  bzero(rerr,sizeof(rerr));

  tmp = sdb_update(tmp,"SET DateStyle TO 'ISO'",rerr)

  if(strlen(rerr) > 0) {
    snprintf(derr,DB_MODULE_ERRLEN,"Can't setup DateStyle to ISO: %s",rerr);
    tmp->env->terminateConnection(tmp->conn);
    Environment::terminateEnvironment(tmp->env);
    delete tmp;
    unlock();
    return 0;
  }
  con.push_back(tmp); // store connection in list
  unlock();
  return num;
}

// send select query to database
bool
db_query(int num, DbTable *d, char *derr) {
  connection_struct *ctmp = 0;
  bool res = false;
  lock();
  for(list<connection_struct*>::const_iterator i=con.begin();i!=con.end();i++) {
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
  for(int i=0; i<d->size(); i++) {
    if(!d->get_request(i)) { // if this is not request
      fff = fff + d->get_name(i);
      fff = fff + ",";
    }
  }
  fff.erase(fff.size()-1,2); //[strlen(fff)-1] = 0; // drop last comma
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
        string_type[sizeof(tmp)-3] = 0; // split if string len more
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",string_type);
        ppp = ppp + tmp;
        break;
       case 5: // date type (datetime/timestamp)
        d->get(name,&date_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",time2iso(date_type).c_str());
        ppp = ppp + tmp;
        break;
       case 6:  // ipaddr type
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
  if(ppp.size() > 5) ppp.erase(ppp.size()-5,5);//[strlen(ppp)-5] = 0; // drop last ' AND '
  // create SQL request
  string req;
  if(ppp.size() > 0)
    req = req + "SELECT " + fff + " FROM " + d->getName() + " WHERE " + ppp;
  else
    req = req + "SELECT " + fff + " FROM " + d->getName();
  // do request
//cerr<<"SELECT: "<<req<<endl;
  ctmp = sdb_query(ctmp,(char*)req.c_str(),derr);
  if(strlen(derr) == 0) {
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
  for(list<connection_struct*>::const_iterator i=con.begin();i!=con.end();i++) {
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
  ctmp = sdb_query(ctmp,req,derr);
  if(strlen(derr) == 0) {
    if(sdb_rows(ctmp) == 0) {
      unlock();
      return 0;
    }
    snprintf(aaaa,200-1,"%s",ctmp->SQL_conn->rset->getString(1).c_str());
std::cerr<<"db_function req="<<req<<" result="<<aaaa<<std::endl;
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
  for(list<connection_struct*>::const_iterator i=con.begin();i!=con.end();i++) {
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
        d->set(name,ipaddr(sdb_get(ctmp->SQL_conn,ctmp->SQL_res,row,k)));
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
db_create(int num, DbTable *d, char *derr) {
  lock();
  connection_struct *ctmp = 0;
  bool res = false;
  for(list<connection_struct*>::const_iterator i=con.begin();i!=con.end();i++) {
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
//      fff = fff + " datetime,";
      fff = fff + " timestamp,";
      break;
     case 6:  // ipaddr
      fff = fff + " inet,";
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
  ctmp = sdb_update(ctmp,(char*)req.c_str(),derr);
  if(strlen(derr) == 0) {
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
  for(list<connection_struct*>::const_iterator i=con.begin();i!=con.end();i++) {
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
    ctmp = sdb_update(ctmp,(char*)req1.c_str(),derr);
    if(!ctmp->SQL_res) {
      unlock();
      return false;
    }
    PQclear(ctmp->SQL_res);
    ctmp->SQL_res = 0;
  }
  if(fff2.size() > 0) {
    fff2.erase(fff2.size()-1,2); // drop last comma
    req2=req2+"CREATE INDEX "+d->getName()+"_idx ON "+d->getName()+"("+fff2+")";
//cerr<<"CREATE IDX: "<<req2<<endl;
    ctmp = sdb_update(ctmp,(char*)req2.c_str(),derr);
    if(!ctmp->SQL_res) {
      unlock();
      return false;
    }
    PQclear(ctmp->SQL_res);
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
  for(list<connection_struct*>::const_iterator i=con.begin();i!=con.end();i++) {
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
  char tmp[80];

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
     case 5:  // datetime/timestamp
      d->get(name,&date_type);
      if(date_type < 100) {
        snprintf(tmp,sizeof(tmp)-1,"NULL,");
      } else {
        snprintf(tmp,sizeof(tmp)-1,"\'%s\',",time2iso(date_type).c_str());
      }
      vvv = vvv + tmp;
      break;
     case 6:
      d->get(name,&ipaddr_type);
      if(ipaddr_type == UNKNOWN_IP) {
        snprintf(tmp,sizeof(tmp)-1,"NULL,");
      } else {
        snprintf(tmp,sizeof(tmp)-1,"\'%s\',",ipaddr_type.get().c_str());
      }
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
  ctmp = sdb_update(ctmp, (char*)req.c_str(), derr);
  if(ctmp->SQL_res) {
    PQclear(ctmp->SQL_res);
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
  for(list<connection_struct*>::const_iterator i=con.begin();i!=con.end();i++) {
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
  ipaddr ipaddr_type;
  char name[100];

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
        string_type[sizeof(tmp)-3] = 0; // cut
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",string_type);
        vvv = vvv + tmp;
        break;
       case 5: // date type (datetime)
        d->get(name,&date_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",time2iso(date_type).c_str());
        vvv = vvv + tmp;
        break;
       case 6:  // ipaddr type
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
       case 6:  // ipaddr type
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
  ctmp = sdb_update(ctmp,(char*)req.c_str(),derr);
  if(ctmp->SQL_res) {
    PQclear(ctmp->SQL_res);
    ctmp->SQL_res = 0;
    unlock();
    return true;
  } else {
    unlock();
    return false;
  }
}

// close - disconnect from server
bool
db_close(int num) {
  lock();
  connection_struct *ctmp;
  ctmp = 0;
  bool res;
  res = false;
  for(list<connection_struct*>::iterator i=con.begin();i!=con.end();i++) {
    ctmp = *i;
    if(ctmp->num == num) {
      res = true;
      if(tmp->conn) {
        tmp->env->terminateConnection(tmp->conn);
      }
      if(tmp->env) {
        Environment::terminateEnvironment(tmp->env);
        tmp->env = 0;
      }
      i = con.erase(i);
      delete ctmp;
      break;
    }
  }
  unlock();
  return res;
}

// delete entry from database/table
bool
db_del(int num,DbTable *d,char *derr) {
  lock();
  connection_struct *ctmp = 0;
  bool res = false;
  for(list<connection_struct*>::const_iterator i=con.begin();i!=con.end();i++) {
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
  ipaddr ipaddr_type;
  char name[100];

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
       case 5: // date type (datetime/timestamp)
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
  req = req+"DELETE FROM "+d->getName()+" WHERE "+ppp;
//cerr<<"DELETE: "<<req<<endl;
  ctmp = sdb_update(ctmp,(char*)req.c_str(),derr);
  if(ctmp->SQL_res) {
    PQclear(ctmp->SQL_res);
    ctmp->SQL_res = 0;
    unlock();
    return true;
  } else {
    unlock();
    return false;
  }
}

#endif //DB_OCCI

/********************************************************/
