// unixODBC database module
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// Support will not be provided under any circumstances for this program by
// tacppd.com, it's employees, volunteers or directors, unless a commercial
// support agreement and/or explicit prior written consent has been given.
// Visit http://tacppd.com for more information

// modifications:
//

#ifdef DB_UODBC

#ifndef _PTHREADS
#define   _PTHREADS   // enable STL pthread-safe code
#endif //_PTHREADS

#ifndef _REENTRANT
#define       _REENTRANT    // Pthread safe stuff in the stdlibs
#endif //_REENTRANT     // or you should compile with -pthread key

#include "db_module.h"
#include <cstdlib>
#include <iostream>

#include <odbc/sql.h>
#include <odbc/sqlext.h>
#include <odbc/sqltypes.h>

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
  SQLHENV       V_OD_Env;     // Handle ODBC environment
  long          V_OD_erg;     // result of functions
  SQLHDBC       V_OD_hdbc;    // Handle connection

  char          V_OD_stat[10]; // Status SQL
  SQLINTEGER    V_OD_err,V_OD_rowanz,V_OD_id;
  SQLSMALLINT   V_OD_mlen;
  char          V_OD_msg[200],V_OD_buffer[200];
};

list<connection_struct*> con; // list of open connections

pthread_mutex_t mutex_; // thread locker

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

// send query to SQL server
PGresult*
sdb_query(PGconn *SQL_conn,char *query,char *derr) {
  if(!SQL_conn) {
    snprintf(derr,DB_MODULE_ERRLEN,"PGSQL: query failed due to disconnect");
    return false;
  }
  PGresult *SQL_res = 0;
//std::cerr<<"PGSQL query: "<<query<<std::endl;
  SQL_res = PQexec(SQL_conn, query);
  if(!SQL_res ||
      PQresultStatus(SQL_res) == PGRES_BAD_RESPONSE ||
      PQresultStatus(SQL_res) == PGRES_NONFATAL_ERROR ||
      PQresultStatus(SQL_res) == PGRES_FATAL_ERROR) {
    if(SQL_res) {
      snprintf(derr,DB_MODULE_ERRLEN,"%s",PQcmdStatus(SQL_res));
      PQclear(SQL_res);
      SQL_res = 0;
    } else
      snprintf(derr,DB_MODULE_ERRLEN,"%s",PQerrorMessage(SQL_conn));
    return 0;
  }
//printf("Query: %s\n",query);
  return SQL_res;
}

// getting data
// return data if success
// NULL - othervise
string
sdb_get(PGconn *SQL_conn,PGresult *SQL_res,int i,int j) {
  char s[300];
  bzero(s,sizeof(s));
  string out;
  if(!SQL_res || !SQL_conn) return out;
  snprintf(s,sizeof(s)-1,"%s",PQgetvalue(SQL_res,i,j));
  int k = strlen(s);
  while((s[k]==' ' || s[k]=='\0') && k>0) s[k--]='\0';
  out = s;
  return out;
}

// number of rows in response
int
sdb_rows(PGconn *SQL_conn,PGresult *SQL_res) {
  if(!SQL_res) return 0;
  return(PQntuples(SQL_res));

  V_OD_erg=SQLRowCount(V_OD_hstmt,&V_OD_rowanz);
  if((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO)) {
    printf("Number ofRowCount %d\n",V_OD_erg);
    SQLFreeHandle(SQL_HANDLE_STMT,V_OD_hstmt);
    SQLDisconnect(V_OD_hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC,V_OD_hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, V_OD_Env);
    exit(0);
  }
  printf("Number of Rows %d\n",V_OD_rowanz);
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
  for(list<connection_struct*>::iterator i=con.begin();i!=con.end();) {
    tmp = *i;
    SQLFreeHandle(SQL_HANDLE_STMT,tmp->V_OD_hstmt);
    SQLDisconnect(tmp->V_OD_hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC,tmp->V_OD_hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV,tmp->V_OD_Env);
    i = con.erase(i);
  }
  unlock();
  pthread_mutex_destroy(&mutex_);
}

