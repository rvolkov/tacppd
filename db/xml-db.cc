// DB module for xml file engine
// (c) Copyright in 2000-2007 by tacppd contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// Igor A. Lyapin 
// Russia

// Support will not be provided under any circumstances for this program by
// tacppd.com, it's employees, volunteers or directors, unless a commercial
// support agreement and/or explicit prior written consent has been given.
// Visit http://tacppd.com for more information

// modifications:
//

//#define TEST

//#ifdef DB_XML

#ifndef _PTHREADS
#define   _PTHREADS   // enable STL pthread-safe code
#endif //_PTHREADS

#ifndef _REENTRANT
#define       _REENTRANT    // Pthread safe stuff in the stdlibs
#endif //_REENTRANT     // or you should compile with -pthread key

#ifndef TEST
#include "db_module.h"
#endif //TEST

#include <expat.h>
#include <iostream>
#include <string>
#include <fstream>
#include <list>
#include <map>
#include <sstream>

#ifdef HAVE_SSTREAM
#include <sstream>
#else
#include <strstream>
#endif

#ifdef TEST
#include "../include/db_module.h"
#endif //TEST

using std::map;
using std::cerr;
using std::endl;
using std::string;
using std::ostringstream;
using std::ifstream;
using std::ofstream;
using std::ios;

extern "C" {
  void db_init(); // init database
  void db_destroy(); // destroy database data
  int db_open(char*,int,char*,char*,char*,char*); // open database
  bool db_close(int); // close database
  bool db_query(int,DbTable*,char*);  // send query
  char * db_function(int,char*,char*); // call db function
  bool db_get(int,DbTable*,int,char*); // get information
  bool db_create(int,DbTable*,char*); // create table
  bool db_create_idx(int,DbTable*,char*); // create table index
  bool db_add(int,DbTable*,char*); // add information to db table
  bool db_update(int,DbTable*,char*); // update db table
  bool db_del(int,DbTable*,char*);  // delete entry from database
};

enum action {SELECT_ROW,INSERT_ROW,UPDATE_ROW,DELETE_ROW,INSERT_TABLE};
// connection information (database engine specific)
struct connection_struct {
  int num; // connect id
  DbTable * dbt;
  //parser common data
  map < int,map <string,string> > resMap;
  bool skipObj;
  XML_Parser parcr;
  int  streamByte[2][2];//[0][0,1] start,len [1][0,1] end,len
			//
  action act; //what action to perform
  int depth; // colum number of current xml phrase
  int buffDeviation; //when changes occure current position in parser buff
  //may be quite different from  dbBuff because of previous changes
};

list<connection_struct*> con; // list of open connections

pthread_mutex_t mutex_; // thread locker
string dbBuff;
string dbname;
bool isOpen=false;

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
 cerr<< "DB init"<<endl;
}

void
db_destroy() {
  pthread_mutex_destroy(&mutex_);
}

string convert2WF(string str) {
//convert special xml symbol
//cerr<<"before convert"<<str<<endl;
    unsigned int pos=0;
    while((pos=str.find('&',pos))!=string::npos){str.replace(pos,1,"&amp;");pos++;};
    while((pos=str.find('>'))!=string::npos){str.replace(pos,1,"&gt;");};
    while((pos=str.find('<'))!=string::npos){str.replace(pos,1,"&lt;");};
    while((pos=str.find('\''))!=string::npos){str.replace(pos,1,"&apos;");};
    while((pos=str.find('\"'))!=string::npos){str.replace(pos,1,"&quot;");};
//cerr<<"after convert"<<str<<endl;    
return str;
}

