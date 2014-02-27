// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: cisco.cc
// description: SNMP module for handling cisco devices

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// Support will not be provided under any circumstances for this program by
// tacppd.com, it's employees, volunteers or directors, unless a commercial
// support agreement and/or explicit prior written consent has been given.
// Visit http://tacppd.com for more information

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

#define SNMP_CISCO_cpmActiveUserID  ".1.3.6.1.4.1.9.10.19.1.3.1.1.3"
#define SNMP_CISCO_cpmUserID        ".1.3.6.1.4.1.9.10.19.1.4.3.1.4"

#define SNMP_CISCO_cpmActiveTTYNumber ".1.3.6.1.4.1.9.10.19.1.3.1.1.14"
#define SNMP_CISCO_cpmTTYNumber       ".1.3.6.1.4.1.9.10.19.1.4.3.1.15"

#define SNMP_CISCO_cpmAssociatedInterface ".1.3.6.1.4.1.9.10.19.1.1.1.1.13"

#define SNMP_CISCO_cpmEntryPort       ".1.3.6.1.4.1.9.10.19.1.4.3.1.11"
#define SNMP_CISCO_cpmActiveEntryPort ".1.3.6.1.4.1.9.10.19.1.3.1.1.10"

#define SNMP_CISCO_cpmEntryChannel        ".1.3.6.1.4.1.9.10.19.1.4.3.1.12"
#define SNMP_CISCO_cpmActiveEntryChannel  ".1.3.6.1.4.1.9.10.19.1.3.1.1.11"

///////////////// methods for processing //////////////////

