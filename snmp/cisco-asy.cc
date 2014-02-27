// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: cisco.cc
// description: SNMP module for handling async cisco access devices
//  (like 2511/2509)

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#ifndef _PTHREADS
#define   _PTHREADS   // enable STL pthread-safe code
#endif //_PTHREADS

#ifndef _REENTRANT
#define       _REENTRANT    // Pthread safe stuff in the stdlibs
#endif //_REENTRANT     // or you should compile with -pthread key

#include "snmp_module.h"
#include <iostream>

/// defines

// set of OIDs
#define SNMP_CISCO_tsLineUser       ".1.3.6.1.4.1.9.2.9.2.1.18"

///////////////// methods for processing //////////////////

// this is for attach this functions in dload system
extern "C" {
  bool getnas(SnmpDevice*,SnmpD*(*)(char*,char*,char*),char*);
  bool dropnas(SnmpDevice*,char*,SnmpD*(*)(char*,char*,char*),
    bool(*)(char*,char*,char*,char*,char),char*);
};

//// some useful funtrions for me /////

// compare string
bool
scomp(char *s1,char *s2) {
  if(!s1 && !s2) return true;
  if(!s1 || !s2) return false;
  if(strcmp(s1,s2) == 0) return true;
  return false;
}

// compare num symbols
bool
scomp(char *s1,char *s2,int num) {
  if(!s1 && !s2) return true;
  if(!s1 || !s2) return false;
  if((int)strlen(s1) < num || (int)strlen(s2) < num) return false;
  if(strncmp(s1,s2,num) == 0) return true;
  return false;
}  

//////////////////// Cisco /////////////////////
//
// support for 2509/2511 and AS5300 (120 and 240 ports)
// don't work with 2522 and 2600/3600
bool
getnas(SnmpDevice *sd,SnmpD*(*get)(char*,char*,char*),char *derr) {
  SnmpOut *tmp;
  SnmpD *snmp_ifName = 0;
  SnmpD *snmp_tsLineUser = 0;
  SnmpD *snmp_cpmActiveUserID = 0;
  SnmpD *snmp_cpmActiveTTYNumber = 0;
  SnmpD *snmp_cpmActiveEntryChannel = 0;
  SnmpD *snmp_cpmActiveEntryPort = 0;
  bool aaa = false;
  char portname[100];

  if(!(snmp_ifName=get((char*)sd->getDevice().c_str(),
        (char*)sd->getCommun().c_str(),SNMP_ifName)) ) {
    snprintf(derr,SNMP_ERROR_MSG_LEN,
      "NAS %s not responded by SNMP, check community",sd->getDevice().c_str());
    return false;
  }

  snmp_tsLineUser=get((char*)sd->getDevice().c_str(),(char*)sd->getCommun().c_str(),
    SNMP_CISCO_tsLineUser);

  // Ok. All readed
  snmp_d *res_ifName = 0;
  snmp_d *res_tsLineUser = 0;

  // 2. - work with tsLineUser, add it in before results
  // for async ports on routers like 2509/2511
  for(int i=0,j=2;(res_ifName = snmp_ifName->get(i)) != 0 &&
      (res_tsLineUser = snmp_tsLineUser->get(j)) != 0 ; i++,j++) {
    // move to 2 (don't understand, but working :( )
    while(res_ifName && !scomp((char*)res_ifName->res.c_str(),"As",2)) {
      res_ifName = snmp_ifName->get(i++);
    }
    if(!res_ifName) break;

    if(res_tsLineUser->res.size()==0) {
      continue;
    }

    // search port
    aaa = false;
    for(int m=0; m<sd->size(); m++) {
      if(sd->get(m)->getPort() == res_ifName->res) {
        aaa = true;
        break;
      }
    }
cerr<<"2: see user="<<res_tsLineUser->res<<" port="<<res_ifName->res<<endl;
    if(!aaa && res_ifName->res.size()>0) {
      tmp = new SnmpOut;
      tmp->setUser(res_tsLineUser->res);
      tmp->setPort(res_ifName->res);
cerr<<"2: add user="<<res_tsLineUser->res<<" port="<<res_ifName->res<<endl;
      sd->add(tmp);
    }
  }
  if(snmp_ifName) delete snmp_ifName;
  if(snmp_tsLineUser) delete snmp_tsLineUser;

  derr[0] = '\0';
  return true;
}

// drop nas port
bool
dropnas(SnmpDevice *sd,char *p,SnmpD*(*get)(char*,char*,char*),
      bool(*puts)(char*,char*,char*,char*,char),char *derr) {
  SnmpD *snmp_ifName = 0;
  char port[50];
 try {
  // change tty to As  (this is tacpp internal port naming)
  if(scomp("tty",p,3))
    snprintf(port,sizeof(port)-1,"As%s",p+3);
  else
    snprintf(port,sizeof(port)-1,"%s",p);

  if(!(snmp_ifName=get((char*)sd->getDevice().c_str(),
        (char*)sd->getCommun().c_str(),SNMP_ifName))) {
    snprintf(derr,SNMP_ERROR_MSG_LEN,
      "ifName returns false, check host %s for community",
        sd->getDevice().c_str());
    throw false;
  }
  int idx = -1;
  snmp_d *res = 0;
  for(int j=0; (res = snmp_ifName->get(j)) != 0; j++) {
    if(res->res == port) idx = j;
    //res = res->next;
  }
  if(idx == -1) {
    snprintf(derr,SNMP_ERROR_MSG_LEN,
      "no ifIndex/ifName information, check host %s",
        sd->getDevice().c_str());
    throw false;
  }
  char str[100];
  idx = idx + 1;
  snprintf(str,sizeof(str)-1,"%s.%d",SNMP_ifAdminStatus,idx);

  puts((char*)sd->getDevice().c_str(),(char*)sd->getCommun().c_str(),str,"2",'i');  // set port to down
  sleep(1); // some delay
  puts((char*)sd->getDevice().c_str(),(char*)sd->getCommun().c_str(),str,"1",'i');  // set port to up
  throw true;
 }
 catch(bool res) {
  if(snmp_ifName) delete snmp_ifName;
  return res;
 }
}

/////////////// end of processing methods ////////////////
