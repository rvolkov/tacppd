// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: Snmp.cc
// description: work with SNMP

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

/*
This software use NET-SNMP package which have next notice:


Various copyrights apply to this package, listed in 2 seperate parts
below.  Please make sure that you read both parts.  The first part is
the copyright for all the code up until 2001, and the second part
applies to the derivative work done since then.

---- Part 1: CMU/UCD copyright notice: (BSD like) -----


       Copyright 1989, 1991, 1992 by Carnegie Mellon University

      Derivative Work - 1996, 1998-2000
Copyright 1996, 1998-2000 The Regents of the University of California

       All Rights Reserved

Permission to use, copy, modify and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU and The Regents of
the University of California not be used in advertising or publicity
pertaining to distribution of the software without specific written
permission.

CMU AND THE REGENTS OF THE UNIVERSITY OF CALIFORNIA DISCLAIM ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL CMU OR
THE REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
FROM THE LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


---- Part 2: NAI copyright notice (BSD) -----

Copyright (c) 2001, NAI Labs
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

*  Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

*  Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

*  Neither the name of the NAI Labs nor the names of its contributors
   may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "global.h"

//////// utilities for snmp_get & snmp_puts

// snmp get
SnmpD*
snmp_get(char *hostname, char *community, char *object) {
  /////
  SnmpD *out = 0;
  out = new SnmpD();
  /////
  snmp_d *output_tmp = 0;
  char str_tmp[500];
  unsigned char *ip;
#ifndef NOUSESNMP
  struct snmp_session session;
  struct snmp_session *ss = 0;
  struct snmp_pdu *pdu = 0;
  struct snmp_pdu *response = 0;
  struct variable_list *vars;
  int dest_port = SNMP_PORT;
  oid root[MAX_NAME_LEN];
  int rootlen;
  oid name[MAX_NAME_LEN];
  int name_length;
  int running, status;//, count;
  int idx = 1;

  signal(13,SIG_IGN); // ignore SIGPIPE (i dont shure in that)
#endif

 try {
#ifndef NOUSESNMP
//  init_mib(); // mib tree initialization
  rootlen = MAX_NAME_LEN;
  read_objid(object, root, (size_t*)&rootlen);

  memset(&session, 0, sizeof(struct snmp_session));
  session.peername = hostname; // store hostname
  session.remote_port = dest_port; // port
  session.retries = SNMP_DEFAULT_RETRIES; // retries number
  session.timeout = 2000000L; // timeout
  session.authenticator = 0;
  // for SNMP v.1
  session.version = SNMP_VERSION_1; // set snmp version to 1
  session.community = (unsigned char *)community; // store community
  session.community_len = strlen(community);
  // setup SNMP
  //snmp_synch_setup(&session); // ??????????

  SOCK_STARTUP;
  if(!(ss=snmp_open(&session))) { // open session
    startlog(LOG_ERROR)<<"Can't open snmp for device "<<hostname<<endlog;
    throw 0;
  }
  memmove(name, root, rootlen * sizeof(oid));
  name_length = rootlen;
  running = 1;
  while(running) {
    running = 0;
    pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
    snmp_add_null_var(pdu, name, name_length);
    status = snmp_synch_response(ss, pdu, &response);
    if(status == STAT_SUCCESS) {
      if(response->errstat == SNMP_ERR_NOERROR) {
        for(vars=response->variables; vars; vars=vars->next_variable) {
          if((int)vars->name_length < rootlen ||
              memcmp(root,vars->name,rootlen * sizeof(oid))) {
            continue; // not part of this subtree
          }
          output_tmp = 0;
          if(!vars->val.string ||
              !utl::scomp((char*)vars->val.string,"tcp")) {
            if(!vars->val.string) { // if void return
              output_tmp = new snmp_d("",idx);
              //if(!output) {
              //  output = output_tmp;
              //  output_start = output;
              //} else {
              //  output_start->next = output_tmp;
              //  output_start = output_start->next;
              //}
            } else {
              // convert to char*
              bzero(str_tmp,sizeof(str_tmp));
              switch(vars->type) {
                case ASN_INTEGER:
                  snprintf(str_tmp,sizeof(str_tmp)-1,"%ld",*vars->val.integer);
                  break;
                case ASN_OCTET_STR:
                  snprintf(str_tmp,vars->val_len+1,"%s",
                    (char*)vars->val.string);
                  break;
                case ASN_BIT_STR:
                  snprintf(str_tmp,vars->val_len+1,"%s",
                    (char*)vars->val.string);
                  break;
//              case ASN_OPAQUE:
//                break;
//              case ASN_OBJECT_ID:
//                break;
//              case ASN_TIMETICKS:
//                break;
                case ASN_GAUGE:
                case ASN_COUNTER:
                case ASN_UINTEGER:
                  snprintf(str_tmp,sizeof(str_tmp)-1,"%lu",*vars->val.integer);
                  break;
                case ASN_IPADDRESS:
                  ip = vars->val.string;
                  snprintf(str_tmp,sizeof(str_tmp)-1,"%d.%d.%d.%d",
                    ip[0],ip[1],ip[2],ip[3]);
                  break;
                case ASN_NULL:
                  snprintf(str_tmp,sizeof(str_tmp)-1,"NULL");
                  break;
//              case ASN_COUNTER64:
//                break;
                default:
                  snprintf(str_tmp,vars->val_len+1,"%s",
                    (char*)vars->val.string);
                  break;
              }
              // push datas
              output_tmp = new snmp_d(str_tmp,idx);
              //if(!output) {
              //  output = output_tmp;
              //  output_start = output;
              //} else {
              //  output_start->next = output_tmp;
              //  output_start = output_start->next;
              //}
            }
          } else {
            // this is digit 0
            output_tmp = new snmp_d("0",idx);
            //if(!output) {
            //  output = output_tmp;
            //  output_start = output;
            //} else {
            //  output_start->next = output_tmp;
            //  output_start = output_start->next;
            //}
          }
          if(vars->type != SNMP_ENDOFMIBVIEW
            && vars->type != SNMP_NOSUCHOBJECT // for robustness
              && vars->type != SNMP_NOSUCHINSTANCE) {
            memmove((char *)name, (char *)vars->name,
              vars->name_length * sizeof(oid));
            name_length = vars->name_length;
            running = 1; // restart so we can get next variable
          }
          //if(output_start) {
          //  output_start->idx = (int)name[name_length-1];
          //  if(name_length > 1)
          //   output_start->idx1 = (int)name[name_length-2];
          //}
          if(output_tmp) {
            output_tmp->idx = (int)name[name_length-1];
            if(name_length > 1) {
              output_tmp->idx1 = (int)name[name_length-2];
            }
            out->add(output_tmp);
          }
        }
      } else {
        if(response->errstat == SNMP_ERR_NOSUCHNAME) {
          startlog(LOG_ERROR)<<"SNMP: End of MIB."<<endlog;
        } else {
          startlog(LOG_ERROR)<<"SNMP: Error in packet. Reason: "<<snmp_errstring(response->errstat)<<endlog;
          if(response->errstat == SNMP_ERR_NOSUCHNAME) {
            startlog(LOG_ERROR)<<"SNMP: The request for object identifier failed"<<endlog;
//            for(count = 1, vars = response->variables; vars &&
//              count != response->errindex;
//                vars = vars->next_variable, count++);
            //if (vars) print_objid(vars->name, vars->name_length);
          }
        }
      }
    } else if(status == STAT_TIMEOUT) {
      startlog(LOG_ERROR)<<"SNMP: no response from "<<hostname<<endlog;
      throw 0;
    } else { // status == STAT_ERROR
      startlog(LOG_ERROR)<<"SNMP: error occurred."<<endlog;
      throw 0;
    }
    if(response) {
      snmp_free_pdu(response);
      response = 0;
    }
  }
#endif
  throw 0;
 }
 catch(int i) {
#ifndef NOUSESNMP
  if(response) snmp_free_pdu(response);
  if(ss) snmp_close(ss);
  SOCK_CLEANUP;
#endif
  return out;
 }
}

// write
bool
snmp_puts(char *hostname,char *community,
      char *object, char *value, char type) {
#ifndef NOUSESNMP
  struct snmp_session session, *ss = 0;
  struct snmp_pdu *pdu = 0, *response = 0;
  struct variable_list *vars;
  oid root[MAX_NAME_LEN];
  int rootlen;
  oid name[MAX_NAME_LEN];
  int name_length;
  int dest_port = SNMP_PORT;
  int status;

  signal(13,SIG_IGN); // ignore SIGPIPE (i don't shure in this)
#endif

 try {
#ifndef NOUSESNMP
  rootlen = MAX_NAME_LEN;
  read_objid(object, root, (size_t*)&rootlen);
  // set snmp parameters
  memset(&session, 0, sizeof(struct snmp_session));
  session.peername = hostname;
  session.remote_port = dest_port;
  session.retries = SNMP_DEFAULT_RETRIES;
  session.timeout = 2000000L;
  session.authenticator = NULL;
  // for SNMP v.1
  session.version = SNMP_VERSION_1;
  session.community = (unsigned char *)community;
  session.community_len = strlen((char *)community);
  // open SNMP session
  //  snmp_synch_setup(&session);	// ?????????
  SOCK_STARTUP;
  if(!(ss=snmp_open(&session))) {
    startlog(LOG_ERROR)<<"Error in open snmp to "<<hostname<<endlog;
    throw 0;
  }
  memmove(name, root, rootlen * sizeof(oid));
  name_length = rootlen;
  // create PDU for SET request and add object names and values to
  // request
  pdu = snmp_pdu_create(SNMP_MSG_SET);
  if(snmp_add_var(pdu, name, name_length, type, value)) {
    startlog(LOG_ERROR)<<"Error in snmp_add_var"<<endlog;
    throw 0;
  }
  // do the request
retry:
  status = snmp_synch_response(ss, pdu, &response);
  if(status == STAT_SUCCESS) {
    if(response->errstat == SNMP_ERR_NOERROR) {
      for(vars = response->variables; vars; vars = vars->next_variable)
        print_variable(vars->name, vars->name_length, vars);
    } else {
      startlog(LOG_ERROR)<<"SNMP: Error in packet. Reason: "<<
        snmp_errstring(response->errstat)<<endlog;
      if(response->errstat == SNMP_ERR_NOSUCHNAME)
        vars = response->variables;
//      if(pdu == snmp_fix_pdu(response, SNMP_MSG_SET)) goto retry;
      if((pdu = snmp_fix_pdu(response, SNMP_MSG_SET))) {
        snmp_free_pdu(response);  // NEW!!!
        goto retry;
      }
    }
  } else if(status == STAT_TIMEOUT) {
    startlog(LOG_ERROR)<<"SNMP:  Timeout: no response from "<<session.peername<<endlog;
    throw 0;
  } else { // status == STAT_ERROR
    throw 0;
  }
#endif
  throw 1;
 }
 catch(int i) {
#ifndef NOUSESNMP
  if(response) snmp_free_pdu(response);
  if(ss) snmp_close(ss);
  SOCK_CLEANUP;
  if(i == 0) return false;
#endif
  return true;
 }
}

/////////////////// class Snmp ///////////////////

// constructor
Snmp::Snmp() {
  count = 0;
  pthread_mutex_init(&Mutex,0);
#ifndef NOUSESNMP
  init_mib(); // mib tree initialization
#endif
}

// destructor
Snmp::~Snmp() {
#ifndef NOUSESNMP
  shutdown_mib();
#endif
  pthread_mutex_destroy(&Mutex);
}

void
Snmp::lock() {
  pthread_mutex_lock(&Mutex);
}

void
Snmp::unlock() {
  pthread_mutex_unlock(&Mutex);
}

// drop user
bool
Snmp::drop(char *nas, char *port) {
  if(!nas || !port) return false;
  if(strlen(nas)==0 || strlen(port)==0) return false;
  bool ret = false;
  startlog(LOG_SNMP)<<"REQUEST TO DROP "<<nas<<"/"<<port<<endlog;
  char derr[SNMP_ERROR_MSG_LEN+1];
  bzero(derr,sizeof(derr));
 try {
  //Device_data *tmp = 0;
  ipaddr ip(nas);
  //tmp = ::coreData->getDevice()->get(ip);
  string cmn = ::coreData->getDevice()->getCommunity(ip);
  if(cmn.size() > 0) {
    lock();
    SnmpDevice *sd = new SnmpDevice((char*)ip.get().c_str(),(char*)cmn.c_str());
    Device_data *tmp = 0;
    tmp = ::coreData->getDevice()->get(ip);
    if(tmp)
      ret = tmp->dropnas(sd,(char*)utl::portConvert(port).c_str(),snmp_get,snmp_puts,derr);
    unlock();
    throw ret;
  }
  startlog(LOG_ERROR)<<"SNMP_DROP: Can't find NAS "<<nas<<" (no such NAS)"<<endlog;
  throw false;
 }
 catch(bool r) {
  if(strlen(derr)>0) startlog(LOG_ERROR)<<"SNMP drop: "<<derr<<endlog;
  if(r) return true;
  else return false;
 }
}

// get information by snmp from nas
SnmpDevice*
Snmp::get(Device_data *tmp) {
  if(!tmp) return 0;
  lock();
  char derr[SNMP_ERROR_MSG_LEN+1];
  bzero(derr,sizeof(derr));
  SnmpDevice *sd = new SnmpDevice((char*)tmp->getHostaddr().get().c_str(),(char*)tmp->getCommunity().c_str());
  bool ret = tmp->getnas(sd,snmp_get,(char*)derr);
  unlock();
  if(ret) return sd;
  delete sd;
  startlog(LOG_SNMP)<<"SNMP error: "<<derr<<endlog;
  return 0;
}

// store traffic in database
void
Snmp::store_traffic(Device_data *tmp, SnmpDevice *sd) {
//if(tmp->counter) ::coreData->getDb()->add_trf(sd);
  return;
}

//////////////////// that's all ////////////////////////