string makeRow (connection_struct*cs,int delim=0);
string makeRow (connection_struct*cs,int delim) {
  bool bool_type = false;
  int int_type = 0;
  float float_type = 0;
  char string_type[300];
  time_t date_type = 0;
  bzero(string_type,sizeof(string_type));

  string row;
  if(cs->act==INSERT_ROW)delim+=3;
  row.assign(delim,' ');
  ostringstream os;
  os<<row<<"<row ";

  for(int i=0; i<cs->dbt->size(); i++) {
    os<<" "<<cs->dbt->get_name(i)<<" = \'";

    if(cs->act==UPDATE_ROW)if(!cs->dbt->get_update(i)) {  
    //get previous row data
      os<<convert2WF(cs->resMap[0].operator[](cs->dbt->get_name(i)))<<"\' ";
      cerr<<"Row for update "<<convert2WF(cs->resMap[0].operator[](cs->dbt->get_name(i)))<<endl;
      continue;
    }

      switch(cs->dbt->get_type(i)) {
       case 1: // bool type
        cs->dbt->get(cs->dbt->get_name(i).c_str(),&bool_type);
        if(bool_type)os<<"true";
	else os<<"false";
        break;
       case 2: // int type
        cs->dbt->get(cs->dbt->get_name(i).c_str(),&int_type);
        os<<int_type;
        break;
       case 3: // float type
        cs->dbt->get(cs->dbt->get_name(i).c_str(),&float_type);
        os<<float_type;
        break;
       case 4: // string type (varchar)
        cs->dbt->get(cs->dbt->get_name(i).c_str(),string_type);
cerr<<" !!!!!!!!update -"<<string_type<<endl;
        os<<convert2WF(string_type);
        break;
       case 5: // date type (datetime)
        cs->dbt->get(cs->dbt->get_name(i).c_str(),&date_type);
	os<<time2iso(date_type);
        break;
       default:
        // error - unknown field
        break;
      }
    
    os<<"\' ";
  }
  os<<"/>";
cerr << "Making the following row for action:-"<<os.str()<<"-\n";
return os.str();
}

bool xdb_compareRow(void *data, const char **attr) {
  connection_struct *cs=(connection_struct*)data;
  bool bool_type = false;
  int int_type = 0;
  float float_type = 0;
  char string_type[300];
  time_t date_type = 0;
  
cerr<<" compare row "<<endl;
  for(int i=0; i<cs->dbt->size(); i++) {
    if(cs->dbt->get_request(i)) { // if this is request
     for(int att=0;attr[att];att+=2)
     {
     if(strcasecmp(attr[att],cs->dbt->get_name(i).c_str()))continue;
      switch(cs->dbt->get_type(i)) {
       case 1: // bool type
        cs->dbt->get(cs->dbt->get_name(i).c_str(),&bool_type);
 cerr<<"compare bool "<<attr[att+1] <<"?"<<bool_type<<endl;

        if(bool(strcasecmp(attr[att+1],"true"))==bool_type)return false;
        break;
       case 2: // int type
        cs->dbt->get(cs->dbt->get_name(i).c_str(),&int_type);
 cerr<<"compare int "<<attr[att+1] <<"?"<<int_type<<endl;
        if(atoi(attr[att+1])!=int_type)return false;
        break;
       case 3: // float type
        cs->dbt->get(cs->dbt->get_name(i).c_str(),&float_type);
 cerr<<"compare float "<<attr[att+1] <<"?"<<float_type<<endl;
        if(atof(attr[att+1])!=float_type)return false;
        break;
       case 4: // string type (varchar)
        cs->dbt->get(cs->dbt->get_name(i).c_str(),string_type);
 cerr<<"compare varchar "<<attr[att+1] <<"?"<<string_type<<endl;
        if(strcasecmp(attr[att+1],string_type))return false;
        break;
       case 5: // date type (datetime)
        cs->dbt->get(cs->dbt->get_name(i).c_str(),&date_type);
 cerr<<"compare time "<<attr[att+1] <<"?"<<time2iso(date_type)<<endl;
	if(strcasecmp(attr[att+1],time2iso(date_type).c_str()))return false;
        break;
       default:
        // error - unknown field
        break;
      }
    }
    }
      
  }
cerr<<" compare is  true"<<endl;
  return true;
}
static void xdb_queryStart(void *data, const char *el, const char **attr) {
    connection_struct *cs=(connection_struct*)data;
cerr<<"xdb_queryStart element="<<el<<" ("<<attr[1]<<") "<<endl;    
    
    if(!strcasecmp(el,"table")) {

      for (int i = 0; attr[i]; i += 2) {
       if(!strcasecmp(attr[i],"name")) {
        if(!strcasecmp(attr[i+1],cs->dbt->getName().c_str())) {
	  cs->skipObj=false;
	  cs->streamByte[0][0]=XML_GetCurrentByteIndex(cs->parcr);
	  cs->streamByte[0][1]=XML_GetCurrentByteCount(cs->parcr);  
	  if(cs->act==INSERT_ROW) { 
	      cs->depth=XML_GetCurrentColumnNumber(cs->parcr);  
	      dbBuff.insert(cs->streamByte[0][0]+cs->streamByte[0][1],
	      "\n"+makeRow(cs,cs->depth));
		cs->skipObj=true;
	
	  }
	}
        else cs->skipObj=true;
       }
      }
     return;
    }
    if(!strcasecmp(el,"database")&&cs->act==INSERT_TABLE) { 
      string dbstr;
      cs->streamByte[0][0]=XML_GetCurrentByteIndex(cs->parcr);
      cs->streamByte[0][1]=XML_GetCurrentByteCount(cs->parcr);  
      cs->depth=XML_GetCurrentColumnNumber(cs->parcr);  
      
      dbstr.assign(cs->depth+3,' ');

      dbstr = "\n"+dbstr+" <TABLE name=\'" + cs->dbt->getName() + "\'>\
               \n"+dbstr+" </TABLE>";

      
      dbBuff.insert(cs->streamByte[0][0]+cs->streamByte[0][1],dbstr);
      cs->skipObj=true;
      
      return ;
    }
    if(cs->skipObj)return;


    if(!strcasecmp(el,"row")) {
    map <string,string> nRow;

    switch (cs->act){

    case SELECT_ROW:
cerr<<"select row"<<endl;
    if(!xdb_compareRow(data,attr))return;
     //--------========== save row
cerr<<"save row"<<endl;
         for(int att=0;attr[att];att+=2)
	 nRow.insert(map <string,string>::value_type(string(attr[att]),string(attr[att+1])));
       
         cs->resMap.insert(map< int,map <string,string> >::value_type(cs->resMap.size(),nRow));
    
    break;
    case UPDATE_ROW:
cerr<<"update row"<<endl;    
      cs->streamByte[0][0]=-1;
      cs->streamByte[0][1]=-1;  

    if(!xdb_compareRow(data,attr))return;
      cs->streamByte[0][0]=XML_GetCurrentByteIndex(cs->parcr);
      cs->streamByte[0][1]=XML_GetCurrentByteCount(cs->parcr);  
      cs->depth=XML_GetCurrentColumnNumber(cs->parcr);  
    for(int att=0;attr[att];att+=2)
	 nRow.insert(map <string,string>::value_type(string(attr[att]),string(attr[att+1])));
       
         cs->resMap.insert(map< int,map <string,string> >::value_type(cs->resMap.size(),nRow));
      
    break;
    case DELETE_ROW:
cerr<<"delete row"<<endl;    
      cs->streamByte[0][0]=-1;
      cs->streamByte[0][1]=-1;  

    if(!xdb_compareRow(data,attr))return;
      cs->streamByte[0][0]=XML_GetCurrentByteIndex(cs->parcr);
      cs->streamByte[0][1]=XML_GetCurrentByteCount(cs->parcr);  
    break;
    default:
cerr<<"defaultact none"<<endl;    
    break;
    }
    }
cerr<<"end query start"<<endl;    
    
}

