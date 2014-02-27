// Oracle C Call Interface module
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// (c) Copyright in 2002 by Shevchenko Vitaly
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

// OCI - Oracle Call Interface
// ORACLE datatype conversions
// 1-b CHAR(1) (Y/N)
// 2-i NUMBER(10) (if int will use 32 bit)
// 3-f FLOAT
// 4-s VARCHAR2(300)
// 5-t DATE
//
// You must set ORACLE_HOME environment variable before use it!


#ifdef DB_OCI

#include <stdlib.h>
#include <iostream>
#include "db_module.h"
#include <oci.h>

using namespace std;

extern "C" {
	int db_open(char*,int,char*,char*,char*,char*);	// open database
	bool db_close(int);					// close database
	bool db_query(int,DbTable*,char*);	// send select query
	char *db_function(int,char*,char*);	// request SQL function (only for SQL!)
	bool db_get(int,DbTable*,int,char*);	// get information
	bool db_create(int,DbTable*,char*);	// create table
	bool db_create_idx(int,DbTable*,char*);	// create table index
	bool db_add(int,DbTable*,char*);	// add information to db table
	bool db_update(int,DbTable*,char*);	// update db table
	bool db_del(int,DbTable*,char*);	// delete entry from database
}

// internal struct for caching defines to Oracle statment
struct OCIDefParam {
	OCIDefine* defnp;
	ub4 position;
	char valuep[300];
	sb4 value_sz;
	ub2 dty;
	ub2 rlen;
};

// global objects
struct connection_struct {
	int num;
	OCIEnv			*env;
	OCIServer		*srv;
	OCIError		*err;
	OCISession	*usr;
	OCISvcCtx		*svc;
	OCIStmt			*stmt;
	SList<OCIDefParam> lRes;				// results of query cache
//	int cur_rec;
};

SList<connection_struct*> con;		// list of open connections

// convert time() to ISO datetime format (which is default for postgresql)
static char*
time2iso(time_t t) {
	struct tm *ttt;
	struct tm res;
	ttt = localtime_r(&t,&res);
	static char str[64];
	bzero(str,sizeof(str));
	snprintf(str,sizeof(str)-1,"%04d-%02d-%02d %02d:%02d:%02d",
		1900+ttt->tm_year,ttt->tm_mon+1,ttt->tm_mday,
			ttt->tm_hour,ttt->tm_min,ttt->tm_sec);
	return str;
}