// this is for attach this functions in dload system
extern "C" {
  bool getnas(SnmpDevice*,snmp_d*(*)(char*,char*,char*),char*);
  bool dropnas(SnmpDevice*,char*,snmp_d*(*)(char*,char*,char*),
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
// use ifInOctets/ifOutOctets for traffic count
bool
getnas(SnmpDevice *sd,snmp_d*(*get)(char*,char*,char*),char *derr) {
  SnmpOut *tmp;
  SnmpD *snmp_ifName = 0;
  SnmpD *snmp_ifInOctets = 0;
  SnmpD *snmp_ifOutOctets = 0;
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
  snmp_cpmActiveUserID=get((char*)sd->getDevice().c_str(),(char*)sd->getCommun().c_str(),
    SNMP_CISCO_cpmActiveUserID);
  snmp_cpmActiveTTYNumber=get((char*)sd->getDevice().c_str(),(char*)sd->getCommun().c_str(),
    SNMP_CISCO_cpmActiveTTYNumber);
  snmp_cpmActiveEntryChannel=get((char*)sd->getDevice().c_str(),(char*)sd->getCommun().c_str(),
    SNMP_CISCO_cpmActiveEntryChannel);
  snmp_cpmActiveEntryPort=get((char*)sd->getDevice().c_str(),(char*)sd->getCommun().c_str(),
    SNMP_CISCO_cpmActiveEntryPort);
  snmp_ifInOctets = get((char*)sd->getDevice().c_str(),
        (char*)sd->getCommun().c_str(),SNMP_ifInOctets);
  snmp_ifOutOctets = get((char*)sd->getDevice().c_str(),
          (char*)sd->getCommun().c_str(),SNMP_ifOutOctets)

  // Ok. All readed
  snmp_d *res_ifName = 0;
  snmp_d *res_ifInOctets = 0;
  snmp_d *res_ifOutOctets = 0;
  snmp_d *res_tsLineUser = 0;
  snmp_d *res_cpmActiveUserID = 0;
  snmp_d *res_cpmActiveTTYNumber = 0;
  snmp_d *res_cpmActiveEntryChannel = 0;
  snmp_d *res_cpmActiveEntryPort = 0;

  // 1. - work with cpmActiveUserID/cpmActiveTTYNumber/cpmActiveEntryPort/
  // cpmActiveEntryChannel (for AS5300)
  for(int i=0;(res_cpmActiveUserID = snmp_cpmActiveUserID->get(i)) != 0 &&
      (res_cpmActiveTTYNumber = snmp_cpmActiveTTYNumber->get(i)) != 0 &&
       (res_cpmActiveEntryPort = snmp_cpmActiveEntryPort->get(i)) != 0 &&
        (res_cpmActiveEntryChannel = snmp_cpmActiveEntryChannel->get(i)) != 0; i++) {

    bzero(portname,sizeof(portname));
    if(res_cpmActiveTTYNumber->res == "-1") {
      snprintf(portname,sizeof(portname)-1,"Se%s:%d",
        res_cpmActiveEntryPort->res.c_str(),
          atoi(res_cpmActiveEntryChannel->res.c_str()) - 1);  // i do now know why, but
    } else {
      snprintf(portname,sizeof(portname)-1,"As%s",
        res_cpmActiveTTYNumber->res.c_str());
    }

    // anything already inserted
    aaa = false;
    for(int i=0; i<sd->size(); i++) {
      if(sd->get(i)->getPort() == portname) {
        aaa = true;
        break;
      }
    }

    if(!aaa && strlen(res_cpmActiveUserID->res)>0) {
      tmp = new SnmpOut;
      tmp->setUser(res_cpmActiveUserID->res);
      tmp->setPort(portname);
      tmp->setIfIO(0);
      tmp->setIfOO(0);
      sd->add(tmp); // add entry to list
    }

    if(!aaa && res_cpmActiveUserID->res.size()>0) {
      tmp = new SnmpOut;
      tmp->setUser(res_cpmActiveUserID->res);
      tmp->setPort(portname);
//cerr<<"1: add user="<<res_cpmActiveUserID->res<<" port="<<portname<<endl;
      tmp->setIfIO(0);
      tmp->setIfOO(0);
      sd->add(tmp); // add entry to list
    }
  }

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

  // 3. - all that else
  for(int i=0;(res_ifName = snmp_ifName->get(i)) != 0; i++) {
    // no in upper results
    if(!aaa) {
      tmp = new SnmpOut;
      tmp->setUser("");
      tmp->setPort(res_ifName->res);
      sd->add(tmp);
    }
  }


  res_ifName = snmp_ifName;
  res_ifInOctets = snmp_ifInOctets;
  res_ifOutOctets = snmp_ifOutOctets;

  while(res_ifName) {
    if(res_ifInOctets && res_ifOutOctets &&
        res_ifName->idx == res_ifInOctets->idx &&
        res_ifName->idx == res_ifOutOctets->idx) {
      // check before added data + add traffic information
      aaa = false;
      for(int i=0; i<sd->size(); i++) {
        if(sd->get(i)->getPort() == res_ifName->res) {
          aaa = true;
          sd->get(i)->setIfIO(atol(res_ifInOctets->res));
          sd->get(i)->setIfOO(atol(res_ifOutOctets->res));
          break;
        }
      }
      // no in upper results
      if(!aaa) {
        tmp = new SnmpOut;
        tmp->setUser("");
        tmp->setPort(res_ifName->res);
        tmp->setIfIO(atol(res_ifInOctets->res));
        tmp->setIfOO(atol(res_ifOutOctets->res));
        sd->add(tmp);
      }
      if(res_ifInOctets) res_ifInOctets = res_ifInOctets->next;
      if(res_ifOutOctets) res_ifOutOctets = res_ifOutOctets->next;
    }
    res_ifName = res_ifName->next;
  }
  if(snmp_ifName) delete snmp_ifName;
  if(snmp_ifInOctets) delete snmp_ifInOctets;
  if(snmp_ifOutOctets) delete snmp_ifOutOctets;
  if(snmp_tsLineUser) delete snmp_tsLineUser;
  if(snmp_cpmActiveUserID) delete snmp_cpmActiveUserID;
  if(snmp_cpmActiveTTYNumber) delete snmp_cpmActiveTTYNumber;
  if(snmp_cpmActiveEntryChannel) delete snmp_cpmActiveEntryChannel;
  if(snmp_cpmActiveEntryPort) delete snmp_cpmActiveEntryPort;

  derr[0] = '\0';
  return true;
}

// drop nas port
bool
dropnas(SnmpDevice *sd,char *p,snmp_d*(*get)(char*,char*,char*),
      bool(*puts)(char*,char*,char*,char*,char),char *derr) {
  snmp_d *snmp_ifName = 0;
  char port[50];
 try {
  // change tty to As  (this is tacpp internal port naming)
  if(scomp("tty",p,3))
    snprintf(port,sizeof(port)-1,"As%s",p+3);
  else
    snprintf(port,sizeof(port)-1,"%s",p);

  if(!(snmp_ifName=get((char*)sd->getDevice().c_str(),
      (char*)sd->getCommun().c_str(),SNMP_ifName))) {
    snprintf(derr,sizeof(derr),
      "ifName returns false, check host %s for community",
        sd->getDevice().c_str());
    throw false;
  }
  int idx = -1;
  snmp_d *res = snmp_ifName;
  for(int j=0; res; j++) {
    if(scomp(res->res,port)) idx = j;
    res = res->next;
  }
  if(idx == -1) {
    snprintf(derr,sizeof(derr),
      "no ifIndex/ifName information, check host %s",
        sd->getDevice().c_str());
    throw false;
  }
  char str[100];
  idx = idx + 1;
  snprintf(str,sizeof(str)-1,"%s.%d",SNMP_ifAdminStatus,idx);

  puts((char*)sd->getDevice().c_str(),
      (char*)sd->getCommun().c_str(),str,"2",'i');  // set port to down
  sleep(1); // some delay
  puts((char*)sd->getDevice().c_str(),
      (char*)sd->getCommun().c_str(),str,"1",'i');  // set port to up
  throw true;
 }
 catch(bool res) {
  if(snmp_ifName) delete snmp_ifName;
  return res;
 }
}

/////////////// end of processing methods ////////////////