static void xdb_queryStop(void *data, const char *el) {
    connection_struct *cs=(connection_struct*)data;
cerr<<"xdb_queryStop element="<<el<<" skip obj="<<cs->skipObj<<endl;
    if(cs->skipObj)return;
    if(!strcasecmp(el,"row")) {
cerr<<"raw end"<<endl;
string tmprow;
int dotS,dotE;


    switch (cs->act) {
        case DELETE_ROW:
cerr<<"raw end DELETE_ROW"<<endl;
	  if(cs->streamByte[0][0]==-1 && cs->streamByte[0][1]==-1)return;
	  
          cs->streamByte[1][0]=XML_GetCurrentByteIndex(cs->parcr);
	  cs->streamByte[1][1]=XML_GetCurrentByteCount(cs->parcr);
cerr<<"cs->streamByte[0][0]="<<cs->streamByte[0][0]<<endl;
cerr<<"cs->streamByte[0][1]="<<cs->streamByte[0][1]<<endl;
cerr<<"cs->streamByte[1][0]="<<cs->streamByte[1][0]<<endl;
cerr<<"cs->streamByte[1][1]="<<cs->streamByte[1][1]<<endl;
cerr<<"Buff deviation ---------------========="<<cs->buffDeviation<<endl;
cerr<<dbBuff<<endl;
	    dotS=cs->streamByte[0][0]-cs->buffDeviation;
	    dbBuff.replace(cs->streamByte[0][0]-cs->buffDeviation,
	    cs->streamByte[1][0]+cs->streamByte[1][1]-cs->streamByte[0][0],"");
	    cs->buffDeviation+=cs->streamByte[1][0]+cs->streamByte[1][1]-cs->streamByte[0][0];
	    dotE=dbBuff.find_first_not_of(" \t",dotS);
	    dotS=dbBuff.find_last_not_of(" \t\n",dotS)+1;
	    dbBuff.replace(dotS,dotE-dotS,"");
cerr<<"dotS="<<dotS<<" dotE"<<dotE<<endl;
	    cs->buffDeviation+=dotE-dotS;
cerr<<"Buff deviation ---------------========="<<cs->buffDeviation<<endl;


	break;
        case UPDATE_ROW:
cerr<<"raw end UPDATE_ROW"<<endl;
	  if(cs->streamByte[0][0]==-1 && cs->streamByte[0][1]==-1)return;
          cs->streamByte[1][0]=XML_GetCurrentByteIndex(cs->parcr);
	  cs->streamByte[1][1]=XML_GetCurrentByteCount(cs->parcr);
	 tmprow=makeRow(cs);
	    dbBuff.replace(cs->streamByte[0][0]-cs->buffDeviation,
	    cs->streamByte[1][0]+cs->streamByte[1][1]-cs->streamByte[0][0],tmprow);
	    cs->buffDeviation+=cs->streamByte[1][0]+cs->streamByte[1][1]-cs->streamByte[0][0]-tmprow.size();
	    cs->resMap.erase(cs->resMap.begin(),cs->resMap.end());
	break;
	default:
	break;
	
    }
cerr<<"-(1)- end"<<endl;
    return;
    }

    if(!strcasecmp(el,"table")) {
cerr<<"table end"<<endl;
    
    if(cs->skipObj==false) {
      cs->streamByte[1][0]=XML_GetCurrentByteIndex(cs->parcr);
      cs->streamByte[1][1]=XML_GetCurrentByteCount(cs->parcr);
     }
    }
    cs->skipObj=true;
cerr<<"-(2)- end"<<endl;
}

