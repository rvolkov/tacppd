// tacppd Authorization pairs modifyer
// (c) Copyright 2000-2007 by Roman Volkov and contributors
// See http://tacppd.org for more information

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// this is system for on-fly authorization values
// modification, for example, if you need to use internal
// IP addr pool system you can use entry like this:
//  addr=INT:addrpool=1
// or you can use additional SQL queryes:
//  timeout=SQL:sqlfunction()
// or if you completely mad, use external program
//  cmd=EXT:getcommand.sh

#include "global.h"

///// AModify_INT

// internal modification functions
// functions support:
//    - addrpool=pool_number
string
AModify_INT::modifyer(const char *s) {
  string str;
  string CValue;

  //// place for internal modifyer functions
  // all functiona have code like that:
  // if(utl::scomp(s,"FUNCTION_NAME=",LENGTH)) {
  //   CValue<<utl::getAVPValue<<end();		// this is right value
  //   str<<"SET NEEDED VALUE"<<end();
  // }

  // internal address pool
  if(utl::scomp(s,"addrpool=",9)) {		// addrpool= function
    CValue = CValue + utl::getAvpValue(s);
    int poolnum = atoi(CValue.c_str());
    if(poolnum > 0)
      str = str + ::coreData->getPool()->get(poolnum);
    else
      str = str + "0.0.0.0";
  }
  // place for the next internal function

  //
  return str;
}

///// AModify_SQL

// external SQL request
// any SQL request, but process only first row in response
string
AModify_SQL::modifyer(const char *s) {
  string str;
  str = str + ::coreData->getDb()->direct_function_query((char*)s);
  //delete [] ss;
  std::cerr<<"Call SQL modifyer with data:"<<s<<" and get result="<<str<<std::endl;
  return str;
}

///// AModify_EXT

// 0. Close the two given file-descruptors
void
close_fds(int fd1, int fd2) {
  if(fd1 >= 0) close(fd1);      // read fd
  if(fd2 >= 0) close(fd2);      // error fd
}

struct ext_data {
  char cmd[200];    // program exec str
  int fd;           // file descr
};

// 1. thread
void extRunner(struct ext_data *d) {
std::cerr<<"Start execl in thread"<<std::endl;
  // change stdout
  int o = dup(STDOUT_FILENO);
  int n = dup(d->fd);
  dup2(n,STDOUT_FILENO);
  if(execl("/bin/sh", "sh", "-c", d->cmd, (char *)0) == -1) {
    startlog(LOG_ERROR)<<"EXT: execl returns -1 for "<<d->cmd<<endlog;
  }
  dup2(o,STDOUT_FILENO);    // set stdout back
  close(d->fd);
  pthread_exit(0);
}

// 2. work
// external program request
string
AModify_EXT::modifyer(const char *cmd) {
  string out;
  int fd[2]; // stdin (fd[0]-read fd[1]-write)
  fd[0] = fd[1] = -1;
  if(pipe(fd) < 0) {
    startlog(LOG_ERROR)<<"EXT: Cannot create pipes for "<<cmd<<endlog;
    close_fds(fd[0], fd[1]); // close fds
    return out;
  }
  struct ext_data *extd = new struct ext_data;
  snprintf(extd->cmd,sizeof(extd->cmd)-1,"%s",cmd);
  extd->fd = fd[1];
//std::cerr<<"EXT: run cmd = "<<cmd<<std::endl;
  pthread_t thrvar;
  pthread_create(&thrvar,NULL,(void*(*)(void*))&extRunner,(void*)extd);
  char buf[255], *bufp, c;
  bzero(buf,sizeof(buf));
  bufp = buf;
  while(read(fd[0], &c, 1) > 0) *bufp++ = c;
  *bufp = '\0';
  out = out + buf;
  pthread_join(thrvar,NULL);

  delete extd;
  close_fds(fd[0], fd[1]);
//std::cerr<<"EXT: readed output = "<<out<<std::endl;
  return out;
}

