// miniSQL support module
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

#ifdef DB_MSQL

#ifndef _PTHREADS
#define   _PTHREADS   // enable STL pthread-safe code
#endif //_PTHREADS

#ifndef _REENTRANT
#define       _REENTRANT    // Pthread safe stuff in the stdlibs
#endif //_REENTRANT     // or you should compile with -pthread key

#include "db_module.h"
#include <cstdlib>
#include <msql.h>
#include <iostream>

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
  int num;
  int SQL_conn;
  m_result *SQL_res;
};

list<connection_struct*> con; // list of open connections

pthread_mutex_t mutex_; // thread locker

// convert char string to time_t value
time_t
char2time(char *c) {
  time_t r = atoi(c);
  return r;
}
// convert time() value to string for store in char() fields
string
time2char(time_t t) {
  char str[100];
  snprintf(str,sizeof(str)-1,"%u",t);
  string sout = str;
  return sout;
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

//  connect to server
int
db_open(char *host,int port,char *dbase,char *login,char *pwd,char *derr) {
  int num = 1;    // connection id
  connection_struct *tmp;
  lock();
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
  if((tmp->SQL_conn=msqlConnect(host))<0) {
    snprintf(derr,DB_MODULE_ERRLEN,"Connection to miniSQL server %s failed",
      host);
    delete tmp;
    unlock();
    return 0;
  }
  if(msqlSelectDB(tmp->SQL_conn,dbase)==-1){
    snprintf(derr,DB_MODULE_ERRLEN,"Connection to miniSQL database %s failed",
      dbase);
    msqlClose(tmp->SQL_conn);
    delete tmp;
    unlock();
    return 0;
  }
  con.push_back(tmp); // store connection in list
  unlock();
  return num;
}

// disconnect from server
bool
db_close(int num) {
  connection_struct *ctmp = 0;
  bool res = false;
  for(list<connection_struct*>::iterator i=con.begin();i!=con.end();i++) {
    ctmp = *i;
    if(ctmp->num == num) {
      i = con.erase(i);
      res = true;
      break;
    }
  }
  if(!res) {
    unlock();
    return false;
  }
  if(ctmp->SQL_res) msqlFreeResult(ctmp->SQL_res);
  msqlClose(ctmp->SQL_conn);
  delete ctmp;
  unlock();
  return true;
}

//  send query to SQL server
//  true - success, false - reject
m_result*
sdb_query(int SQL_conn,char *query,char *derr) {
  int err;
  m_result *SQL_res;
  if((err=msqlQuery(SQL_conn,query))==-1){
    snprintf(derr,DB_MODULE_ERRLEN,"%s",msqlErrMsg);
    return 0;
  }
  SQL_res = msqlStoreResult();
  return SQL_res;
}

//  getting data
// return data if success
// NULL - othervise
string
sdb_get(int SQL_conn,m_result *SQL_res,int i,int j) {
  m_row row;
  string sout;
  if(i >= msqlNumRows(SQL_res)) return sout;
  msqlDataSeek(SQL_res,i);
  row = msqlFetchRow(SQL_res);
  char ret[800];
  bzero(ret,sizeof(ret));
  if(row) {
    snprintf(ret,sizeof(ret)-1,"%s",row[j]);
    sout = ret;
    return sout;
  }
  return sout;
}

// number of rows in response
int
sdb_rows(int SQL_conn,m_result *SQL_res) {
  return(msqlNumRows(SQL_res));
}

// send query to database
bool
db_query(int num, DbTable *d, char *derr) {
  connection_struct *ctmp = 0;
  bool res = false;
  lock();
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
  fff.erase(fff.size()-1,2); // drop last comma
  // create list of known fields for request
  bool bool_type = false;
  int int_type = 0;
  float float_type = 0;
  char string_type[300];
  time_t date_type = 0;
  char name[100];
  char tmp[100];
  for(int i=0; i<d->size(); i++) {
    if(d->get_request(i)) { // if this is request
      snprintf(name,sizeof(name)-1,"%s",d->get_name(i).c_str());
      ppp = ppp + name; // name
      ppp = ppp + "="; // only '='
      switch(d->get_type(i)) {
       case 1: // bool type
        d->get(name,&bool_type);
        if(bool_type) ppp = ppp + "t";
        else ppp = ppp + "f";
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
        string_type[sizeof(tmp)-3] = 0; // cut tail if present
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",string_type);
        ppp = ppp + tmp;
        break;
       case 5: // date type (datetime)
        d->get(name,&date_type);
        snprintf(tmp,sizeof(tmp)-1,"\'%s\'",time2char(date_type).c_str());
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
  if(ppp.size() > 0)
    req = req + "SELECT " + fff + " FROM " + d->getName() + " WHERE " + ppp;
  else
    req = req + "SELECT " + fff + " FROM " + d->getName();
  // do request
//cerr<<"SELECT: "<<req<<endl;
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
    return "";
  }
  // create SQL request
  char req[400];
  snprintf(req,sizeof(req)-1,"SELECT %s", func);
  // do request
  ctmp->SQL_res = sdb_query(ctmp->SQL_conn,req,derr);
  if(ctmp->SQL_res) return "";
  else return "";
}

// get information from database to DbTable list
bool
db_get(int num,DbTable *d,int row,char *derr) {
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
    return false;
  }
  if(sdb_rows(ctmp->SQL_conn,ctmp->SQL_res) <= row) return false;
  int k=0;
  for(int i=0; i<d->f.size(); i++) {
  	if(!d->f.get(i)->request) {	// if this is not request
  		switch(d->f.get(i)->t) {
  		 case 1:			// bool type
  			if(sdb_get(ctmp->SQL_conn,ctmp->SQL_res,row,k)[0] == 't')
  				d->f.get(i)->bool_type = true;
  			else
  				d->f.get(i)->bool_type = false;
  			break;
  		 case 2:			// int type
  			d->f.get(i)->int_type =
  				atoi(sdb_get(ctmp->SQL_conn,ctmp->SQL_res,row,k));
  			break;
  		 case 3:			// float type
  			d->f.get(i)->float_type =
  				atof(sdb_get(ctmp->SQL_conn,ctmp->SQL_res,row,k));
  			break;
       case 4:  // string type
        sprintf(d->f.get(i)->string_type,"%s",
          sdb_get(ctmp->SQL_conn,ctmp->SQL_res,row,k));
        break;
       case 5:  // date type
        d->f.get(i)->date_type =
          char2time(sdb_get(ctmp->SQL_conn,ctmp->SQL_res,row,k));
        break;
       default:
        // error
        break;
      }
      k++;
    }
  }
  return true;
}

// create table
bool
db_create(int num,DbTable *d, char *derr) {
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
		return false;
	}
	char req[600];
	char fff[500];
	char tmp[30];
	// create list of required fields
	bzero(fff,sizeof(fff));
	for(int i=0; i<d->f.size(); i++) {
		strcat(fff,d->f.get(i)->name);
		switch(d->f.get(i)->t) {
		 case 1:		// bool
			strcat(fff," char(1)");
			if(strlen(d->f.get(i)->defv) > 0) {
				strcat(fff," DEFAULT ");
				if(d->f.get(i)->defv[0]=='t')
					strcat(fff,"\'t\'");
				else
					strcat(fff,"\'f\'");
			}
			strcat(fff,",");
			break;
		 case 2:		// int
			strcat(fff," int");
			if(strlen(d->f.get(i)->defv) > 0) {
				strcat(fff," DEFAULT ");
				strcat(fff,d->f.get(i)->defv);
			}
			strcat(fff,",");
			break;
		 case 3:		// float
			strcat(fff," float");
			if(strlen(d->f.get(i)->defv) > 0) {
				strcat(fff," DEFAULT ");
				strcat(fff,d->f.get(i)->defv);
			}
			strcat(fff,",");
			break;
		 case 4:		// string
			snprintf(tmp,sizeof(tmp)-1," char(%d)",
				d->f.get(i)->string_len);
			strcat(fff,tmp);
			if(strlen(d->f.get(i)->defv) > 0) {
				strcat(fff," DEFAULT ");
				strcat(fff,"\'");
				strcat(fff,d->f.get(i)->defv);
				strcat(fff,"\'");
			}
			strcat(fff,",");
			break;
		 case 5:		// datetime
			strcat(fff," char(10),");
			break;
		 default:
			// error
			break;
		}
	}
	fff[strlen(fff)-1] = 0;	// drop last comma
	snprintf(req, sizeof(req)-1, "CREATE TABLE %s(%s)",
		d->name, fff);
	ctmp->SQL_res = sdb_query(ctmp->SQL_conn,req,derr);
	if(ctmp->SQL_res) return true;
	else return false;
}

// create table index
bool
db_create_idx(int num, DbTable *d, char *derr) {
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
		return false;
	}
	char req1[600];
	char req2[600];
	char fff1[500];
	char fff2[500];
	bzero(fff1,sizeof(fff1));
	bzero(fff2,sizeof(fff2));
//	char tmp[30];
	// create list of required fields
	for(int i=0; i<d->f.size(); i++) {
		if(d->f.get(i)->idx == T_UNIQUE) {
			strcat(fff1,d->f.get(i)->name);
			strcat(fff1,",");
		}
		if(d->f.get(i)->idx == T_INDEX) {
			strcat(fff2,d->f.get(i)->name);
			strcat(fff2,",");
		}
	}
	if(strlen(fff1) > 0) {
		fff1[strlen(fff1)-1] = 0;	// drop last comma
		snprintf(req1,sizeof(req1)-1,"CREATE UNIQUE INDEX %s_idx ON %s(%s)",
			d->name,d->name,fff1);
		ctmp->SQL_res = sdb_query(ctmp->SQL_conn,
			req1, derr);
		if(!ctmp->SQL_res) return false;
	}
	if(strlen(fff2) > 0) {
		fff2[strlen(fff2)-1] = 0;	// drop last comma
		snprintf(req2,sizeof(req2)-1,"CREATE INDEX %s_idx ON %s(%s)",
			d->name,d->name,fff2);
		ctmp->SQL_res = sdb_query(ctmp->SQL_conn,
			req2, derr);
		if(!ctmp->SQL_res) return false;
	}
	return true;
}