bool xmlAction(int num,DbTable *d,char *derr,action act) {
  lock();
  cerr<<"xmlAction connection="<<num<<endl;
  connection_struct *ctmp = 0;
  
  bool res = false;
  for(list<connection_struct*>::iterator i=con.begin();i!=con.end();i++) {
    ctmp = *i;
    if(ctmp->num == num) {
      res = true;
      break;
    }
  }
  cerr<<"Connection is "<<res<<endl;
  if(!res) {
    snprintf(derr,DB_MODULE_ERRLEN,"No connection open");
    unlock();
    return false;
  }
  
  XML_Parser parcr = XML_ParserCreate(NULL);
  cerr<<"Creating parser"<<endl;
  if (! parcr) {
  //throw
   unlock();
    cerr<<"Couldn't allocate memory for parser\n";
    return false;
  }
  ctmp->parcr=parcr;
  ctmp->act=act;

  ctmp->dbt=d;
  ctmp->skipObj=false;
  ctmp->resMap.erase(ctmp->resMap.begin(),ctmp->resMap.end());
  ctmp->buffDeviation=0;
  
  XML_SetUserData(parcr,(void*)ctmp);
  XML_SetElementHandler(parcr, xdb_queryStart, xdb_queryStop);
  XML_SetParamEntityParsing(parcr,XML_PARAM_ENTITY_PARSING_ALWAYS);

  string duuf;

 if(act==SELECT_ROW)res=XML_Parse(parcr, dbBuff.c_str(), dbBuff.length(), 0);
 else {
   duuf=dbBuff;
   res=XML_Parse(parcr, duuf.c_str(), duuf.length(), 0);
  }
  
  if (! res) {
    cerr<<"Parse error at line "<<XML_GetCurrentLineNumber(parcr)<<
      "Column"<<XML_GetCurrentColumnNumber(parcr)<<
      ":"<<XML_ErrorString(XML_GetErrorCode(parcr))<<endl;
    cerr<<"Buffer=\n"<<dbBuff<<endl;
    unlock();
    exit(-1);  
 }
cerr<<"Xml action stop"<<endl;
  unlock();
  return true;
}

int db_open(char *host,int port,char *dbase,char *login,char *pwd,char *derr) {
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
  cerr<<"Num="<<num<<endl;
  tmp = new connection_struct;
  tmp->num = num;

  if(isOpen) {  con.push_back(tmp);
unlock();return num;}

  cerr<<"Pre lock"<<endl;

  isOpen=true;
  dbname=dbase;
  cerr<<"creating fstream"<<endl;

  ifstream *fcfstr=new ifstream();
  cerr<<"Trying to open"<<dbname<<endl;
  fcfstr->open(dbname.c_str(),std::ios::in);
  if(!fcfstr) {
    isOpen=false;
    cerr<<"Could not open file"<<endl;
    unlock();
    return -1;
    //throw smthing
  }
  fcfstr->unsetf(ios::skipws);
  cerr<<"ReadBuff"<<endl;
  char t;
   while(fcfstr->get(t))dbBuff+=t;
  cerr<<"End ReadBuff"<<endl;
  cerr<< dbBuff;

  fcfstr->close();
  delete fcfstr;
  con.push_back(tmp);
  unlock();
  return num;
}

