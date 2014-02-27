// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: Tci.cc
// description: Tacppd Control Interface

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"

#define TCI_TIMEOUT   60

TciServer::TciServer(int fh,struct in_addr addr) : Packet(fh) {
  char addrs[30];
  snprintf(addrs,sizeof(addrs)-1,"%s",inet_ntoa(addr));
  address.fill(addrs);
  in_address = addr;
  handle = fh;
}

TciServer::~TciServer() {}

string
TciServer::read() {
  unsigned char s[2];
  char c;
  char buf[120];
  bzero(buf,sizeof(buf));
  int i=0;
  bzero(s,sizeof(s));
  int resi = 0;
  while((resi=sockread(s, 1, TCI_TIMEOUT))>0 && i<100 &&
      s[0]!='\n' && s[0]!='\r') {
    c = s[0];
    if(c == '\xff') {
      if(sockread(s, 1, HTTP_TIMEOUT) == -1) break;
      bzero(s,sizeof(s));
      continue;
    }
    buf[i++] = s[0];
    bzero(s,sizeof(s));
  }
  string ostr;
  if(resi < 0) return ostr;
  if(s[0]=='\r') sockread(s, 1, HTTP_TIMEOUT);
  ostr = buf;
  return ostr;
}

bool
TciServer::write(string s) {
  string str;
  str = s + "\r\n";
  if(sockwrite((unsigned char *)str.c_str(),str.size(),HTTP_TIMEOUT) > 0) {
    return true;
  }
  return false;
}

void
TciServer::process() {
  string str;
  // 1. read first string, this is username for access
  string username = read();
  //std::cerr<<"TCI username="<<username<<std::endl;
  // 2. read second string, this is password for access
  string password = read();
  //std::cerr<<"TCI password="<<password<<std::endl;
  // 3. compare username/password with internal data
  if(!::coreData->getManager()->check((char*)username.c_str(),
      (char*)password.c_str(),(char*)address.get().c_str(),"tci")) {
    write("#:\tError");
    return;
  }
  write("#:\tOk");
  string command = "nocommand";
  while(command!="exit" && command!="quit" && command.size()>0) {
    // 4. read command
    command = read();
    if(command.size() == 0) break;
    std::cerr<<"TCI receive command: "<<command<<std::endl;

    // showusers
    if(command=="showusers") {
      str.erase();
      Device_data *dtmp = 0;
      DeviceUser_data utmp;
      write("#:\tBegin");
      for(int i=0; (dtmp = ::coreData->getDevice()->get(i)) != 0; i++) {
        str=str+"#:\tDevice\t"+dtmp->getHostaddr().getn()+"("+dtmp->getDescr()+")";
        write(str);
        str.erase();
        int jjj = 0;
        while(true) {
          utmp = dtmp->gets(jjj);
          if(!utmp.isPresent()) break;
          string ph = utmp.getPhone();
          if(ph.size()==0) ph = "unknown/unknown";
          str.erase();
          str = utmp.getUsername()+"\t"+utmp.getPort()+"\t"+
            ph+"\t"+utmp.getUserAddr().get();
          write(str);
          str.erase();
          jjj++;
        }
      }
      write("#:\tEnd");
    }
    // showdevices
    if(command == "showdevices") {
      Device_data *tmp = 0;
      write("#:\tBegin");
      for(int j = 0; (tmp = ::coreData->getDevice()->get(j))!=0; j++) {
        str.erase();
        str = tmp->getDescr()+"\t"+tmp->getHostaddr().getn()+"\t"+(tmp->getStatus()?"Up":"Down");
        write(str);
      }
      write("#:\tEnd");
    }
    // checkpwd
    if(command == "checkpwd") {
      string username = read();
      string password = read();
      write("#:\tOk");
    }
    // cryptpwd
    if(command == "cryptpwd") {
      string dbid = read();
      string username = read();
      string password = read();
      write("#:\tOk");
    }
    // clearuser
    if(command == "clearuser") {
      username = read(); // read username for clear
      Device_data *dtmp = 0;
      DeviceUser_data utmp;
      // search for users and drop all ports with this username
      for(int i=0; (dtmp=::coreData->getDevice()->get(i)) != 0; i++) {
        int j = 0;
        while(true) {
          utmp = dtmp->gets(j);
          if(!utmp.isPresent()) break;
          if(utmp.getUsername() == username) {    ::snmpSystem.drop((char*)dtmp->getHostaddr().get().c_str(),(char*)utmp.getPort().c_str());
          }
        }
      }
      write("#:\tOk");
    }
    // cleardeviceport
    if(command == "cleardeviceport") {
      string device = read();
      string port = read();
      ::snmpSystem.drop((char*)device.c_str(),(char*)port.c_str()); // drop port
      write("#:\tOk");
    }
    // --- database manipulation section ---
    // create tables in database
    if(command == "db_create") {
      write("#:\tBegin");
      write(::coreData->getDb()->create_all());
      write("#:\tEnd");
    }
    // add entry to database table
    if(command == "db_add") {
      string dbtable = read();
      write("#:\tBegin");

      write("#:\tEnd");
    }
    // del entry from database table
    if(command == "db_del") {
      string dbtable = read();
      write("#:\tBegin");

      write("#:\tEnd");
    }
    // modify entry in database table
    if(command == "db_modify") {
      string dbtable = read();
      write("#:\tBegin");

      write("#:\tEnd");
    }
    // show entry in database table
    if(command == "db_show") {
      string dbtable = read();
      write("#:\tBegin");

      write("#:\tEnd");
    }
    // --- configuration manipulation section ---
    // show config
    if(command == "cf_show") {
      write("#:\tBegin");
      write(::coreData->showcf((char*)username.c_str()));
      write("#:\tEnd");
    }
    // add config command
    if(command == "cf_command") {
      string cfcommand = read();
      write("#:\tBegin");

      write("#:\tEnd");
    }
    // store configuration in config file
    if(command == "cf_write") {
      write("#:\tBegin");
      char spath[256];
      snprintf(spath,sizeof(spath),"%s/etc/tacppd.conf",::MAINDIR);
      write(::coreData->writecf(spath,(char*)username.c_str()));
      write("#:\tEnd");
    }
  }
}

//////////////// that is all ///////////////////