// insert entry to database/table
bool
db_add(int num, DbTable *d, char *derr) {
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
		return false;
	}
	char req[900];
	char fff[700];
	char vvv[700];
	char tmp[30];

	bzero(fff,sizeof(fff));
	bzero(vvv,sizeof(vvv));
	for(int i=0; i<d->f.size(); i++) {
		snprintf(tmp,sizeof(tmp)-1,"%s,",d->f.get(i)->name);
		strcat(fff,tmp);
		switch(d->f.get(i)->t) {
		 case 1:			// bool
			if(d->f.get(i)->bool_type) snprintf(tmp,sizeof(tmp)-1,"true,");
			else snprintf(tmp,sizeof(tmp)-1,"false,");
			strcat(vvv,tmp);
			break;
		 case 2:			// int
			snprintf(tmp,sizeof(tmp)-1,"%d,",d->f.get(i)->int_type);
			strcat(vvv,tmp);
			break;
		 case 3:			// float
			snprintf(tmp,sizeof(tmp)-1,"%f,",d->f.get(i)->float_type);
			strcat(vvv,tmp);
			break;
		 case 4:			// string
			if(!d->f.get(i)->string_type) snprintf(tmp,sizeof(tmp)-1,"\'\',");
			else snprintf(tmp,sizeof(tmp)-1,"\'%s\',",
				d->f.get(i)->string_type);
			strcat(vvv,tmp);
			break;
		 case 5:			// datetime
			snprintf(tmp,sizeof(tmp)-1,"\'%s\',",
				time2char(d->f.get(i)->date_type));
			strcat(vvv,tmp);
			break;
		 default:
			// error
			break;
		}
	}
	fff[strlen(fff)-1] = 0;	// drop last comma
	vvv[strlen(vvv)-1] = 0;	// drop last comma
	snprintf(req, sizeof(req)-1, "INSERT INTO %s(%s) VALUES (%s)",
		d->name, fff, vvv);
	ctmp->SQL_res = sdb_query(ctmp->SQL_conn,req,derr);
	if(ctmp->SQL_res) return true;
	else return false;
}