bool db_close(int num) {
  lock();
  connection_struct *ctmp = 0;
  bool res = false;
  action ac=UPDATE_ROW;
  cerr<<"close connection num="<<num<<endl;
  if(!isOpen)return res;
  for(list<connection_struct*>::iterator i=con.begin();i!=con.end();i++) {
    ctmp = *i;
    if(ctmp->num == num) {
      res = true;
      ac= ctmp->act;
      i = con.erase(i);
      delete ctmp;
      break;
    }
  }
  cerr<<"now flash changes to file."<<endl;
  if(con.size()==0 && ac !=SELECT_ROW) {
  cerr<<"now flash changes to file."<<endl;

//    isOpen=false;

    ofstream *fcfstr=new ofstream();
    fcfstr->open(dbname.c_str(),std::ios::out);
    if(!fcfstr) {
//      isOpen=false;
// throw fstream open err
unlock();
    return false;
    }
    fcfstr->unsetf(ios::skipws);
//   while(!fcfstr->eof()) {
       (*fcfstr)<<dbBuff;
//   }

    fcfstr->close();
    delete fcfstr;
    res=true;
  }
  unlock();
  return res;
}

bool
db_query(int num, DbTable *d, char *derr) {
  return xmlAction(num,d,derr,SELECT_ROW);
}

char *db_function(int i,char *a,char *b) {
  return "";
}

bool db_get(int num,DbTable *d,int row,char *derr) {
  lock();
  cerr<<"db_get"<<endl;
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
  if(ctmp->resMap.find(row) == ctmp->resMap.end()) {
    unlock();
    return false;
  }
  int k=0;
  char name[100];
  for(int i=0; i<d->size(); i++) {
    if(!d->get_request(i)) { // if this is not request

      snprintf(name,sizeof(name)-1,"%s",d->get_name(i).c_str());
cerr<<"Request to db_get "<<d->get_name(i)<<"=";
      switch(d->get_type(i)) {
       case 1:  // bool type
 cerr<<ctmp->resMap[row].operator[](d->get_name(i))<<"=(1)"<<endl;
        if(ctmp->resMap[row].operator[](d->get_name(i)).c_str()[0] == 't')
          d->set(name,true);
        else
          d->set(name,false);
        break;
       case 2:  // int type
 cerr<<ctmp->resMap[row].operator[](d->get_name(i))<<"=(2)"<<endl ;      
        d->set(name,atoi(
	ctmp->resMap[row].operator[](d->get_name(i)).c_str()
	));
        break;
       case 3:  // float type
 cerr<<ctmp->resMap[row].operator[](d->get_name(i))<<"=(3)"<<endl  ;     
        d->set(name,(float)atof(
	ctmp->resMap[row].operator[](d->get_name(i)).c_str()
	));

        break;
       case 4:  // string type
 cerr<<ctmp->resMap[row].operator[](d->get_name(i))<<"=(4)"<<endl   ;    
        d->set(name,
	ctmp->resMap[row].operator[](d->get_name(i))
	);

        break;
       case 5:  // date type
//        d->set(name, iso2time((char*)sdb_get(ctmp->SQL_conn,ctmp->SQL_res,row,k).c_str()));
 cerr<<ctmp->resMap[row].operator[](d->get_name(i))<<"=(5)"<<endl;
        d->set(name,iso2time((char*)(
	ctmp->resMap[row].operator[](d->get_name(i)).c_str()
	)));

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

bool db_create(int num,DbTable *d, char *derr) {
cerr<<"db_create"<<endl;
  return xmlAction(num,d,derr,INSERT_TABLE);
}

bool db_create_idx(int i,DbTable *a,char *b) {
  return true;
}

bool db_add(int num,DbTable *d,char *derr) {
cerr<<"db_add"<<endl;
  return xmlAction(num,d,derr,INSERT_ROW);
}

bool db_update(int num,DbTable *d,char *derr) {
cerr<<"db_update"<<endl;
  return xmlAction(num,d,derr,UPDATE_ROW);
}

bool db_del(int num,DbTable *d,char *derr) {
cerr<<"db_del"<<endl;
  return xmlAction(num,d,derr,DELETE_ROW);
}

/********************************************************/
/*
void main () {
char a[]="sddf";
int num;
cerr<<"DbInit"<<endl;
db_init();
cerr<<"DbOpen"<<endl;
num=db_open(a,1,"./xml-db.xml",a,a,a);
cerr<<"DbClose"<<endl;
db_close(num);
cerr<<"DbDestroy"<<endl;
db_destroy();
}
*/

//#endif //DB_XML