// database system open
//  0 - fail, >=1 - connection id
int
db_open(char *host,int port,char *dbase,char *login,char *pwd,char *derr) {
  int num = 1; // connection id
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

  // 1. allocate Environment handle and register version
  tmp->V_OD_erg = SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&(tmp->V_OD_Env));
  if((tmp->V_OD_erg != SQL_SUCCESS) && (tmp->V_OD_erg != SQL_SUCCESS_WITH_INFO)) {
    printf("Error AllocHandle\n");
    unlock();
    return 0;
  }
  tmp->V_OD_erg=SQLSetEnvAttr(tmp->V_OD_Env, SQL_ATTR_ODBC_VERSION,
                               (void*)SQL_OV_ODBC3, 0);
  if((tmp->V_OD_erg != SQL_SUCCESS) && (tmp->V_OD_erg != SQL_SUCCESS_WITH_INFO)) {
    printf("Error SetEnv\n");
    SQLFreeHandle(SQL_HANDLE_ENV, tmp->V_OD_Env);
    unlock();
    return 0;
  }
  // 2. allocate connection handle, set timeout
  tmp->V_OD_erg = SQLAllocHandle(SQL_HANDLE_DBC, tmp->V_OD_Env, &(tmp->V_OD_hdbc));
  if((tmp->V_OD_erg != SQL_SUCCESS) && (tmp->V_OD_erg != SQL_SUCCESS_WITH_INFO)) {
    printf("Error AllocHDB %d\n", tmp->V_OD_erg);
    SQLFreeHandle(SQL_HANDLE_ENV, tmp->V_OD_Env);
    unlock();
    return 0;
  }
  SQLSetConnectAttr(tmp->V_OD_hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER *)5, 0);
  // 3. Connect to the datasource "web"
  tmp->V_OD_erg = SQLConnect(tmp->V_OD_hdbc, (SQLCHAR*) "web", SQL_NTS,
                                     (SQLCHAR*) "christa", SQL_NTS,
                                     (SQLCHAR*) "", SQL_NTS);
  if((tmp->V_OD_erg != SQL_SUCCESS) && (tmp->V_OD_erg != SQL_SUCCESS_WITH_INFO)) {
    printf("Error SQLConnect %d\n",tmp->V_OD_erg);
    SQLGetDiagRec(SQL_HANDLE_DBC, tmp->V_OD_hdbc,1,
      tmp->V_OD_stat, &tmp->V_OD_err,tmp->V_OD_msg,100,&tmp->V_OD_mlen);
    printf("%s (%d)\n",tmp->V_OD_msg,tmp->V_OD_err);
    SQLFreeHandle(SQL_HANDLE_ENV, tmp->V_OD_Env);
    unlock();
    return 0;
  }
  printf("Connected !\n");
  /* continued on next page */
  con.push_back(tmp); // store connection in list
  unlock();
  return num;
}

// send select query to database
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
  char name[100];
  for(int i=0; i<d->size(); i++) {
    bzero(string_type,sizeof(string_type));
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
        d->get(name,string_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",string_type);
        ppp = ppp + tmp;
        break;
       case 5: // date type (datetime)
        d->get(name,&date_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",time2iso(date_type).c_str());
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
  if(ctmp->SQL_res) {
    PQclear(ctmp->SQL_res); // clear previous data !!!
    ctmp->SQL_res = 0;
  }
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
  char tmp[30];
  // create list of required fields
  char string_type[300];
  bzero(string_type,sizeof(string_type));
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
    PQclear(ctmp->SQL_res);
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
    PQclear(ctmp->SQL_res);
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
  char tmp[30];

  bool bool_type = false;
  int int_type = 0;
  float float_type = 0;
  char string_type[300];
  time_t date_type = 0;
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
//cerr<<"name="<<name<<" str="<<string_type<<endl;
      if(strlen(string_type)==0) snprintf(tmp,sizeof(tmp)-1,"\'\',");
      else snprintf(tmp,sizeof(tmp)-1,"\'%s\',",string_type);
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
  char tmp[30];

  bool bool_type = false;
  int int_type = 0;
  float float_type = 0;
  char string_type[300];
  bzero(string_type,sizeof(string_type));
  time_t date_type = 0;
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
        d->get(name,string_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",string_type);
        vvv = vvv + tmp;
        break;
       case 5: // date type (datetime)
        d->get(name,&date_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",time2iso(date_type).c_str());
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
        d->get(name,string_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",string_type);
        ppp = ppp + tmp;
        break;
       case 5: // date type (datetime)
        d->get(name,&date_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",time2iso(date_type).c_str());
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
  connection_struct *ctmp = 0;
  bool res = false;
  for(list<connection_struct*>::iterator i=con.begin();i!=con.end();i++) {
    ctmp = *i;
    if(ctmp->num == num) {
      res = true;
      SQLFreeHandle(SQL_HANDLE_STMT,ctmp->V_OD_hstmt);
      SQLDisconnect(ctmp->V_OD_hdbc);
      SQLFreeHandle(SQL_HANDLE_DBC,ctmp->V_OD_hdbc);
      SQLFreeHandle(SQL_HANDLE_ENV,ctmp->V_OD_Env);
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
  char tmp[30];
  //bzero(ppp,sizeof(ppp));

  bool bool_type = false;
  int int_type = 0;
  float float_type = 0;
  char string_type[300];
  bzero(string_type,sizeof(string_type));
  time_t date_type = 0;
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
        d->get(name,string_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",string_type);
        ppp = ppp + tmp;
        break;
       case 5: // date type (datetime)
        d->get(name,&date_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",time2iso(date_type).c_str());
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
    PQclear(ctmp->SQL_res);
    ctmp->SQL_res = 0;
    unlock();
    return true;
  } else {
    unlock();
    return false;
  }
}

#endif //DB_UODBC

/********************************************************/