// update fields into database
bool
db_update(int num, DbTable *d, char *derr) {
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
		return false;
	}
	char req[500];
	char ppp[400];
	char vvv[400];
	char tmp[30];

	bzero(vvv,sizeof(vvv));
	for(int i=0; i<d->f.size(); i++) {
		if(d->f.get(i)->update) {	// if this is update request
			strcat(vvv,d->f.get(i)->name);	// name
			strcat(vvv,"=");			// only '='
			switch(d->f.get(i)->t) {
			 case 1:			// bool type
				if(d->f.get(i)->bool_type) strcat(vvv,"true");
				else strcat(vvv,"false");
				break;
			 case 2:			// int type
				snprintf(tmp,sizeof(tmp)-1,"%d",d->f.get(i)->int_type);
				strcat(vvv,tmp);
				break;
			 case 3:			// float type
				snprintf(tmp,sizeof(tmp)-1,"%f",d->f.get(i)->float_type);
				strcat(vvv,tmp);
				break;
			 case 4:			// string type (varchar)
				snprintf(tmp,sizeof(tmp)-1,"\'%s\'",d->f.get(i)->string_type);
				strcat(vvv,tmp);
				break;
			 case 5:			// date type (datetime)
				snprintf(tmp,sizeof(tmp)-1,"\'%s\'",
					time2char(d->f.get(i)->date_type));
				strcat(vvv,tmp);
				break;
			 default:
				// error - unknown field
				break;
			}
			strcat(vvv,",");
		}
	}
	vvv[strlen(vvv)-1] = 0;	// drop last comma

	bzero(ppp,sizeof(ppp));
	for(int i=0; i<d->f.size(); i++) {
		if(d->f.get(i)->request) {	// if this is request
			strcat(ppp,d->f.get(i)->name);	// name
			strcat(ppp,"=");			// only '='
			switch(d->f.get(i)->t) {
			 case 1:			// bool type
				if(d->f.get(i)->bool_type) strcat(ppp,"true");
				else strcat(ppp,"false");
				break;
			 case 2:			// int type
				snprintf(tmp,sizeof(tmp)-1,"%d",d->f.get(i)->int_type);
				strcat(ppp,tmp);
				break;
			 case 3:			// float type
				snprintf(tmp,sizeof(tmp)-1,"%f",d->f.get(i)->float_type);
				strcat(ppp,tmp);
				break;
			 case 4:			// string type (varchar)
				snprintf(tmp,sizeof(tmp)-1,"\'%s\'",d->f.get(i)->string_type);
				strcat(ppp,tmp);
				break;
			 case 5:			// date type (datetime)
				snprintf(tmp,sizeof(tmp)-1,"\'%s\'",
					time2char(d->f.get(i)->date_type));
				strcat(ppp,tmp);
				break;
			 default:
				// error - unknown field
				break;
			}
			strcat(ppp," AND ");	// we do not support OR
		}
	}
	if(strlen(ppp) > 5) ppp[strlen(ppp)-5] = 0;	// drop last ' AND '
	snprintf(req, sizeof(req)-1, "UPDATE %s SET %s WHERE %s",
		d->name, vvv, ppp);

	ctmp->SQL_res = sdb_query(ctmp->SQL_conn,req,derr);
	if(ctmp->SQL_res) return true;
	else return false;
}