////// AModify

// constructor
AModify::AModify(const string u,const ipaddr n,const string p,const string r) {
  name = u;
  nas = n;
  port = p;
  phone = r;
  pthread_mutex_init(&Mutex_,0);
  pthread_mutex_lock(&Mutex_);
}

// destr
AModify::~AModify() {
  pthread_mutex_unlock(&Mutex_);
  pthread_mutex_destroy(&Mutex_);
}

string
AModify::setvars(const string sc) {
  char str[250];
  char c[300];
  snprintf(c,sizeof(c)-1,"%s",sc.c_str());
  bzero(str,sizeof(str));
  int k = 0;
  for(int i=0; c[i]!=0 && k<(int)sizeof(str); i++) {
    if(c[i] == '$') { // mark
      if(utl::scomp(c+i,"$name",5)) { // $name variable
        for(int j=0; name.c_str()[j]!=0; j++) str[k++] = name.c_str()[j];
        i += 5;
      }
      if(utl::scomp(c+i,"$nas",4)) { // $nas variable
        for(int j=0; nas.get()[j]!=0; j++) str[k++] = nas.get()[j];
        i += 4;
      }
      if(utl::scomp(c+i,"$port",5)) { // $port variable
        for(int j=0; port.c_str()[j]!=0; j++) str[k++] = port.c_str()[j];
        i += 5;
      }
      if(utl::scomp(c+i,"$phone",6)) { // $port variable
        for(int j=0; phone.c_str()[j]!=0; j++) str[k++] = phone.c_str()[j];
        i += 6;
      }
    }
    str[k++] = c[i];
  }
  string outs(str);
  return outs;
}

// select appropriate method for abstraction layer
//
// ToDo - if modifyer will return several values...
//   we should create several AV pairs...         Cool ? :)
db_user
AModify::modifyer(const db_user ax) {
  db_user d = ax;
  user_avp avp;
  // go throught db_user's authorization AVPs structure
//  for(list<user_avp>::iterator i=d.avp.begin();i!=d.avp.end(); i++) {
//    avp = *i;

  for(int i = 0; i < d.avp_size(); i++) {
    avp = d.avp_get(i);

    // work with avp
    string AValue;
    string Attr;
    string Command;
    Attr = Attr + utl::getAvpName(avp.getAvp().c_str());
    AValue = AValue + utl::getAvpValue(avp.getAvp().c_str());

    am = 0;
    // check for internal name INT, SQL or EXT
    if(utl::scomp(AValue.c_str(),"INT:",4)) {// internal function
      Command = Command + utl::getAvpMod(AValue.c_str());
      am = new AModify_INT(); // processing
    }
    if(utl::scomp(AValue.c_str(),"SQL:",4)) {	// SQL function
      Command = Command + utl::getAvpMod(AValue.c_str());
      am = new AModify_SQL(); // call SQL command
    }
    if(utl::scomp(AValue.c_str(),"EXT:",4)) {	// external program
      Command = Command + ::MAINDIR + "/ext/" + utl::getAvpMod(AValue.c_str());
      am = new AModify_EXT(); // call external program
    }
    // now work
    if(am) { // Aha!!! This is here....
      // 1. set variables values in Command
      char NewCommand[250];
      snprintf(NewCommand,sizeof(NewCommand),"%s",
        setvars(Command).c_str());
      // 2. call modifyer and create changed AV pair
      avp.setAvp(Attr + "=" + am->modifyer(NewCommand));

      //*i = avp;
      d.avp_set(i,avp);
      i = 0;
//    snprintf(d.avp[k].avp,sizeof(d.avp[k].avp),"%s=%s",
//      Attr.c_str(),am->modifyer(NewCommand)); // change to new value
      delete am; // free for next try
    }
  }
//std::cerr<<"Point AM"<<std::endl;
  return d; // i think, that it will be all....
}

//////////// that's all, folks ////////////