// convert ISO datetime format to time() value
static time_t
iso2time(char *iso) {
	char str[20];
	struct tm ttt;
	
	// read year
	int k = 0;
	for(int i=0; i<4; i++,k++) {
		str[k] = iso[i];
	}
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

static char*
checkerr(OCIError *errhp, sword status) {
	text errbuf[512];
	sb4 errcode = 0;
	static char out[600];

	switch (status) {
	case OCI_SUCCESS:				// success
		break;
	case OCI_SUCCESS_WITH_INFO:
		return "Error - OCI_SUCCESS_WITH_INFO";
		break;
	case OCI_NEED_DATA:
		return "Error - OCI_NEED_DATA";
		break;
	case OCI_NO_DATA:
		return "Error - OCI_NODATA";
		break;
	case OCI_ERROR:
		(void) OCIErrorGet((dvoid *)errhp, (ub4) 1, (text *) NULL, &errcode,
			errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
		snprintf(out,sizeof(out)-1,"Error - %.*s", 512, errbuf);
		return out;
		break;
	case OCI_INVALID_HANDLE:
		return "Error - OCI_INVALID_HANDLE";
		break;
	case OCI_STILL_EXECUTING:
		return "Error - OCI_STILL_EXECUTE";
		break;
	case OCI_CONTINUE:
		return "Error - OCI_CONTINUE";
		break;
	default:
		break;
	}
}                

// connect to the server
int
db_open(char *host,int port,char *dbase,char *login,char *pwd,char *derr) {
	sword status;
	int num = 1;	// connection id
	for(int i=0; i< con.size(); i++) {	// select connection id num
		if(con.get(i)->num == num) {
			num++;
			i = 0;
		}
	}
	connection_struct *tmp = new connection_struct;		// create new connection
	tmp->num = num;
	// initial mode - initializes the environment handle
	OCIEnvCreate(&tmp->env, OCI_DEFAULT, (dvoid*)0, 0, 0, 0, (size_t)0, (dvoid**)0);
	// allocate an error handle
	OCIHandleAlloc((dvoid*)tmp->env, (dvoid**)&tmp->err, OCI_HTYPE_ERROR, 0, (dvoid**)0);
	// We are use Single User, Single Connection - try to use simple logon session
	if(status=OCILogon(tmp->env, tmp->err, &tmp->svc, (text*)login, strlen(login),
			(text*)pwd, strlen(pwd), (text*)dbase, strlen(dbase)) != OCI_SUCCESS) {
		// can't connect to server
		snprintf(derr,DB_MODULE_ERRLEN,"Cant do OCILogon: %s",checkerr(tmp->err,status));
		delete tmp;
		return 0;
	}
	con.add(tmp);
	return num;
}

// disconnect from the server
bool
db_close(int num) {
	connection_struct *ctmp = 0;
	for(int i=0; i<con.size(); i++) {
		if(con.get(i)->num == num) {
			ctmp = con.remove(i);
			break;
		}
	}
	if(ctmp == 0) return false;
	if(ctmp->svc) {
		OCILogoff(ctmp->svc, ctmp->err);			// because we were use OCILogon
		if(ctmp->svc) OCIHandleFree((dvoid*)ctmp->svc, OCI_HTYPE_SVCCTX);
		ctmp->svc = 0;
	}
	// free a error handle
	if(ctmp->err) OCIHandleFree((dvoid*)ctmp->err, OCI_HTYPE_ERROR);
	ctmp->err = 0;
	// terminate all
	OCITerminate(OCI_DEFAULT);
	delete ctmp;									// delete connection structure
	return true;
}

// send query to SQL server
// true - success, false - reject
static bool
sdb_query(int num, char *query,char *derr) {
	connection_struct *ctmp = 0;
	sword status;
	int cnumber = 0;
	for(int i=0; i<con.size(); i++) {
		if(con.get(i)->num == num) {
			ctmp = con.get(i);
			cnumber = i;
			break;
		}
	}
	if(ctmp == 0) {
		snprintf(derr,DB_MODULE_ERRLEN,"No connection open");
		return false;
	}
	if(OCIHandleAlloc(ctmp->env, (dvoid**) &ctmp->stmt, OCI_HTYPE_STMT, 0, 0) != OCI_SUCCESS) {
		snprintf(derr,DB_MODULE_ERRLEN,"Cant do OCIHandleAlloc");
		return false;
	}
	if(OCIStmtPrepare(ctmp->stmt, ctmp->err, (text*)query,
			(ub4)strlen(query),OCI_NTV_SYNTAX, OCI_DEFAULT) != OCI_SUCCESS) {
		snprintf(derr,DB_MODULE_ERRLEN,"Cant do OCIStmtPrepare");
		return false;
	}
	if(status=OCIStmtExecute(ctmp->svc,ctmp->stmt,ctmp->err,0,0,0,0,OCI_DEFAULT) != OCI_SUCCESS) {
		snprintf(derr,DB_MODULE_ERRLEN,"Cant do OCIStmtExecute: %s",checkerr(ctmp->err,status));
		return false;
	}
	// store result columns
	// как я понимаю тему, здесь сохраняются какие-то данные, что-то по номеру
	// столбца и его позиции
	while(true)	{
		OCIDefParam	tmp;
		if(OCIDefineByPos(ctmp->stmt, &tmp.defnp, ctmp->err,
				tmp.position, (ub1 *)tmp.valuep, 300,
					SQLT_CHR, (dvoid *) 0,
						&tmp.rlen, 0, OCI_DEFAULT) != OCI_SUCCESS) break;
		tmp.valuep[tmp.rlen] = 0;	// set end of string
		ctmp->lRes.add(tmp);			// add to list
	}
//	ctmp->cur_rec = 0;

	// мне кажется, здесь следует попытаться сразу считать результаты запроса
	while(true) {
		if(OCIStmtFetch(ctmp->stmt, ctmp->err, 1, OCI_FETCH_NEXT, OCI_DEFAULT) != OCI_SUCCESS)
			break;		// ok, we read all
	}


	return true;
}

// send select query to database
bool
db_query(int num, DbTable *d, char *derr) {
	char fff[600];
	char req[900];
	char ppp[600];
	// create list of required fields
	bzero(fff,sizeof(fff));
	for(int i=0; i<d->f.size(); i++) {
		if(!d->f.get(i)->request) {	// if this is not request
			strcat(fff,d->f.get(i)->name);
			strcat(fff,",");
		}
	}
	fff[strlen(fff)-1] = 0;	// drop last comma
	// create list of known fields for request
	bzero(ppp,sizeof(ppp));
	char tmp[100];
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
				snprintf(tmp,sizeof(tmp)-1,"%u",
					(unsigned int)d->f.get(i)->int_type);
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
					time2iso(d->f.get(i)->date_type));
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
	// create SQL request
	if(strlen(ppp) > 0)
		snprintf(req,sizeof(req)-1,"SELECT %s FROM %s WHERE %s",
			fff,d->name,ppp);
	else
		snprintf(req,sizeof(req)-1,"SELECT %s FROM %s",
			fff,d->name);	
	// do request
	if(sdb_query(num,req,derr)) return true;
	else return false;
}

// send function query to database
char*
db_function(int num, char *func, char *derr) {
	connection_struct *ctmp = 0;
	int cnumber = 0;
	for(int i=0; i<con.size(); i++) {
		if(con.get(i)->num == num) {
			ctmp = con.get(i);
			cnumber = i;
			break;
		}
	}
	if(ctmp == 0) {
		snprintf(derr,DB_MODULE_ERRLEN,"No connection open");
		return false;
	}
	// create SQL request
	char req[400];
	snprintf(req,sizeof(req)-1,"SELECT %s", func);
	// do request
	if(sdb_query(num,req,derr)) {
		// now we can get result and out it here
	} else return "";
}

//  getting data
bool
db_get(int num,DbTable *d,int row,char *derr) {
	connection_struct *ctmp = 0;
	for(int i=0; i<con.size(); i++) {
		if(con.get(i)->num == num) {
			ctmp = con.get(i);
			break;
		}
	}
	if(ctmp == 0) {
		snprintf(derr,DB_MODULE_ERRLEN,"No connection open");
		return false;
	}
//	if(sdb_rows(ctmp->SQL_conn,ctmp->SQL_res) <= row) return false;
	int k=0;
	for(int i=0; i<d->f.size(); i++) {
		if(!d->f.get(i)->request) {	// if this is not request
			switch(d->f.get(i)->t) {
			 case 1:			// bool type
//				if(sdb_get(ctmp->SQL_conn,ctmp->SQL_res,row,k)[0] == 't')
//					d->f.get(i)->bool_type = true;
//				else
//					d->f.get(i)->bool_type = false;
				break;
			 case 2:			// int type
//				d->f.get(i)->int_type = atoi(sdb_get(ctmp->SQL_conn,
//					ctmp->SQL_res,row,k));
				break;
			 case 3:			// float type
//				d->f.get(i)->float_type = atof(sdb_get(ctmp->SQL_conn,
//					ctmp->SQL_res,row,k));
				break;
			 case 4:			// string type
//				sprintf(d->f.get(i)->string_type,"%s",
//					sdb_get(ctmp->SQL_conn,ctmp->SQL_res,row,k));
				break;
			 case 5:			// date type
//				d->f.get(i)->date_type =
//					iso2time(sdb_get(ctmp->SQL_conn,ctmp->SQL_res,row,k));
				break;
			 default:
				// error
				break;
			}
			k++;
		}
	}
	// XXXXXXXXXXXXXXXXXXXXXX

	return true;
}

// create table
bool
db_create(int num, DbTable *d, char *derr) {
	char req[600];
	char fff[500];
	char tmp[30];
	// create list of required fields
	bzero(fff,sizeof(fff));
	for(int i=0; i<d->f.size(); i++) {
		strcat(fff,d->f.get(i)->name);
		switch(d->f.get(i)->t) {
		 case 1:		// bool
			strcat(fff," bool");
			if(strlen(d->f.get(i)->defv) > 0) {
				strcat(fff," DEFAULT ");
				if(d->f.get(i)->defv[0]=='t')
					strcat(fff,"true");
				else
					strcat(fff,"false");
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
			snprintf(tmp,sizeof(tmp)-1," varchar(%d)",
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
			strcat(fff," datetime,");
			break;
		 default:
			// error
			break;
		}
	}
	fff[strlen(fff)-1] = 0;	// drop last comma
	snprintf(req, sizeof(req)-1, "CREATE TABLE %s(%s)",
		d->name, fff);
//printf("%s",req);
	if(sdb_query(num,	req, derr)) return true;
	else return false;
}

// create table index
bool
db_create_idx(int num, DbTable *d, char *derr) {
	char req1[600];
	char req2[600];
	char fff1[500];
	char fff2[500];
	bzero(fff1,sizeof(fff1));
	bzero(fff2,sizeof(fff2));
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
		if(!sdb_query(num, req1, derr)) return false;
	}
	if(strlen(fff2) > 0) {
		fff2[strlen(fff2)-1] = 0;	// drop last comma
		snprintf(req2,sizeof(req2)-1,"CREATE INDEX %s_idx ON %s(%s)",
			d->name,d->name,fff2);
		if(!sdb_query(num, req2, derr)) return false;
	}
	return true;
}

// insert entry to database/table
bool
db_add(int num, DbTable *d, char *derr) {
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
				time2iso(d->f.get(i)->date_type));
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
//printf("Query2:%s\n",req);
	if(sdb_query(num,	req, derr)) return true;
	else return false;
}

// update fields into database
bool
db_update(int num, DbTable *d, char *derr) {
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
					time2iso(d->f.get(i)->date_type));
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
					time2iso(d->f.get(i)->date_type));
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
	if(sdb_query(num,	req, derr)) return true;
	else return false;
}

// delete entry from database/table
bool
db_del(int num, DbTable *d, char *derr) {
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
					time2iso(d->f.get(i)->date_type));
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
	if(sdb_query(num, req, derr)) return true;
	return false;
}


/*
int
main() {
	char derr[800];
	bzero(derr,sizeof(derr));
	setenv("ORACLE_HOME","/usr2/oracle/product/9.2.0",1);
	int aaa = db_open("",0,"moon1.ospd.tts","tacacs","tacppd-test",derr);
	if(strlen(derr) > 0) cout<<"derr: "<<derr<<endl;
	cout<<"connected"<<endl;
	sdb_query(aaa, "CREATE TABLE aaa()", derr);
	if(strlen(derr) > 0) cout<<"derr: "<<derr<<endl;
//    sleep(100);
	db_close(aaa);
}
*/

#endif // DB_OCI