// delete entry from database/table
bool
db_del(int num, DbTable *d, char *derr) {
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
		return false;
	}
	char req[900];
	char ppp[700];
	char tmp[30];

	bzero(ppp,sizeof(ppp));
	for(int i=0; i<d->f.size(); i++) {
		if(d->f.get(i)->request) {	// if this is request
			strcat(ppp,d->f.get(i)->name);	// name
			strcat(ppp,"=");			// only '='
			switch(d->f.get(i)->t) {
			 case 1:			// bool type
				if(d->f.get(i)->bool_type) strcat(ppp,"true");
				else strcat(ppp,"false");
				break;
			 case 2:			// int type
				snprintf(tmp,sizeof(tmp)-1,"%d",d->f.get(i)->int_type);
				strcat(ppp,tmp);
				break;
			 case 3:			// float type
				snprintf(tmp,sizeof(tmp)-1,"%f",d->f.get(i)->float_type);
				strcat(ppp,tmp);
				break;
			 case 4:			// string type (varchar)
				snprintf(tmp,sizeof(tmp)-1,"\'%s\'",d->f.get(i)->string_type);
				strcat(ppp,tmp);
				break;
			 case 5:			// date type (datetime)
				snprintf(tmp,sizeof(tmp)-1,"\'%s\'",
					time2char(d->f.get(i)->date_type));
				strcat(ppp,tmp);
				break;
			 default:
				// error - unknown field
				break;
			}
			strcat(ppp," AND ");	// we do not support OR
		}
	}
	if(strlen(ppp) > 5) ppp[strlen(ppp)-5] = 0;	// drop last ' AND '
	snprintf(req, sizeof(req)-1, "DELETE FROM %s WHERE %s",
		d->name, ppp);

	ctmp->SQL_res = sdb_query(ctmp->SQL_conn,req,derr);
	if(ctmp->SQL_res) return true;
	else return false;
}

#endif //DB_MSQL
/////////////////////////////////////////////////////
