// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: Tacacs.cc
// description: tacacs+ low-level functions

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

/*
This software can have parts of code from Cisco's tac_plus daemon
(i do not remember what i used from it, may be some for
authorization processing) with next copyright notice:

  Copyright (c) 1995-2000 by Cisco systems, Inc.

  Permission to use, copy, modify, and distribute modified and
  unmodified copies of this software for any purpose and without fee is
  hereby granted, provided that (a) this copyright and permission notice
  appear on all copies of the software and supporting documentation, (b)
  the name of Cisco Systems, Inc. not be used in advertising or
  publicity pertaining to distribution of the program without specific
  prior permission, and (c) notice be given in supporting documentation
  that use, modification, copying and distribution is by permission of
  Cisco Systems, Inc.

  Cisco Systems, Inc. makes no representations about the suitability
  of this software for any purpose.  THIS SOFTWARE IS PROVIDED ``AS
  IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
  WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
  FITNESS FOR A PARTICULAR PURPOSE.
*/

// modifications:
//

#include "global.h"


///// TacacsSessionContainer

TacacsSessionContainer::TacacsSessionContainer(ipaddr i,int f,int s,char *k,
        char *l,char *p,bool a) {
  pthread_mutex_init(&mutex_,0);
  pthread_mutex_init(&mutex2_,0);
  lock();
  loginstring_ = l;
  pwdstring_ = p;
  authoriz_ = a;
  created_ = time(0); // time, when session was created
  session_id_ = s;
  key_ = k;
  sock_ = f;
  peer_ = i;
  tac = new TacacsPacket(k,i.get().c_str(),f,session_id_);
  tac_authen = 0;
  tac_author = 0;
  tac_account = 0;
  unlock();
}

TacacsSessionContainer::~TacacsSessionContainer() {
  lock();
  if(tac_account) delete tac_account;
  if(tac_author) delete tac_author;
  if(tac_authen) delete tac_authen;
  delete tac;
  unlock();
  // socket will be closed in Listener code
  pthread_mutex_destroy(&mutex_);
  pthread_mutex_destroy(&mutex2_);
}

void
TacacsSessionContainer::lock() {
  pthread_mutex_lock(&mutex_);
}

void
TacacsSessionContainer::unlock() {
  pthread_mutex_unlock(&mutex_);
}

void
TacacsSessionContainer::lock2() {
  pthread_mutex_lock(&mutex2_);
}

void
TacacsSessionContainer::unlock2() {
  pthread_mutex_unlock(&mutex2_);
}

time_t
TacacsSessionContainer::getCreated() {
  lock();
  time_t ret = created_;
  unlock();
  return ret;
}

//void
//TacacsSessionContainer::setCreated(time_t d) {
//  lock();
//  created_ = d;
//  unlock();
//}

string
TacacsSessionContainer::getLoginstring() {
  string ret;
  lock();
  ret = loginstring_;
  unlock();
  return ret;
}

void
TacacsSessionContainer::setLoginstring(string d) {
  lock();
  loginstring_ = d;
  unlock();
}

string
TacacsSessionContainer::getPwdstring() {
  string ret;
  lock();
  ret = pwdstring_;
  unlock();
  return ret;
}

void
TacacsSessionContainer::setPwdstring(string d) {
  lock();
  pwdstring_ = d;
  unlock();
}

bool
TacacsSessionContainer::getAuthoriz() {
  bool ret;
  lock();
  ret = authoriz_;
  unlock();
  return ret;
}

void
TacacsSessionContainer::setAuthoriz(bool d) {
  lock();
  authoriz_ = d;
  unlock();
}

string
TacacsSessionContainer::getKey() {
  string ret;
  lock();
  ret = key_;
  unlock();
  return ret;
}

void
TacacsSessionContainer::setKey(string d) {
  lock();
  key_ = d;
  unlock();
}

int
TacacsSessionContainer::getSock() {
  int ret;
  lock();
  ret = sock_;
  unlock();
  return ret;
}

void
TacacsSessionContainer::setSock(int d) {
  lock();
  sock_ = d;
  unlock();
}

unsigned int
TacacsSessionContainer::getSessionId() {
  unsigned int ret;
  lock();
  ret = session_id_;
  unlock();
  return ret;
}

void
TacacsSessionContainer::setSessionId(unsigned int d) {
  lock();
  session_id_ = d;
  unlock();
}

ipaddr
TacacsSessionContainer::getPeer() {
  ipaddr ret;
  lock();
  ret = peer_;
  unlock();
  return ret;
}

void
TacacsSessionContainer::setPeer(ipaddr d) {
  lock();
  peer_ = d;
  unlock();
}

// process input packet, if this is it's packet, return true, otherwise - false
// h - packet header, p - packet body
void
TacacsSessionContainer::process(unsigned char *h,unsigned char *p) {
  int type = tac->selector(h); // set packet type: Authen, Author or Account
  int seq = tac->getSeqNo(); // sequence number
  startlog(LOG_TACACS)<<"Session Process type="<<type<<" seq="<<seq<<endlog;

//std::cerr<<"session process type="<<type<<" seq="<<seq<<std::endl;

  ////////// AUTHENTICATION /////////
  if(type == TAC_PLUS_AUTHEN) {
    if(!tac_authen) { // create method if this is first authentication in session
      tac_authen = new TacacsServerAuthentication(tac);
    }
    // now test packet sequences
    switch(seq) {
      // sequence == 1, this is authentication start packet
      case 1:
        // the start packet can contain username,
        // can do not contain username,
        // and should contain username/password if CHAP/PAP

        // clear variables - this is start packet and we do not need in old information
        tac_authen->setRes(0);
        tac_authen->setRequest(0);
        tac_authen->en_req = 0;
        tac_authen->setUser("");
        tac_authen->setPort("");
        tac_authen->setCisco("");
        tac_authen->setRemAddr("");
        bzero(tac_authen->pwstring,sizeof(tac_authen->pwstring));
        bzero(tac_authen->password,sizeof(tac_authen->password));
        bzero(tac_authen->data,sizeof(tac_authen->data));
        tac_authen->setEnable("");

        // process start packet
        tac_authen->setRes(tac_authen->get_start(h,p,tac_authen->data));

//std::cout<<"authentication get_start res="<<tac_authen->getRes()<<" user="<<tac_authen->getUser()<<" cisco="<<tac_authen->getCisco().get()<<" port="<<tac_authen->getPort()<<" rema="<<tac_authen->getRemAddr()<<" data="<<tac_authen->data<<std::endl;

        // check for error
        if(tac_authen->getRes() == 0) { // error in get_start
          startlog(LOG_TACACS)<<"authen_get_start from "<<
            tac_authen->getCisco()<<" returns 0"<<endlog;
          tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_FAIL,"","",
            NOECHO,TAC_PLUS_VER_0);
          return;
        }

        // set type of request
        tac_authen->setRequest(tac_authen->getRes());


        // process ascii logins/passwords
        if(tac_authen->getRes()==TACACS_ASCII_LOGIN || tac_authen->getRes()==TACACS_ENABLE_REQUEST) {
          if(tac_authen->getRes() == TACACS_ASCII_LOGIN) { // if this is login/password
            startlog(LOG_TACACS)<<"Proccess ASCII login/password from "<<tac_authen->getCisco()<<" for "<<tac_authen->getUser()<<endlog;
          } else { // if this is login/enable
            startlog(LOG_TACACS)<<"Proccess ENABLE request from "<<
                tac_authen->getCisco()<<" ("<<tac_authen->getUser()<<")"<<endlog;
            tac_authen->en_req = tac_authen->getRes();
          }
          // a. start without username
          if(tac_authen->getUser().size() == 0) {
//std::cerr<<"Point 0.1"<<std::endl;
            // in last field:  0-echo 1-no echo
            // this is echo during login/pwd input
            startlog(LOG_TACACS)<<"Request loginstring: "<<getLoginstring()<<endlog;
            if(!tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_GETUSER,
                (char*)getLoginstring().c_str(),"",ECHO,TAC_PLUS_VER_0)) {
//std::cerr<<"Point 0.1.1"<<std::endl;
              startlog(LOG_TACACS)<<"error in authen_send_reply to "<<tac_authen->getCisco()<<endlog;
              return;
            }
            return;
          }
//std::cerr<<"Point 0.2"<<std::endl;
          // b. start with username, we should check for nopassword or request password
          // for "nopassword" password
          startlog(LOG_TACACS)<<"try nopassword db_authentication"<<endlog;
          if(tac_authen->db_authentication(tac_authen->getUser(),"(null)",tac_authen->getRemAddr(),
              tac_authen->getCisco(),tac_authen->getPort(),tac_authen->getEnable())) {
//std::cerr<<"Point 0.3"<<std::endl;
            // permit entrance
            startlog(LOG_EVENT)<<"access open for user "<<tac_authen->getUser()<<" (nopassword)"<<endlog;
            tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_PASS,
              "","",NOECHO,TAC_PLUS_VER_0);
            return;
          }
//std::cerr<<"Point 0.4"<<std::endl;
          if(!tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_GETPASS,
              (char*)getPwdstring().c_str(),"",NOECHO,TAC_PLUS_VER_0)) {
//std::cerr<<"Point 0.5"<<std::endl;
            startlog(LOG_TACACS)<<"cant send getpass reply to client"<<endlog;
            return;
          }
//std::cerr<<"Point 0.6"<<std::endl;
          return;
        } else // this is not ASCII request
        // process CHAP/PAP login/passwords
        if((tac_authen->getRequest()==TACACS_PAP_LOGIN || tac_authen->getRequest()==TACACS_CHAP_LOGIN) &&
              (tac_authen->data && strlen(tac_authen->data) > 0) ) {
//std::cerr<<"CHAP/PAP authentication request for "<<tac_authen->getUser()<<std::endl;
          startlog(LOG_TACACS)<<"CHAP/PAP authentication request: "<<
            tac_authen->getUser()<<" "<<tac_authen->data<<endlog;
          bzero(tac_authen->pwstring,sizeof(tac_authen->pwstring));
          strncpy(tac_authen->pwstring, tac_authen->data, sizeof(tac_authen->pwstring)-1);
          tac_authen->chap = true;
        } else {
          // process SENDPASS requests (IOS 11.3(4) requires it sometime)
          // we are don't send password in any case
          if(tac_authen->getRequest()==TACACS_CHAP_PASSWORD || tac_authen->getRequest()==TACACS_PAP_PASSWORD) {
            startlog(LOG_TACACS)<<
              "SENDPASS request from NAS "<<tac_authen->getCisco()<<
                  " ("<<tac_authen->getUser()<<"/"<<tac_authen->getPort()<<")"<<endlog;
            tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_PASS,"","",
              NOECHO,TAC_PLUS_VER_0);
            return;
          } else {
            startlog(LOG_TACACS)<<"Undefined or unsupported authentication method for "<<tac_authen->getCisco()<<endlog;
            tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_FAIL,"","",
              NOECHO,TAC_PLUS_VER_0);
            return;
          }
        }
//std::cerr<<"Try authenticate: user="<<tac_authen->getUser()<<" pwd="<<tac_authen->pwstring<<" remaddr="<<tac_authen->getRemAddr()<<" cisco="<<tac_authen->getCisco().get()<<" port="<<tac_authen->getPort()<<std::endl;

        if(strlen(tac_authen->pwstring) > 0) {
          // check by db functions
          if(!tac_authen->db_authentication(tac_authen->getUser(), tac_authen->pwstring, tac_authen->getRemAddr(), tac_authen->getCisco(), tac_authen->getPort(), tac_authen->getEnable())) {
            startlog(LOG_TACACS)<<"No authentication for user "<<tac_authen->getUser()<<" from "<<tac_authen->getCisco()<<endlog;
            tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_FAIL,"","",
              NOECHO,TAC_PLUS_VER_0);
            return;
          }
          // access permit:
          startlog(LOG_EVENT)<<"access open for user "<<tac_authen->getUser()<<endlog;
          if(tac_authen->chap)
            tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_PASS, "", "", NOECHO,
              TAC_PLUS_VER_1);
          else
            tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_PASS, "", "", NOECHO,
              TAC_PLUS_VER_0);
          startlog(LOG_TACACS)<<"end of authentication user "<<tac_authen->getUser()<<" nas "<<tac_authen->getCisco()<<endlog;
          return;
        }
        return;
        break;          // this is for fun only :)

      // this is second continue packet
      // sequences == 2,3,4
      case 2:
      case 3:
      case 4:
        if(tac_authen->getRequest()==TACACS_ASCII_LOGIN ||
                      tac_authen->getRequest()==TACACS_ENABLE_REQUEST) {
          if(tac_authen->getUser().size() == 0) {
            bzero(tac_authen->data,sizeof(tac_authen->data));
            char user_msg[USERNAME_LEN];
            bzero(user_msg,sizeof(user_msg));
            if((tac_authen->setRes(tac_authen->get_cont(h, p, user_msg, tac_authen->data, USERNAME_LEN))) == -1) {
              startlog(LOG_TACACS)<<"error in authen_get_cont(USERNAME_LEN)"<<endlog;
              tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_FAIL,"","",
                NOECHO,TAC_PLUS_VER_0);
              return;
            }
            tac_authen->setUser(user_msg);
//std::cerr<<"Point X1.5 user="<<tac_authen->getUser()<<" data="<<tac_authen->data<<std::endl;
            startlog(LOG_EVENT)<<"Receive username="<<tac_authen->getUser()<<" data="<<tac_authen->data<<endlog;
//std::cerr<<"Point X1.6"<<std::endl;
            if(tac_authen->getRes() == 1) {      // don't remember what is this means?
//std::cerr<<"Point X1.7"<<std::endl;
              startlog(LOG_INFORM)<<"Receive message: ("<<
                tac_authen->data<<") from "<<tac_authen->getPort()<<"/"<<
                  tac_authen->getRemAddr()<<"/"<<tac_authen->getCisco()<<endlog;
//std::cerr<<"Point X1.8"<<std::endl;
              tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_FAIL,"","",
                NOECHO,TAC_PLUS_VER_0);
//std::cerr<<"Point X1.9"<<std::endl;
              return;
            }
//std::cerr<<"Point X1.10"<<std::endl;
            // now we have user name and must try check it by nopassword
            // for "nopassword" password
            startlog(LOG_TACACS)<<"second nopassword db_authentication"<<endlog;
            if(tac_authen->db_authentication(tac_authen->getUser(), "(null)", tac_authen->getRemAddr(), tac_authen->getCisco(), tac_authen->getPort(), tac_authen->getEnable())) {
//std::cerr<<"Point X1.11"<<std::endl;
              // permit entrance
              startlog(LOG_EVENT)<<"access open for user "<<tac_authen->getUser()<<"%s (nopassword)"<<endlog;
              tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_PASS,
                "","",NOECHO,TAC_PLUS_VER_0);
              return;
            }
//std::cerr<<"Point X1.12"<<std::endl;
            if(tac_authen->en_req == TACACS_ENABLE_REQUEST) {
//std::cerr<<"Point X1.13"<<std::endl;
              if(!tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_GETPASS,
                  "ENABLE PASSWORD:", "", NOECHO, TAC_PLUS_VER_0)) {
//std::cerr<<"Point X1.14"<<std::endl;
                startlog(LOG_TACACS)<<"Cant send enable password request to client"<<endlog;
                return;
              }
//std::cerr<<"Point X1.15"<<std::endl;
            } else {
//std::cerr<<"Point X1.16"<<std::endl;
              if(!tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_GETPASS,
                  (char*)getPwdstring().c_str(),"",NOECHO,TAC_PLUS_VER_0)) {
//std::cerr<<"Point X1.17"<<std::endl;
                startlog(LOG_TACACS)<<"Cant send getpass reply to client"<<endlog;
                return;
              }
//std::cerr<<"Point X1.18"<<std::endl;
            }
//std::cerr<<"Point 4"<<std::endl;
            return;
          } // if username was already present, move to next sequence code with current sequence
//std::cerr<<"Point X1.19"<<std::endl;
        } else {
//std::cerr<<"Point X1.20"<<std::endl;
          return;
        }
        // we must move to next sequence code from old sequence


      // third continue packet (also can be continue of second packet)
      // sequence == 5,6,7
      case 5:
      case 6:
      case 7:
//std::cerr<<"Point X2"<<std::endl;
        if(tac_authen->getRequest()==TACACS_ASCII_LOGIN ||
              tac_authen->getRequest()==TACACS_ENABLE_REQUEST) {
//std::cerr<<"Point X2.1"<<std::endl;
          bzero(tac_authen->password,sizeof(tac_authen->password));
          bzero(tac_authen->data,sizeof(tac_authen->data));
//std::cerr<<"Point X2.2"<<std::endl;
          if((tac_authen->setRes(tac_authen->get_cont(h,p,tac_authen->password,
              tac_authen->data,PASSWORD_LEN))) == -1) {
//std::cerr<<"Point X2.3"<<std::endl;
            startlog(LOG_TACACS)<<"error in authen_get_cont(PASSWORD)"<<endlog;
            tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_FAIL,"","",
              NOECHO,TAC_PLUS_VER_0);
//std::cerr<<"error in authen_get_cont(PASSWORD)"<<std::endl;
            return;
          }
//std::cerr<<"Point X2.4 password="<<tac_authen->password<<" data="<<tac_authen->data<<std::endl;
          startlog(LOG_EVENT)<<"ASCII authen request: "<<tac_authen->getUser()<<" "<<tac_authen->data<<endlog;
//std::cerr<<"ASCII authen request user="<<tac_authen->user<<" data="<<tac_authen->data<<std::endl;
          if(tac_authen->getRes() == 1) {
//std::cerr<<"Point X2.5"<<std::endl;
            startlog(LOG_TACACS)<<"Authen continue aborted"<<endlog; // ???
//std::cerr<<"Authen continue aborted"<<std::endl;
            return;
          }
//std::cerr<<"Point X2.6"<<std::endl;
          if(tac_authen->en_req == TACACS_ENABLE_REQUEST) {
//std::cerr<<"Point X2.7"<<std::endl;
            tac_authen->setEnable(tac_authen->password);
          } else {
//std::cerr<<"Point X2.8 pwd="<<tac_authen->password<<std::endl;
            bzero(tac_authen->pwstring,sizeof(tac_authen->pwstring));
            snprintf(tac_authen->pwstring, sizeof(tac_authen->pwstring)-1, "%s", tac_authen->password);
          }
//std::cerr<<"Point X2.9"<<std::endl;
        } else {
//std::cerr<<"Point X2.10"<<std::endl;
          startlog(LOG_TACACS)<<"Wrong authentication packet with seq="<<seq<<endlog;
//std::cerr<<"Wrong authentication packet with seq="<<seq<<std::endl;
          return;
        }
//std::cerr<<"Point X2.11 user="<<tac_authen->user<<" pwd="<<tac_authen->pwstring<<std::endl;
        // check by db functions
        if(!tac_authen->db_authentication(tac_authen->getUser(), tac_authen->pwstring, tac_authen->getRemAddr(), tac_authen->getCisco(), tac_authen->getPort(), tac_authen->getEnable())) {
//std::cerr<<"Point X2.12"<<std::endl;
          startlog(LOG_TACACS)<<"Cant authenticate user "<<tac_authen->getUser()<<endlog;
//std::cerr<<"Cant authenticate user "<<tac_authen->user<<std::endl;
          tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_FAIL, "", "",
            NOECHO, TAC_PLUS_VER_0);
          return;
        }
//std::cerr<<"Point X2.13"<<std::endl;
        // access permit:
        startlog(LOG_EVENT)<<"access open for user "<<tac_authen->getUser()<<endlog;
//std::cerr<<"access open for user "<<tac_authen->user<<std::endl;
        if(tac_authen->chap) {
//std::cerr<<"Point X2.14"<<std::endl;
          tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_PASS, "", "", NOECHO,
            TAC_PLUS_VER_1);
        } else {
//std::cerr<<"Point X2.15"<<std::endl;
          tac_authen->send_reply(TAC_PLUS_AUTHEN_STATUS_PASS, "", "", NOECHO,
            TAC_PLUS_VER_0);
        }
        startlog(LOG_EVENT)<<"end of authentication"<<endlog;
//std::cerr<<"end of authentication"<<std::endl;
        return;
        break;

      default:
        startlog(LOG_TACACS)<<"invalid sequence during authentication "<<seq<<endlog;
        return;
        break;
    }

  } // end of authentication


  ///////// AUTHORIZATION //////////

  if(type == TAC_PLUS_AUTHOR) {
    if(!tac_author) {
      tac_author = new TacacsServerAuthorization(tac,authoriz_);
    }
   try {
      if(!tac_author->get_request(h,p,&tac_author->method,&tac_author->priv_lvl,
        &tac_author->authen_type,&tac_author->authen_service,
          tac_author->user,tac_author->port,tac_author->rem_addr,
            tac_author->avpair)) {
        startlog(LOG_TACACS)<<"error in authorization request packet ("<<
            tac_author->user<<","<<tac_author->port<<
              ","<<tac_author->rem_addr<<") from "<<getPeer()<<endlog;
        throw 0;
      }
//std::cerr<<"authoriz start info: user="<<tac_author->user<<" port="<<tac_author->port<<" remaddr="<<tac_author->rem_addr<<std::endl;
      startlog(LOG_TACACS)<<"AUTHOR: "<<tac_author->user<<" \'"<<
          tac_author->port<<"\' \'"<<tac_author->rem_addr<<
            "\' process for "<<getPeer()<<endlog;

      for(int i=0; tac_author->avpair[i] && i<MAX_AVP; i++) {
        startlog(LOG_TACACS)<<"AUTHOR: received av-pair from nas "<<
          getPeer()<<":"<<tac_author->avpair[i]<<endlog;
//std::cerr<<"received av-pair from "<<peer.get()<<": "<<tac_author->avpair[i]<<std::endl;
      }

      if(tac_author->user && strlen(tac_author->user)>0) {
        startlog(LOG_TACACS)<<"AUTHOR: user > 0, read database"<<endlog;
//std::cerr<<"Point 1"<<std::endl;
        tac_author->dbd = tac_author->db_authorization(tac_author->user,
          (char*)getPeer().get().c_str(),tac_author->port,tac_author->rem_addr);
//std::cerr<<"Point 2"<<std::endl;
        startlog(LOG_TACACS)<<"AUTHOR: AVP database readed"<<endlog;
        if(!tac_author->dbd.getResult()) {
//std::cerr<<"Point 3"<<std::endl;
          startlog(LOG_TACACS)<<"AUTHOR: no data received from database"<<endlog;
          throw 1;
        }
//std::cerr<<"Point 4"<<std::endl;
      } else {
        startlog(LOG_TACACS)<<"AUTHOR: user == 0, get user DEFAULT data"<<endlog;
        tac_author->dbd = tac_author->db_authorization("DEFAULT",
          (char*)getPeer().get().c_str(), tac_author->port, tac_author->rem_addr);
        if(!tac_author->dbd.getResult()) {
          startlog(LOG_TACACS)<<"AUTHOR: user == 0, init void data"<<endlog;
          throw 1;
        }
      }
      user_avp avp;
      for(int i=0; i < tac_author->dbd.avp_size(); i++) {
        avp = tac_author->dbd.avp_get(i);
        startlog(LOG_AAA)<<"received from db: service="<<avp.getService()<<
            ", protocol="<<avp.getProtocol()<<", avp="<<avp.getAvp()<<endlog;
      }

//std::cerr<<"Point 7"<<std::endl;
//std::cerr<<"Point 7="<<tac_author->avpair[0]<<std::endl;
      if(tac_author->avpair[0]) {
//std::cerr<<"Point 7.1"<<std::endl;
        // service=xxx av-pair must be always present !
        if(strlen(tac_author->avpair[0])<8 ||
            !utl::scomp(tac_author->avpair[0],"service=",8)) {   // wrong avpair
//std::cerr<<"Point 7.2"<<std::endl;
          startlog(LOG_INFORM)<<"Wrong avpair ["<<tac_author->avpair[0]<<
              "] from "<<tac_author->user<<"/"<<tac_author->port<<
                "/"<<tac_author->rem_addr<<"/"<<getPeer()<<endlog;
//std::cerr<<"Point 7.3"<<std::endl;
          throw 1;
        }
//std::cerr<<"Point 8"<<std::endl;
        tac_author->j = 0;// clear counter
        if(tac_author->curService) delete [] tac_author->curService;
        tac_author->curService = utl::newstr(tac_author->avpair[0]+8); // store service
        tac_author->start_av = 1;
        tac_author->bavpair[tac_author->j++] = utl::newstr(tac_author->avpair[0]); // copy service=
//std::cerr<<"Point 9"<<std::endl;
        if(utl::scomp(tac_author->avpair[1],"protocol=",9)) {// if present protocol=xxx avpair
          if(tac_author->curProtocol) delete [] tac_author->curProtocol;
          tac_author->curProtocol = utl::newstr(tac_author->avpair[1]+9);
          tac_author->start_av = 2;
          tac_author->bavpair[tac_author->j++] = utl::newstr(tac_author->avpair[1]);	// copy protocol=
        }
        tac_author->bavpair[tac_author->j] = 0;// set null in end
      }
//std::cerr<<"Point 10"<<std::endl;

      // x) - if no service/protocol received from NAS, try to add our own values
      // (this is mostly for Cisco VoIP gateways)
      if(!tac_author->curService && !tac_author->curProtocol) {
        tac_author->j = 0;
//std::cerr<<"Point 10.1"<<std::endl;
        for(int k=0; k < tac_author->dbd.avp_size() && tac_author->j < 100; k++) {
          avp = tac_author->dbd.avp_get(k);
//std::cerr<<"Point 10.2"<<std::endl;
          if(avp.getAvp().size() > 0) {
//std::cerr<<"Point 10.3"<<std::endl;
            tac_author->avps[tac_author->j++] = utl::newstr((char*)avp.getAvp().c_str());
//std::cerr<<"Point 10.4"<<std::endl;
            tac_author->avps[tac_author->j] = 0;
//std::cerr<<"Point 10.5"<<std::endl;
            tac_author->added++;// we should set PASS_ADD if before we not set PASS_REPL
//std::cerr<<"Point 10.6"<<std::endl;
            startlog(LOG_TACACS)<<"AUTHOR: avp "<<avp.getAvp()<<
              " add to x) (added="<<tac_author->added<<")"<<endlog;
//std::cerr<<"Point 10.7"<<std::endl;
          }
//std::cerr<<"Point 10.8"<<std::endl;
        }
std::cerr<<"Point 1"<<std::endl;
        tac_author->send_response(TAC_PLUS_AUTHOR_STATUS_PASS_ADD,"","",
          tac_author->avps);
std::cerr<<"Point 1.1"<<std::endl;
        throw 0;
      }
//std::cerr<<"Point 11"<<std::endl;
      if(!tac_author->curService) tac_author->curService = utl::newstr("");
      if(!tac_author->curProtocol) tac_author->curProtocol = utl::newstr("");
//std::cerr<<"Point 12"<<std::endl;
      // search for unknown services/protocols
      bool flag_serv = false;
      bool flag_proto = false;

      for(int i=0; i < tac_author->dbd.avp_size(); i++) {
        avp = tac_author->dbd.avp_get(i);

        startlog(LOG_AAA)<<"compare service="<<tac_author->curService<<
          " with db.service="<<avp.getService()<<endlog;
        if(avp.compareService(tac_author->curService)) {
          flag_serv = true;
          if(avp.compareProtocol(tac_author->curProtocol)) flag_proto = true;
        }
        if(flag_serv && flag_proto) break;
      }
//std::cerr<<"Point 13"<<std::endl;
      if(!flag_serv && !flag_proto) { // if wrong service/protocol
        startlog(LOG_AAA)<<"Wrong service/protocol "<<
          tac_author->curService<<"/"<<tac_author->curProtocol<<endlog;
//std::cerr<<"Wrong service/protocol "<<tac_author->curService<<"/"<<tac_author->curProtocol<<std::endl;
        throw 1;
      }
//std::cerr<<"Point 14"<<std::endl;
      cmd_avp cmd;
      // review throught received from NAS avpairs list
      for(int i = tac_author->start_av; tac_author->avpair[i]; i++) {
        if(!tac_author->avpair[i] || strlen(tac_author->avpair[i])==0) continue;
        tac_author->nas_avp = tac_author->avpair[i];
        startlog(LOG_TACACS)<<
          "AUTHOR: Review received from NAS "<<getPeer()<<
              " avpair: "<<tac_author->nas_avp<<endlog;
        if(tac_author->match_attr(tac_author->nas_avp,"cmd=")) {
          tac_author->flag1 = false;
          tac_author->flag2 = true;

          for(int k=0; k < tac_author->dbd.cmd_size(); k++) {
            cmd = tac_author->dbd.cmd_get(k);

            if(cmd.getCmdperm().size() > 0) {
              startlog(LOG_TACACS)<<"AUTHOR: compare "<<
                tac_author->get_value(tac_author->nas_avp)<<
                  " with cmdperm="<<cmd.getCmdperm()<<endlog;
              // check for cmdpermit
              if(regcomp(&tac_author->preg,cmd.getCmdperm().c_str(),REG_EXTENDED) == 0) {
                if(regexec(&tac_author->preg, tac_author->get_value(tac_author->nas_avp), tac_author->nmatch, tac_author->pmatch, 0) == 0)
                  tac_author->flag1 = true;
                regfree(&tac_author->preg);
              }
            }
            if(cmd.getCmddeny().size() > 0) {
              startlog(LOG_TACACS)<<"AUTHOR: compare "<<
                tac_author->get_value(tac_author->nas_avp)<<
                  " with cmddeny="<<cmd.getCmddeny()<<endlog;
              // check by cmddeny
              if(regcomp(&tac_author->preg, cmd.getCmddeny().c_str(),REG_EXTENDED) == 0) {
                if(regexec(&tac_author->preg, tac_author->get_value(tac_author->nas_avp), tac_author->nmatch, tac_author->pmatch, 0) == 0)
                  tac_author->flag2 = false;
                regfree(&tac_author->preg);
              }
            }
          }
          if(!tac_author->flag2) throw 1; // if any match in cmddeny
          if(!tac_author->flag1) throw 1; // if no matches in cmdperm
          continue;
        }
        if(tac_author->match_attr(tac_author->nas_avp,"cmd-arg=")) {
          if(utl::scomp("cmd-arg=<cr>",tac_author->nas_avp)) continue;
          tac_author->flag3 = false;
          tac_author->flag4 = true;

          for(int k=0; k < tac_author->dbd.cmd_size(); k++) {
            cmd = tac_author->dbd.cmd_get(k);

            if(cmd.getArgperm().size() > 0) {
              startlog(LOG_TACACS)<<"AUTHOR: compare "<<
                tac_author->get_value(tac_author->nas_avp)<<
                  " with argperm="<<cmd.getArgperm()<<endlog;
              // check by argperm
              if(regcomp(&tac_author->preg,cmd.getArgperm().c_str(),
                    REG_EXTENDED) == 0) {
                if(regexec(&tac_author->preg, tac_author->get_value(tac_author->nas_avp),
                    tac_author->nmatch, tac_author->pmatch, 0) == 0)
                  tac_author->flag3 = true;
                regfree(&tac_author->preg);
              }
            }
            if(cmd.getArgdeny().size() > 0) {
              startlog(LOG_TACACS)<<"AUTHOR: compare "<<
                tac_author->get_value(tac_author->nas_avp)<<
                  " with argdeny="<<cmd.getArgdeny()<<endlog;
              // check by argdeny
              if(regcomp(&tac_author->preg,cmd.getArgdeny().c_str(),
                    REG_EXTENDED) == 0) {
                if(regexec(&tac_author->preg, tac_author->get_value(tac_author->nas_avp),
                    tac_author->nmatch, tac_author->pmatch, 0) == 0)
                  tac_author->flag4 = false;
                regfree(&tac_author->preg);
              }
            }
          }
          if(!tac_author->flag4) throw 1; // if any match in argdeny
          if(!tac_author->flag3) throw 1; // if no match in argperm
          continue;
        }
        if(tac_author->mandatory(tac_author->nas_avp)) {// this argument from nas is mandatory
          // a/b) mandatory/optional database avpairs
          //   if we have received avp like addr=10.1.1.1
          //   and it match with database avp addr=10.1.1.1
          //   or in database we see optional avp like addr*10.1.1.1
          //   then copy this avp to reply

         for(int k=0; k < tac_author->dbd.avp_size(); k++) {
           avp = tac_author->dbd.avp_get(k);

            // if db avp equal nas avp
            if(utl::scomp(avp.getAvp().c_str(),tac_author->nas_avp)) {
              tac_author->bavpair[tac_author->j++] = utl::newstr(tac_author->nas_avp);
              tac_author->bavpair[tac_author->j] = 0; // set null in end
              startlog(LOG_TACACS)<<"AUTHOR: avp "<<tac_author->nas_avp<<" add to a)"<<endlog;
              goto next_nas_arg;
            }
            // if attrs equal or optional argument
            if(tac_author->match_attr((char*)avp.getAvp().c_str(),tac_author->nas_avp) &&
                tac_author->optional((char*)avp.getAvp().c_str())) {
              tac_author->bavpair[tac_author->j++] = utl::newstr(tac_author->nas_avp);
              tac_author->bavpair[tac_author->j] = 0; // set null at the end
              startlog(LOG_TACACS)<<"AUTHOR: avp "<<tac_author->nas_avp<<" add to b)"<<endlog;
              goto next_nas_arg;
            }
          }

          // c)
          //  if for this NAS default authorization DENY,
          //  than we should return FAIL
          if(!authoriz_) {
            startlog(LOG_TACACS)<<"AUTHOR: FAIL because default author=deny c)"<<endlog;
            throw 1;
          }
          // d) if for this NAS default authorization PERMIT,
          if(authoriz_) {
            tac_author->bavpair[tac_author->j++] = utl::newstr(tac_author->nas_avp);// copy
            tac_author->bavpair[tac_author->j] = 0;// stoper
            startlog(LOG_TACACS)<<"AUTHOR: avp "<<tac_author->nas_avp<<" add to d)"<<endlog;
            goto next_nas_arg;
          }
        } else {// NAS avpair optional
          // e) av from nas optional like addr*10.2.2.2
          //    and in db av optional like addr*10.2.2.2 too
          for(int k=0; k < tac_author->dbd.avp_size(); k++) {
            avp = tac_author->dbd.avp_get(k);

            if(utl::scomp(tac_author->nas_avp,avp.getAvp().c_str())) {
              tac_author->bavpair[tac_author->j++] = utl::newstr(tac_author->nas_avp);
              tac_author->bavpair[tac_author->j] = 0;
              startlog(LOG_TACACS)<<"AUTHOR: avp "<<tac_author->nas_avp<<" add to e)"<<endlog;
              goto next_nas_arg;
            }
          }
          // f) nas av optional like addr*10.3.3.3
          //    in db mandatory av like addr=5.1.1.1
          //    then change to avp from db
          for(int k=0; k < tac_author->dbd.avp_size(); k++) {
            avp = tac_author->dbd.avp_get(k);

            if(tac_author->mandatory((char*)avp.getAvp().c_str()) &&
                tac_author->match_attr(tac_author->nas_avp,(char*)avp.getAvp().c_str())) {
              tac_author->bavpair[tac_author->j++] = utl::newstr((char*)avp.getAvp().c_str());
              tac_author->bavpair[tac_author->j] = 0;
              tac_author->replaced++; //  we should send this with PASS_REPL
              startlog(LOG_TACACS)<<"AUTHOR: avp "<<tac_author->nas_avp<<" add to f)"<<endlog;
              goto next_nas_arg;
            }
          }
          // g) if nas avp optional like addr*10.1.1.1
          //    in db av optional addr*10.1.1.1
          //    set addr*10.1.1.1 in reply
          for(int k=0; k < tac_author->dbd.avp_size(); k++) {
            avp = tac_author->dbd.avp_get(k);

            if(tac_author->mandatory((char*)avp.getAvp().c_str())) continue;
            if(tac_author->match_attr(tac_author->nas_avp, (char*)avp.getAvp().c_str()) &&
                tac_author->match_value(tac_author->nas_avp, (char*)avp.getAvp().c_str())) {
              tac_author->bavpair[tac_author->j++] = utl::newstr((char*)avp.getAvp().c_str());
              tac_author->bavpair[tac_author->j] = 0;
              tac_author->replaced++;   //  we should send this with PASS_REPL
              startlog(LOG_TACACS)<<"AUTHOR: avp "<<tac_author->nas_avp<<" add to g)"<<endlog;
              goto next_nas_arg;
            }
          }
          // h)	if nas avp optional like addr*10.1.1.1
          //    and in db avp optional addr*2.1.1.1
          //    then set addr*10.1.1.1 in reply
          for(int k=0; k < tac_author->dbd.avp_size(); k++) {
            avp = tac_author->dbd.avp_get(k);

            if(tac_author->mandatory((char*)avp.getAvp().c_str())) continue;
            if(tac_author->match_attr(tac_author->nas_avp, (char*)avp.getAvp().c_str())) {
              tac_author->bavpair[tac_author->j++] = utl::newstr((char*)avp.getAvp().c_str());
              tac_author->bavpair[tac_author->j] = 0;
              tac_author->replaced++;   //  we should send this with PASS_REPL
              startlog(LOG_TACACS)<<"AUTHOR: avp "<<tac_author->nas_avp<<" add to h)"<<endlog;
              goto next_nas_arg;
            }
          }
          // i)
          // if default authorization for NAS is deny
          // no entries to bavpair
          // and send with PASS_REPL
          if(!authoriz_) {
            tac_author->replaced++;
            startlog(LOG_TACACS)<<"AUTHOR: avp NULL according i)"<<endlog;
            goto next_nas_arg;
          }
          // j)
          // if default authorization for NAS is permit
          if(authoriz_) {
            tac_author->bavpair[tac_author->j++] = utl::newstr(tac_author->nas_avp);
            tac_author->bavpair[tac_author->j] = 0;
            tac_author->replaced++;  //  we should send this with PASS_REPL
            startlog(LOG_TACACS)<<"AUTHOR: avp "<<tac_author->nas_avp<<" added to j)"<<endlog;
            goto next_nas_arg;	
          }
        }
        next_nas_arg:;
      }
      // end of review avpairs list from NAS

      // k) after all avps had proceed, for
      //    mandatory avp from db, if don't match with attribute,
      //    which in reply list already, add that avp
      for(int k=0; k < tac_author->dbd.avp_size(); k++) {
        avp = tac_author->dbd.avp_get(k);

        if(!avp.compareService(tac_author->curService) ||
            !avp.compareProtocol(tac_author->curProtocol)) continue;
        if(tac_author->optional((char*)avp.getAvp().c_str())) continue;
        for(int i=0; tac_author->bavpair[i] != 0; i++) {
          if(tac_author->match_attr((char*)avp.getAvp().c_str(), tac_author->bavpair[i]))
          goto next_cfg_arg;
        }
        if(avp.getAvp().size() > 0) {
          tac_author->bavpair[tac_author->j++] = utl::newstr((char*)avp.getAvp().c_str());
          tac_author->bavpair[tac_author->j] = 0;
          tac_author->added++;// we should set PASS_ADD if before we not set PASS_REPL
          startlog(LOG_TACACS)<<"AUTHOR: avp "<<avp.getAvp()<<" add to k) (added="<<tac_author->added<<")"<<endlog;
        }
        next_cfg_arg:;
      }
      ///////////////////////////
      // if PASS_REPL, then send bavpair with PASS_REPL and this is all
      ///////////////////////////
      if(tac_author->replaced > 0) {
        startlog(LOG_TACACS)<<"AUTHOR: We are sending PASS_REPL"<<endlog;
        for(int k=0; tac_author->bavpair[k] && k<MAX_AVP; k++)
          startlog(LOG_TACACS)<<"AUTHOR:   REPL avp="<<tac_author->bavpair[k]<<endlog;
std::cerr<<"Point 2"<<std::endl;
        tac_author->send_response(TAC_PLUS_AUTHOR_STATUS_PASS_REPL,
          "","",tac_author->bavpair);
std::cerr<<"Point 2.1"<<std::endl;
        throw 0;
      }
      ///////////////////////////////////////////////////////
      // if PASS_ADD (we should return only the additions)
      ///////////////////////////////////////////////////////
      if(tac_author->added > 0) {
        startlog(LOG_TACACS)<<"AUTHOR: We are sending PASS_ADD *"<<endlog;
        // delete all, which we have received from NAS, send only new from db
        tac_author->avps[0] = 0;
        int num_in_args, num_out_args;
        for(num_in_args=0; tac_author->avpair[num_in_args]!=0 && num_in_args<MAX_AVP;
          num_in_args++);
        for(num_out_args=0; tac_author->bavpair[num_out_args]!=0 &&
          num_out_args < MAX_AVP; num_out_args++);
        startlog(LOG_TACACS)<<"AUTHOR: compare num of args: "<<num_in_args<<" vs "<<num_out_args<<endlog;
        int p=0;
        for(int k=num_in_args; k<num_out_args && k<MAX_AVP; k++) {
          if(strlen(tac_author->bavpair[k]) > 0) {// drop avpairs with len==0
            tac_author->avps[p++] = utl::newstr(tac_author->bavpair[k]);
            tac_author->avps[p] = 0;// set null at the end
          }
        }
        for(int k=0; tac_author->avps[k] && k<MAX_AVP; k++)
          startlog(LOG_TACACS)<<"AUTHOR:   ADD avp="<<tac_author->avps[k]<<endlog;
std::cerr<<"Point 3"<<std::endl;
        tac_author->send_response(TAC_PLUS_AUTHOR_STATUS_PASS_ADD, "", "",
          tac_author->avps);
std::cerr<<"Point 3.1"<<std::endl;
        throw 0;
      }
      // we should reply, because no additions or replacements
      //char ret[1][1];
      //*ret[0] = 0;
      char **ret;
      ret = 0;
std::cerr<<"Point 4"<<std::endl;
      tac_author->send_response(TAC_PLUS_AUTHOR_STATUS_PASS_ADD,"","",(char**)ret);
std::cerr<<"Point 4.1"<<std::endl;
      throw 0;
//    }// end of get request

//    if(seq > 2) {// no such authorization sequence
//      startlog(LOG_TACACS)<<"wrong sequence number during authorization = "<<seq<<endlog;
//      throw 1;// no this sequence number during authorization process
//    }

  } // end of try

  catch(int r) {
//std::cerr<<"Point XX1"<<std::endl;
    if(tac_author->curService) delete [] tac_author->curService;
//std::cerr<<"Point XX2"<<std::endl;
    if(tac_author->curProtocol) delete [] tac_author->curProtocol;
//std::cerr<<"Point XX3"<<std::endl;
    for(int iii=0; tac_author->bavpair[iii] && iii<MAX_AVP; iii++) {
      delete [] tac_author->bavpair[iii];
      //tac_author->bavpair[iii] = 0;
    }
//std::cerr<<"Point XX4"<<std::endl;
    for(int iii=0; tac_author->avpair[iii] && iii<MAX_AVP; iii++) {
      utl::delstr(tac_author->avpair[iii]);
      //tac_author->avpair[iii] = 0;
    }
//std::cerr<<"Point XX5"<<std::endl;
    for(int iii=0; tac_author->avps[iii] && iii<MAX_AVP; iii++) {
      utl::delstr(tac_author->avps[iii]);
      //tac_author->avps[iii] = 0;
    }
//std::cerr<<"Point XX6"<<std::endl;
    char **fpair;
    if(r != 0) {
      fpair = 0;
std::cerr<<"Point 5"<<std::endl;
      tac_author->send_response(TAC_PLUS_AUTHOR_STATUS_FAIL,"","",(char**)fpair);
std::cerr<<"Point 5.1"<<std::endl;

    }
//std::cerr<<"authorization end"<<std::endl;
//std::cerr<<"Point XX7"<<std::endl;
    return;
  }

  } // end of authorization



  ////////// ACCOUNTING ////////////

  if(type == TAC_PLUS_ACCT) {
    if(!tac_account) {
      tac_account = new TacacsServerAccounting(tac);
    }
    if(seq == 1) { // accounting start packet
//std::cerr<<"Point Y1"<<std::endl;
      //char *avpair[255]; // for AVP
      int method=0, priv_lvl=0, authen_type=0, authen_service=0;
      char username[USERNAME_LEN];
      char port[PORT_LEN];
      char rem_addr[REM_ADDR_LEN];
      char nas[IP_LEN];
      char typestr[20];
      list<string> logavp;
      char str[128];
      bzero(username,sizeof(username));
      bzero(port,sizeof(port));
      bzero(rem_addr,sizeof(rem_addr));
      bzero(nas,sizeof(nas));
      bzero(str,sizeof(str));

      //for(list<string>::iterator i=avpair.begin();i!=avpair.end() && avpair.size()>0;) {
      //  i = avpair.erase(i);
      //}

//std::cerr<<"Point Y2"<<std::endl;
      int flag = tac_account->get_request(h,p,&method,&priv_lvl,
          &authen_type,&authen_service,username,port,rem_addr,nas);
      //,avpair);
//std::cerr<<"Point Y3"<<std::endl;
      if(flag == 0) {
//std::cerr<<"Point Y4"<<std::endl;
        startlog(LOG_INFORM)<<"Error in ACCOUNT_GET_REQUEST packet from "<<nas<<endlog;
        // error in packet
      } else {
//std::cerr<<"Point Y5"<<std::endl;
        if(flag == TAC_PLUS_ACCT_FLAG_START)
          snprintf(typestr,sizeof(typestr)-1,"START");
        else if(flag == TAC_PLUS_ACCT_FLAG_STOP)
          snprintf(typestr,sizeof(typestr)-1,"STOP");
        else snprintf(typestr,sizeof(typestr)-1,"UPDATE");

        string stmp;

//        while(avpair[i] && strlen(avpair[i])>0)
//          startlog(LOG_TACACS)<<"  Receive AVpair: "<<avpair[i++]<<endlog;
        startlog(LOG_TACACS)<<"send reply TAC_PLUS_ACCT_STATUS_SUCCESS"<<endlog;
        tac_account->send_reply("","",TAC_PLUS_ACCT_STATUS_SUCCESS);
        // create array for database output
        snprintf(str,sizeof(str),"action=%s",typestr);
        logavp.push_back(str);
        snprintf(str,sizeof(str),"user=%s",username);
        logavp.push_back(str);
        snprintf(str,sizeof(str),"port=%s",port);
        logavp.push_back(str);
        snprintf(str,sizeof(str),"from=%s",rem_addr);
        logavp.push_back(str);
        snprintf(str,sizeof(str),"nas=%s",nas);
        logavp.push_back(str);

        for(list<string>::iterator i=tac_account->avpair.begin();i!=tac_account->avpair.end() && !(tac_account->avpair.empty());) {
          stmp = *i;
          i = tac_account->avpair.erase(i);
          if(stmp.size() > 0) {
            startlog(LOG_TACACS)<<"  Receive AVpair: "<<stmp<<endlog;
            if(!utl::scomp((char*)stmp.c_str(),"faxrelay-start-time=",20)) {
              logavp.push_back(stmp);
            }
          }
        }
//std::cerr<<"Point Y6"<<std::endl;
        tac_account->db_accounting(logavp); // call for database function
//std::cerr<<"Point Y7"<<std::endl;
        for(list<string>::iterator i=logavp.begin();i!=logavp.end() && !logavp.empty();) {
          stmp = *i;
          i = logavp.erase(i);
//          stmp.erase();
        }
//        logavp.clear();
//std::cerr<<"Point Y8"<<std::endl;
      }
//std::cerr<<"Point Y9"<<std::endl;
    }
//std::cerr<<"Point Y10"<<std::endl;
  }
//std::cerr<<"Point Y11"<<std::endl;
}

///// TacacsPacketReader

// constr
TacacsPacketReader::TacacsPacketReader(char *peer,int f) : Packet(f) {
  pthread_mutex_init(&mutex_,0);
  char l[100]; // login request string
  char p[100]; // password request string
  char key[100]; // key
  bool da = false; // default authorization
  // find this NAS, select MD5-key
  Device_data *dtmp;
  dtmp = 0;
  ipaddr ip(peer);
  dtmp = coreData->getDevice()->get(ip);
  if(!dtmp) {
    startlog(LOG_INFORM)<<"Connect attempt from non-authorized device "<<ip<<" to TACACS+"<<endlog;
    return;
  }
  snprintf(l,sizeof(l),"%s",dtmp->getLoginstring().c_str());
  snprintf(p,sizeof(p),"%s",dtmp->getPwdstring().c_str());
  snprintf(key,sizeof(key),"%s",dtmp->getTacKey().c_str());
  da = dtmp->getDefAuthor();


  unsigned char *buf;
  buf = 0;
  unsigned char *buf2;
  buf2 = 0;
  TacacsSessionContainer *tmp;
  tmp = 0;
  HDR *hdr;
  hdr = 0;
  unsigned int sess;
  sess = 0;
  int paklen;
  paklen = 0;
  int len;
  len = 0;
  bool res;
  res = false;

  while(true) {
    if(buf) delete [] buf;
    buf = read_packet_header();
    if(buf == 0) { // problems with connect
      lock();
      for(list<TacacsSessionContainer*>::iterator i=tacacsSessionQueue_.begin();
          i!=tacacsSessionQueue_.end() && !tacacsSessionQueue_.empty();) {
        tmp = *i;
        i = tacacsSessionQueue_.erase(i);
        startlog(LOG_TACACS)<<"tacacs+ session "<<tmp->getSessionId()<<
            " disconnect from "<<peer<<" sock="<<f<<endlog;
        delete tmp; // delete session
        tmp = 0;
      }
      unlock();
      break;
    }
    hdr = (HDR*)buf;
    sess = ntohl(hdr->session_id); // get session id
    //std::cerr<<"Session_id="<<sess<<std::endl;
    paklen = TAC_PLUS_HDR_SIZE + ntohl(hdr->datalength);
    len = ntohl(hdr->datalength);

    // search if session with this id and peer had already been created
    res = false;
    lock();
    for(list<TacacsSessionContainer*>::const_iterator i=tacacsSessionQueue_.begin();
          i!=tacacsSessionQueue_.end(); i++) {
      tmp = *i;
      if(tmp->getPeer()==peer && tmp->getSock()==f && tmp->getSessionId()==sess) {
        res = true;
        break;
      }
    }
    unlock();
    if(!res) { // no such session created
      // create session and create session thread
      tmp = new TacacsSessionContainer(ip, f, sess, key, l, p, da);
//      lock();
      tacacsSessionQueue_.push_back(tmp);
//      unlock();
      startlog(LOG_TACACS)<<"create new tacacs+ session with "<<peer<<" sessid="<<sess<<endlog;
    }
    if(buf2) delete [] buf2;
    buf2 = tmp->tac->read_packet(buf);

    // buf - packet header
    // buf2 - packet body

    if(buf2 == 0) { // problems with connect
      lock();
      for(list<TacacsSessionContainer*>::iterator i=tacacsSessionQueue_.begin();
          i!=tacacsSessionQueue_.end() && !tacacsSessionQueue_.empty();) {
        tmp = *i;
        i = tacacsSessionQueue_.erase(i);
        startlog(LOG_TACACS)<<"tacacs+ session "<<tmp->getSessionId()<<
            " disconnect from "<<peer<<" due connect error"<<endlog;
        delete tmp;
      }
      //tacacsSessionQueue_.clear();
      unlock();
      break;
    }
    tmp->lock2();
    tmp->process(buf, buf2);
    tmp->unlock2();

    if(buf) delete [] buf;
    buf = 0;
    if(buf2) delete [] buf2;
    buf2 = 0;
  }
  if(buf) delete [] buf;
  if(buf2) delete [] buf2;
}

// destr
TacacsPacketReader::~TacacsPacketReader() {
  lock();
  TacacsSessionContainer *tmp = 0;
  for(list<TacacsSessionContainer*>::iterator i=tacacsSessionQueue_.begin();
       i!=tacacsSessionQueue_.end() && !tacacsSessionQueue_.empty();) {
    tmp = *i;
    i = tacacsSessionQueue_.erase(i);
    startlog(LOG_TACACS)<<"tacacs+ session "<<tmp->getSessionId()<<
       " disconnect2"<<endlog;
    delete tmp; // delete session
  }
  unlock();
  pthread_mutex_destroy(&mutex_);
}

void
TacacsPacketReader::lock() {
  pthread_mutex_lock(&mutex_);
}

void
TacacsPacketReader::unlock() {
  pthread_mutex_unlock(&mutex_);
}

// read only packet header
unsigned char*
TacacsPacketReader::read_packet_header() {
  HDR hdr;
  int len;
  bzero((char*)&hdr, sizeof(hdr)); // clear hdr place
  // read a packet header
  len = sockread((unsigned char *)&hdr,
    TAC_PLUS_HDR_SIZE, TAC_PLUS_READ_TIMEOUT);
  if(len != TAC_PLUS_HDR_SIZE) return 0;
  if((hdr.version & TAC_PLUS_MAJOR_VER_MASK) != TAC_PLUS_MAJOR_VER) {
    startlog(LOG_ERROR)<<
      "Illegal major version specified: found "<<hdr.version<<
          " wanted "<<TAC_PLUS_MAJOR_VER<<endlog;
    return 0;
  }
  unsigned char *ret;
  ret = new unsigned char[TAC_PLUS_HDR_SIZE + 1];
  bcopy(&hdr, ret, TAC_PLUS_HDR_SIZE);
  return ret;
}

///// TacacsServer

/////////////////////////////////////////////////////
//
//  sockets and MD5 encrypting (packet handling)
//
/////////////////////////////////////////////////////

TacacsPacket::TacacsPacket(const char *k,const char *p,int f,unsigned int d) : Packet(f) {
  pthread_mutex_init(&mutex_,0);
  lock();
  setFid(f); // store file descriptor
  key_ = k;
  peer_ = p;
  sock_ = f;
  seq_no_ = 0;
  aborted_ = 0;
  version_ = 0;
  session_id_ = d;
  unlock();
}

// destructor (socket will be closed in listener code)
TacacsPacket::~TacacsPacket() {
  pthread_mutex_destroy(&mutex_);
}

void
TacacsPacket::lock() {
  pthread_mutex_lock(&mutex_);
}

void
TacacsPacket::unlock() {
  pthread_mutex_unlock(&mutex_);
}

// selector - on input - packet, in output number of it's type
//            0 - unknown type
int
TacacsPacket::selector(unsigned char *buf) {
  if(!buf) return 0;
  HDR *hdr = (HDR*)buf;

  switch(hdr->type) {
    case TAC_PLUS_AUTHEN:// authentication
      return 1;
      break;
    case TAC_PLUS_AUTHOR:// authorization
      return 2;
      break;
    case TAC_PLUS_ACCT:// accounting
      return 3;
      break;
    default:// unknown
      return 0;
      break;
  }
}

void
TacacsPacket::setSessionId(unsigned int d) {
  lock();
  session_id_ = d;
  unlock();
}

unsigned int
TacacsPacket::getSessionId() {
  unsigned int ret;
  lock();
  ret = session_id_;
  unlock();
  return ret;
}

void
TacacsPacket::setAborted(int d) {
  lock();
  aborted_ = d;
  unlock();
}

int
TacacsPacket::getAborted() {
  int ret;
  lock();
  ret = aborted_;
  unlock();
  return ret;
}

void
TacacsPacket::setSeqNo(int d) {
  lock();
  seq_no_ = d;
  unlock();
}

int
TacacsPacket::getSeqNo() {
  int ret;
  lock();
  ret = seq_no_;
  unlock();
  return ret;
}

void
TacacsPacket::setSock(int d) {
  lock();
  sock_ = d;
  unlock();
}

int
TacacsPacket::getSock() {
  int ret;
  lock();
  ret = sock_;
  unlock();
  return ret;
}

void
TacacsPacket::setLastExch(time_t d) {
  lock();
  last_exch_ = d;
  unlock();
}

time_t
TacacsPacket::getLastExch() {
  time_t ret;
  lock();
  ret = last_exch_;
  unlock();
  return ret;
}

void
TacacsPacket::setKey(string d) {
  lock();
  key_ = d;
  unlock();
}

string
TacacsPacket::getKey() {
  string ret;
  lock();
  ret = key_;
  unlock();
  return ret;
}

void
TacacsPacket::setKeyline(int d) {
  lock();
  keyline_ = d;
  unlock();
}

int
TacacsPacket::getKeyline() {
  int ret;
  lock();
  ret = keyline_;
  unlock();
  return ret;
}

void
TacacsPacket::setPeer(string d) {
  lock();
  peer_ = d;
  unlock();
}

string
TacacsPacket::getPeer() {
  string ret;
  lock();
  ret = peer_;
  unlock();
  return ret;
}

void
TacacsPacket::setVersion(unsigned char d) {
  lock();
  version_ = d;
  unlock();
}

unsigned char
TacacsPacket::getVersion() {
  unsigned char ret;
  lock();
  ret = version_;
  unlock();
  return ret;
}

// create_md5_hash(): create an md5 hash of the "session_id", "the user's
// key", "the version number", the "sequence number", and an optional
// 16 bytes of data (a previously calculated hash). If not present, this
// should be NULL pointer.
//
// Write resulting hash into the array pointed to by "hash".
//
// The caller must allocate sufficient space for the resulting hash
// (which is 16 bytes long). The resulting hash can safely be used as
// input to another call to create_md5_hash, as its contents are copied
// before the new hash is generated.
void
TacacsPacket::create_md5_hash(int sess_id,
                    unsigned char v,
                    unsigned char seq,
                    unsigned char* prev_hash,
                    unsigned char* hash) {
  unsigned char *md_stream, *mdp;
  int md_len=0;
  MD5_CTX mdcontext;

  char k[32];
  bzero(k,sizeof(k));
  snprintf(k,sizeof(k)-1,"%s",key_.c_str());

  md_len = sizeof(sess_id)+strlen(k)+sizeof(v)+sizeof(seq);
  if(prev_hash) md_len += MD5_LEN;
  md_stream = new unsigned char[md_len];
  mdp = md_stream;
  bcopy(&sess_id, mdp, sizeof(sess_id));
  mdp += sizeof(sess_id);
  bcopy(k, mdp, strlen(k));
  mdp += strlen(k);

  bcopy(&v, mdp, sizeof(v));
  mdp += sizeof(v);
  bcopy(&seq, mdp, sizeof(seq));
  mdp += sizeof(seq);
  if(prev_hash) {
    bcopy(prev_hash, mdp, MD5_LEN);
    mdp += MD5_LEN;
  }
  MD5_Init(&mdcontext);
  MD5_Update(&mdcontext, md_stream, md_len);
  MD5_Final(hash, &mdcontext);

  delete [] md_stream;
  return;
}

// Overwrite input data with en/decrypted version by generating an
// MD5 hash and xor'ing data with it.
// When more than 16 bytes of hash is needed, the MD5 hash is performed
// again with the same values as before, but with the previous hash value
// appended to the MD5 input stream.
bool
TacacsPacket::md5_xor(HDR* hdr, unsigned char* data) {
  int i, j;
  unsigned char hash[MD5_LEN];// the md5 hash
  unsigned char last_hash[MD5_LEN];// the last hash we generated
  unsigned char *prev_hashp=(unsigned char*)0; // pointer to last
                                              // created hash
  int data_len;
  int sess_id;
  //unsigned char ver;
  unsigned char seq;

  bzero(hash, MD5_LEN);
  bzero(last_hash, MD5_LEN);

  data_len = ntohl(hdr->datalength);
  sess_id = hdr->session_id;       // always in network order for hashing
  version_ = hdr->version;
  seq = hdr->seq_no;

  if(key_ != "none") {
    for(i=0; i<data_len; i+=16) {
      create_md5_hash(sess_id, version_, seq, prev_hashp, hash);
      bcopy(hash, last_hash, MD5_LEN);
      prev_hashp = last_hash;
      //
      for(j=0; j<16; j++) {
        if((i+j) >= data_len) {
          hdr->encryption = (hdr->encryption == TAC_PLUS_CLEAR)
            ? TAC_PLUS_ENCRYPTED : TAC_PLUS_CLEAR;
          return true;
        }
        data[i + j] ^= hash[j];
      }
    }
    hdr->encryption = (hdr->encryption == TAC_PLUS_CLEAR)
      ? TAC_PLUS_ENCRYPTED : TAC_PLUS_CLEAR;
  } else {
    hdr->encryption = TAC_PLUS_CLEAR;
  }
  return true;
}

// read_packet - Read a packet and decrypt it from TACACS+ server
// return packet, if NULL - failure
unsigned char*
TacacsPacket::read_packet() {
  HDR hdr;
  unsigned char *pkt = 0;
  unsigned char *data;
  int len;
  bzero((char*)&hdr, sizeof(hdr));
  lock();
  // read a packet header
  len = sockread((unsigned char *)&hdr,
    TAC_PLUS_HDR_SIZE, TAC_PLUS_READ_TIMEOUT);
  if(len != TAC_PLUS_HDR_SIZE) {
    startlog(LOG_ERROR)<<peer_<<": illegal header size"<<endlog;
    unlock();
    return 0;
  }
  if((hdr.version & TAC_PLUS_MAJOR_VER_MASK) != TAC_PLUS_MAJOR_VER) {
    startlog(LOG_ERROR)<<peer_<<": Illegal major version specified: found "<<
      hdr.version<<" wanted "<<TAC_PLUS_MAJOR_VER<<endlog;
    unlock();
    return 0;
  }
  // get memory for the packet
  len = TAC_PLUS_HDR_SIZE + ntohl(hdr.datalength);
  if(len > 10000) {
    startlog(LOG_INFORM)<<"Too long packet (>10000) from "<<peer_<<
      "discarding"<<endlog;
    unlock();
    return 0;
  }
 try {
  pkt = new unsigned char[len + 3]; // occupy mem with length
  bzero(pkt,sizeof(char)*(len+3)); // clear
  bcopy(&hdr, pkt, TAC_PLUS_HDR_SIZE); // initialise the packet
  data = pkt + TAC_PLUS_HDR_SIZE; // the data start here
  // read the rest of the packet data
  if(sockread(data, ntohl(hdr.datalength),
      TAC_PLUS_READ_TIMEOUT) != (int)ntohl(hdr.datalength)) {
    startlog(LOG_EVENT)<<peer_<<": start_session: bad socket read"<<endlog;
    throw 0;
  }
  seq_no_++; // should now equal that of incoming packet
  last_exch_ = time(0); // time of exchange
  if(seq_no_ != hdr.seq_no) {
//std::cerr<<"Error with illegal session sequence"<<std::endl;
    startlog(LOG_ERROR)<<peer_<<
        ": (read_packet1)Illegal session seq #"<<seq_no_<<" != session seq in packet #"<<hdr.seq_no<<endlog;
    throw 0;
  }
  // decrypt the data portion
  if(!md5_xor(&hdr, data)) {
//std::cerr<<"Error with descrypting"<<std::endl;
    startlog(LOG_EVENT)<<peer_<<": start_session error decrypting data"<<endlog;
    throw 0;
  }
  version_ = hdr.version;
  unlock();
  return ((unsigned char*)pkt);
 }
 catch(int ret) {
  if(pkt) delete [] pkt;
  pkt = 0;
  unlock();
  return 0;
 }
}

// read_packet - Read a packet and decrypt it from TACACS+ server
// without header
unsigned char*
TacacsPacket::read_packet(unsigned char *h) {
//  lock();
  HDR hdr;
  unsigned char *pkt;
  unsigned char *data;
  int len;
  int datalen;
  pkt = 0;
  data = 0;
 try {
  bzero((char*)&hdr, sizeof(hdr));
  bcopy(h, &hdr, TAC_PLUS_HDR_SIZE); // set header
//std::cerr<<"packet header seq_no = "<<(int)hdr.seq_no<<std::endl;

//  if((hdr.version & TAC_PLUS_MAJOR_VER_MASK) != TAC_PLUS_MAJOR_VER) {
//    startlog(LOG_ERROR)<<peer_<<
//      ": Illegal major version specified: found "<<hdr.version<<" wanted "<<
//        TAC_PLUS_MAJOR_VER<<endlog;
//    throw 0;
//  }

  // get memory for the packet
  datalen = ntohl(hdr.datalength);
  len = TAC_PLUS_HDR_SIZE + datalen;
  if(len > 10000) {
    startlog(LOG_INFORM)<<
      "Too long packet (>10000) from "<<peer_<<", discarding, check keys"<<endlog;
    throw 0;
  }
  pkt = new unsigned char[len + 3]; // occupy mem with length
  bzero(pkt,sizeof(char)*(len+3)); // clear this mem
  data = pkt; // store start of ptr
  // read the rest of the packet data
  if(sockread(data, datalen,
      TAC_PLUS_READ_TIMEOUT) != datalen) {
    startlog(LOG_EVENT)<<peer_<<": start_session: bad socket read"<<endlog;
    throw 0;
  }
//std::cerr<<"*** add 1 seq_no in read_packet old="<<seq_no<<std::endl;
  seq_no_++; // should now equal that of incoming packet
//std::cerr<<"after, seq_no="<<seq_no<<std::endl;
  last_exch_ = time(0); // time of exchange
//std::cerr<<"Compare internal seq_no="<<seq_no<<" with packet seq="<<(int)hdr.seq_no<<std::endl;
  if(seq_no_ != (int)hdr.seq_no) {
    startlog(LOG_ERROR)<<peer_<<": (read_packet2)Illegal session seq #"<<seq_no_<<
        " != session seq #"<<hdr.seq_no<<" in packet"<<endlog;
//std::cerr<<"From "<<peer<<" illegal expected seq #"<<seq_no<<" != packet seq #"<<(int)hdr.seq_no<<std::endl;
    throw 0;
  }
  // decrypt the data portion
  if(!md5_xor(&hdr, data)) {
    startlog(LOG_EVENT)<<peer_<<": start_session error decrypting data"<<endlog;
    throw 0;
  }
  version_ = hdr.version;
//  unlock();
  return ((unsigned char*)pkt);
 }
 catch(int ret) {
  if(pkt) delete [] pkt;
//  unlock();
  return 0;
 }
}

// write_packet - Send a data packet to TACACS+ server
//       pak     pointer to packet data to send
//       return: true/false
bool
TacacsPacket::write_packet(unsigned char *pak) {
//  lock();
  unsigned char *data;
  int len = 0;
  HDR *hdr = (HDR*)pak;
  len = TAC_PLUS_HDR_SIZE + ntohl(hdr->datalength);
  data = pak + TAC_PLUS_HDR_SIZE; // the data start here
  // encrypt the data portion
  if(!md5_xor((HDR*)pak,data)) {
    startlog(LOG_ERROR)<<peer_<<": write_packet error encrypting data"<<endlog;
//    unlock();
    return false;
  }
  if(sockwrite(pak,len,TAC_PLUS_WRITE_TIMEOUT) != len) {
    startlog(LOG_ERROR)<<peer_<<": write_packet sockwrite error"<<endlog;
//    unlock();
    return false;
  }
  last_exch_ = time(0);
//  unlock();
  return true;
}

// free_avpairs
//void
//TacacsPacket::free_avpairs(char **avp) {
//  int i=0;
//  while(avp[i]) {
//    delete [] avp[i];
//    avp[i] = 0;
//    i++;
//  }
//}

///////////////////////////////////////////////////////////
// TACACS+ Client/Server classes
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////
//
//   AUTHENTICATION
//
///////////////////////////////////////////////////////

// authen_send_start - sending start authentication packet
//       (we are as client initiate connection)
//     port            tty10 or Async10
//     username
//     type
//     data            external data to tacacs+ server
bool
TacacsClient::authen_send_start(const char* port,
                      const char* username,
                      const int type,
                      const char* data) {
  char buf[256];
  char name[70];
  char addr[57];
  bzero(buf,sizeof(buf));
  bzero(name,sizeof(name));
  bzero(addr,sizeof(addr));

  HDR *hdr = (HDR *)buf;
  struct authen_start *ask =
      (struct authen_start*)(buf+TAC_PLUS_HDR_SIZE);
  char *u = (char*)
    (buf+TAC_PLUS_HDR_SIZE+TAC_AUTHEN_START_FIXED_FIELDS_SIZE);
  char *p = (char*)(u+strlen(username));
  char *a = (char*)(p+strlen(port));
  char *d;

  bzero(buf, sizeof(buf));// clear

  ipaddr our_addr;// our host ip
  //gethostname(name,sizeof(name));// this is address
  strncpy(addr,our_addr.get().c_str(),sizeof(addr)-1);

  d = (char*)(a + strlen(addr));
  //    header
  // version (TAC_PLUS_MAJOR_VER | TAC_PLUS_MINOR_VER_0)
  if(type==TACACS_ENABLE_REQUEST || type==TACACS_ASCII_LOGIN)
    hdr->version = TAC_PLUS_VER_0;
  else
    hdr->version = TAC_PLUS_VER_1;// for PPP(CHAP/PAP) we need in 1

  hdr->type = TAC_PLUS_AUTHEN;// packet type - AUTHENTICATION
//std::cerr<<"*** add 1 seq_no in read_packet old="<<seq_no<<std::endl;
  int seq_no = getSeqNo();
  hdr->seq_no = ++seq_no; // sequence for first request should be 1
  setSeqNo(seq_no);
  // encryption TAC_PLUS_ENCRYPTED || TAC_PLUS_CLEAR
  hdr->encryption = TAC_PLUS_CLEAR;
  hdr->session_id = htonl(getSessionId()); // session id
  // data length
  if(type == TACACS_CHAP_LOGIN || type == TACACS_MSCHAP_LOGIN)
    hdr->datalength = htonl(TAC_AUTHEN_START_FIXED_FIELDS_SIZE
      +strlen(username)+strlen(port)+strlen(addr)+1+strlen(data));
  else if(type == TACACS_PAP_LOGIN || type == TACACS_ARAP_LOGIN)
    hdr->datalength = htonl(TAC_AUTHEN_START_FIXED_FIELDS_SIZE
      +strlen(username)+strlen(port)+strlen(addr)+strlen(data));
  else
    hdr->datalength = htonl(TAC_AUTHEN_START_FIXED_FIELDS_SIZE
      +strlen(username)+strlen(port)+strlen(addr));
  ask->priv_lvl = TAC_PLUS_PRIV_LVL_MIN;       // privilege level

  switch(type) {
    case TACACS_ENABLE_REQUEST:
      ask->action = TAC_PLUS_AUTHEN_LOGIN;
      ask->service = TAC_PLUS_AUTHEN_SVC_ENABLE;
      break;
    case TACACS_ASCII_LOGIN:
      ask->action = TAC_PLUS_AUTHEN_LOGIN;
      ask->authen_type = TAC_PLUS_AUTHEN_TYPE_ASCII;
      ask->service = TAC_PLUS_AUTHEN_SVC_LOGIN;
      break;
    case TACACS_PAP_LOGIN:
      ask->action = TAC_PLUS_AUTHEN_LOGIN;
      ask->authen_type = TAC_PLUS_AUTHEN_TYPE_PAP;
      break;
    case TACACS_PAP_OUT:
      ask->action = TAC_PLUS_AUTHEN_SENDAUTH;
      ask->authen_type = TAC_PLUS_AUTHEN_TYPE_PAP;
      break;
    case TACACS_CHAP_LOGIN:
      ask->action = TAC_PLUS_AUTHEN_LOGIN;
      ask->authen_type = TAC_PLUS_AUTHEN_TYPE_CHAP;
      break;
    case TACACS_CHAP_OUT:
      ask->action = TAC_PLUS_AUTHEN_SENDAUTH;
      ask->authen_type = TAC_PLUS_AUTHEN_TYPE_CHAP;
      break;
    case TACACS_MSCHAP_LOGIN:
      ask->action = TAC_PLUS_AUTHEN_LOGIN;
      ask->authen_type = TAC_PLUS_AUTHEN_TYPE_MSCHAP;
      break;
    case TACACS_MSCHAP_OUT:
      ask->action = TAC_PLUS_AUTHEN_SENDAUTH;
      ask->authen_type = TAC_PLUS_AUTHEN_TYPE_MSCHAP;
      break;
    case TACACS_ARAP_LOGIN:
      ask->action = TAC_PLUS_AUTHEN_LOGIN;
      ask->authen_type = TAC_PLUS_AUTHEN_TYPE_ARAP;
      break;
    case TACACS_ASCII_CHPASS:
      ask->action = TAC_PLUS_AUTHEN_CHPASS;
      ask->authen_type = TAC_PLUS_AUTHEN_TYPE_ASCII;
      break;
  }
  // the length of fields in start packet
  // using without convertation ntohs or htons
  // (this is not clean in RFC)
  ask->user_len = strlen(username);  // username length
  if(ask->user_len > USERNAME_LEN) return false;
  ask->port_len = strlen(port);      // portname length
  if(ask->port_len > PORT_LEN) return false;
  ask->rem_addr_len = strlen(addr);  // addr length
  if(ask->rem_addr_len > REM_ADDR_LEN) return false;
  ask->data_len = strlen(data);      // data length
  if(strlen(username) > 0) strncpy(u,username,USERNAME_LEN-1); // user
  if(strlen(port) > 0)     strncpy(p,port,PORT_LEN-1);         // port
  if(strlen(addr) > 0)     strncpy(a,addr,REM_ADDR_LEN-1);     // addr
  if(type == TACACS_CHAP_LOGIN) {
    *d++ = 1;
    strcpy(d,data);
  }
  if(type == TACACS_ARAP_LOGIN || type == TACACS_PAP_LOGIN)
    strcpy(d,data);
  if(write_packet((unsigned char*)buf)) return true;
  return false;
}


//// TacacsServerAuthentication

// constructor
TacacsServerAuthentication::TacacsServerAuthentication(TacacsPacket *ts) {
  pthread_mutex_init(&mutex_,0);
  lock();
//  snprintf(loginstring,sizeof(loginstring),"%s",l);
//  snprintf(pwdstring,sizeof(pwdstring),"%s",pws);
  t = ts;
  // init
  //bzero(user,sizeof(user));
  //bzero(port,sizeof(port));
  //bzero(rem_addr,sizeof(rem_addr));
  bzero(data,sizeof(data));
  //bzero(cisco,sizeof(cisco));
  bzero(password,sizeof(password));
  //bzero(enable,sizeof(enable));
  chap = false;
  en_req = 0;
  buf_cont = 0;
  res_ = 0;
  request_ = 0;
  unlock();
}

TacacsServerAuthentication::~TacacsServerAuthentication() {
  pthread_mutex_destroy(&mutex_);
}

void
TacacsServerAuthentication::lock() {
  pthread_mutex_lock(&mutex_);
}

void
TacacsServerAuthentication::unlock() {
  pthread_mutex_unlock(&mutex_);
}

string
TacacsServerAuthentication::getUser() {
  lock();
  string ret = user_;
  unlock();
  return ret;
}

void
TacacsServerAuthentication::setUser(string d) {
  lock();
  user_ = d;
  unlock();
}

string
TacacsServerAuthentication::getEnable() {
  lock();
  string ret = enable_;
  unlock();
  return ret;
}

void
TacacsServerAuthentication::setEnable(string d) {
  lock();
  enable_ = d;
  unlock();
}

string
TacacsServerAuthentication::getPort() {
  lock();
  string ret = port_;
  unlock();
  return ret;
}

void
TacacsServerAuthentication::setPort(string d) {
  lock();
  port_ = d;
  unlock();
}

string
TacacsServerAuthentication::getRemAddr() {
  lock();
  string ret = rem_addr_;
  unlock();
  return ret;
}

void
TacacsServerAuthentication::setRemAddr(string d) {
  lock();
  rem_addr_ = d;
  unlock();
}

ipaddr
TacacsServerAuthentication::getCisco() {
  lock();
  ipaddr ret = cisco_;
  unlock();
  return ret;
}

void
TacacsServerAuthentication::setCisco(ipaddr d) {
  lock();
  cisco_ = d;
  unlock();
}

int
TacacsServerAuthentication::getRes() {
  lock();
  int ret = res_;
  unlock();
  return ret;
}

int
TacacsServerAuthentication::setRes(int d) {
  lock();
  res_ = d;
  unlock();
  return d;
}

int
TacacsServerAuthentication::getRequest() {
  lock();
  int ret = request_;
  unlock();
  return ret;
}

void
TacacsServerAuthentication::setRequest(int d) {
  lock();
  request_ = d;
  unlock();
}

//  authen_get_start  (server function)
//  review start packet
//  return 0 - error, type - success
int
TacacsServerAuthentication::get_start(unsigned char *h,unsigned char *pak,
          char *data) {
  int type = 0;
  //if(!pak) {
  //  TLOG(LOG_ERROR,"TACACS authen_get_start receive pak == NULL");
  //  return 0;
  //}
  HDR *hdr = (HDR *) h;
  struct authen_start *start=(struct authen_start *)pak;
//std::cerr<<"start->action="<<(int)start->action<<std::endl;
  char *u = (char *)
    (pak+TAC_AUTHEN_START_FIXED_FIELDS_SIZE);
  char *p = (char *)(u + (int)start->user_len);
  char *r = (char *)(p + (int)start->port_len);
  char *d = (char *)(r + (int)start->rem_addr_len);

  t->setVersion(hdr->version);
  if(t->getVersion()!=TAC_PLUS_VER_0 && t->getVersion()!=TAC_PLUS_VER_1) {
    startlog(LOG_ERROR)<<"Unknown packet version from "<<t->getPeer()<<", check tacacs+ key"<<endlog;
//std::cerr<<"Point 1"<<std::endl;
    return 0;
  }
  char username[USERNAME_LEN];
  bzero(username,USERNAME_LEN);
  char port[PORT_LEN];
  bzero(port,PORT_LEN);
  char rem_addr[REM_ADDR_LEN];
  bzero(rem_addr,REM_ADDR_LEN);
  bzero(data,DATA_LEN);
  char cisco[IP_LEN];
  bzero(cisco,IP_LEN);
  setCisco(t->getPeer().c_str());

  if(hdr->seq_no != 1) {
    startlog(LOG_ERROR)<<"authen_get_start: invalid sequence, check tacacs+ key"<<endlog;
//std::cerr<<"Point 2"<<std::endl;
    return 0;
  }
  startlog(LOG_TACACS)<<"start->action="<<start->action<<
      " start->service="<<start->service<<
        " start->authen_type="<<start->authen_type<<
          " version="<<t->getVersion()<<endlog;
  if(start->action == TAC_PLUS_AUTHEN_LOGIN &&
      start->service == TAC_PLUS_AUTHEN_SVC_ENABLE)
    type = TACACS_ENABLE_REQUEST;
  // ASCII CHPASS request
  if(start->action == TAC_PLUS_AUTHEN_CHPASS &&
      start->authen_type == TAC_PLUS_AUTHEN_TYPE_ASCII)
    type = TACACS_ASCII_CHPASS;
  // inbound ASCII login
  if(start->action == TAC_PLUS_AUTHEN_LOGIN &&
      (start->service == TAC_PLUS_AUTHEN_SVC_LOGIN ||
        start->service == TAC_PLUS_AUTHEN_SVC_H323_VSA) &&
      start->authen_type == TAC_PLUS_AUTHEN_TYPE_ASCII)
    type = TACACS_ASCII_LOGIN;
  // inbound PAP login
  if(start->action == TAC_PLUS_AUTHEN_LOGIN &&
      start->authen_type == TAC_PLUS_AUTHEN_TYPE_PAP &&
      t->getVersion() == TAC_PLUS_VER_1)
    type = TACACS_PAP_LOGIN;
  // outbound PAP request
  if(start->action == TAC_PLUS_AUTHEN_SENDAUTH &&
      start->authen_type == TAC_PLUS_AUTHEN_TYPE_PAP &&
      t->getVersion() == TAC_PLUS_VER_1)
    type = TACACS_PAP_OUT;
  // inbound CHAP login
  if(start->action == TAC_PLUS_AUTHEN_LOGIN &&
      start->authen_type == TAC_PLUS_AUTHEN_TYPE_CHAP &&
      t->getVersion() == TAC_PLUS_VER_1)
    type = TACACS_CHAP_LOGIN;
  // outbaund CHAP request
  if(start->action == TAC_PLUS_AUTHEN_SENDAUTH &&
      start->authen_type == TAC_PLUS_AUTHEN_TYPE_CHAP &&
      t->getVersion() == TAC_PLUS_VER_1)
    type = TACACS_CHAP_OUT;
  // inbound MS-CHAP login
  if(start->action == TAC_PLUS_AUTHEN_LOGIN &&
      start->authen_type == TAC_PLUS_AUTHEN_TYPE_MSCHAP &&
      t->getVersion() == TAC_PLUS_VER_1)
    type = TACACS_MSCHAP_LOGIN;
  // outbaund MS-CHAP request
  if(start->action == TAC_PLUS_AUTHEN_SENDAUTH &&
      start->authen_type == TAC_PLUS_AUTHEN_TYPE_MSCHAP &&
      t->getVersion() == TAC_PLUS_VER_1)
    type = TACACS_MSCHAP_OUT;
  // inbound ARAP login
  if(start->action == TAC_PLUS_AUTHEN_LOGIN &&
      start->authen_type == TAC_PLUS_AUTHEN_TYPE_ARAP &&
      t->getVersion() == TAC_PLUS_VER_1)
    type = TACACS_ARAP_LOGIN;
  // SENDPASS requests
  if(start->action == TAC_PLUS_AUTHEN_SENDPASS &&
      start->authen_type == TAC_PLUS_AUTHEN_TYPE_CHAP &&
      t->getVersion() == TAC_PLUS_VER_0)
    type = TACACS_CHAP_PASSWORD;
  if(start->action == TAC_PLUS_AUTHEN_SENDPASS &&
      start->authen_type == TAC_PLUS_AUTHEN_TYPE_PAP &&
      t->getVersion() == TAC_PLUS_VER_0)
    type = TACACS_PAP_PASSWORD;
  //////////////////////

  if(start->user_len > USERNAME_LEN) {
    startlog(LOG_ERROR)<<"start->user_len > USERNAME_LEN, check key please"<<endlog;
//std::cerr<<"Point 3"<<std::endl;
    return 0;
  }
//std::cerr<<"Point 3.1"<<std::endl;
  strncpy(username,u,start->user_len); // user
  username[start->user_len] = 0;
//std::cerr<<"Point 3.2"<<std::endl;
  setUser(username); // store username in internal structure
//std::cerr<<"Point 3.3 getUser="<<getUser()<<std::endl;
  if(start->port_len > PORT_LEN) {
    startlog(LOG_ERROR)<<"start->port_len > PORT_LEN, check key please"<<endlog;
//std::cerr<<"Point 4"<<std::endl;
    return 0;
  }
//std::cerr<<"Point 4.1"<<std::endl;
  strncpy(port,p,start->port_len); // port
  port[start->port_len] = 0;
  setPort(port); // store port in internal structure
  if(start->rem_addr_len > REM_ADDR_LEN) {
    startlog(LOG_ERROR)<<"start->rem_addr_len > REM_ADDR_LEN, check key please"<<endlog;
//std::cerr<<"Point 5"<<std::endl;
    return 0;
  }
//std::cerr<<"Point 5.1"<<std::endl;
  strncpy(rem_addr,r,start->rem_addr_len); // addr
  rem_addr[start->rem_addr_len] = 0;
  setRemAddr(rem_addr);
  if(start->data_len > DATA_LEN) {
    startlog(LOG_ERROR)<<"start->data_len > DATA_LEN, check key please"<<endlog;
//std::cerr<<"Point 6"<<std::endl;
    return 0;
  }
//std::cerr<<"Point 6.1"<<std::endl;
  strncpy(data,d,(int)start->data_len); // data
  data[start->data_len] = 0;
//std::cerr<<"data_len="<<(int)start->data_len<<" data="<<data<<std::endl;
  return type;
}

// send REPLY packet (server function)
// return status packet
// and set variables
bool
TacacsServerAuthentication::send_reply(const int status,
        const char* server_msg,
        const char* data,int flag,int ver) {
  char buf[512];
  bzero(buf,sizeof(buf)); // clean
  HDR *hdr = (HDR *)buf; // header
  // data
  struct authen_reply *dat = (struct authen_reply *)
    (buf+TAC_PLUS_HDR_SIZE);
  char *s = (char *)
    (buf+TAC_PLUS_HDR_SIZE+TAC_AUTHEN_REPLY_FIXED_FIELDS_SIZE);
  char *d = (char *)(s+strlen(server_msg));

  hdr->version = ver; // set version (1 - for CHAP/PAP)
  hdr->type = TAC_PLUS_AUTHEN;// type of packet - authentication
//std::cerr<<"*** add 1 seq_no in send_reply old="<<t->seq_no<<std::endl;
  int seq_no = t->getSeqNo();
  hdr->seq_no = ++seq_no; // sequence number
  t->setSeqNo(seq_no);
  hdr->encryption = TAC_PLUS_CLEAR; // encryption
  // !!!
  //   hdr->session_id = htonl(session->session_id);  // session id
  hdr->session_id = htonl(t->getSessionId());  // session id

  // packet length
  hdr->datalength = htonl(TAC_AUTHEN_REPLY_FIXED_FIELDS_SIZE
    +strlen(server_msg)+strlen(data));
  // compose packet
  dat->status = status;
  dat->flags = flag; // 1-noecho  0-echo
  dat->msg_len = htons(strlen(server_msg));
  dat->data_len = htons(strlen(data));
  if(strlen(server_msg) > 0) strcpy(s,server_msg);
  if(strlen(data) > 0) strcpy(d,data);

  if(t->write_packet((unsigned char*)buf)) return true;
  return false;
}

// get REPLY reply (client function)
// return status packet and set variables
//        return
//                -1      FAILURE
int
TacacsClient::authen_get_reply(char* server, char* datas) {
  char *buf = (char*)read_packet();
  if(!buf) return 0;
  HDR *hdr = (HDR *)buf; // header
  // static data
  struct authen_reply *rep=(struct authen_reply*)
    (buf+TAC_PLUS_HDR_SIZE);
  // server message
  char *serv_msg = (char *)
    (buf+TAC_PLUS_HDR_SIZE+TAC_AUTHEN_REPLY_FIXED_FIELDS_SIZE);
  // server data
  char *dat_pak = (char*)(serv_msg + ntohs(rep->msg_len));
  int mlen=0,dlen=0;

 try {
  bzero(server,sizeof(server));
  bzero(datas,sizeof(datas));
  // fields length
  mlen = ntohs(rep->msg_len);
  dlen = ntohs(rep->data_len);
  if(hdr->datalength != (int)htonl(TAC_AUTHEN_REPLY_FIXED_FIELDS_SIZE +
      mlen + dlen))  {
    startlog(LOG_ERROR)<<"authen_get_reply:invalid AUTHEN/REPLY packet, check key"<<endlog;
    throw 0;
  }
  // session->session_id = ntohl(hdr->session_id);
  if(mlen > 0) strncpy(server,serv_msg,mlen);
  if(dlen > 0) strncpy(datas,dat_pak,dlen);
  throw rep->status;
 }
 catch(int ret) {
  delete [] buf;
  buf = 0;
  return ret;
 }
}

// Send CONTINUE packet
// (client function)
bool
TacacsClient::authen_send_cont(const char* user_msg,
                    const char* data) {
  char buf[512];
  bzero(buf,sizeof(buf));
  HDR *hdr = (HDR *)buf;// header
  // datas
  struct authen_cont *ask = (struct authen_cont *)
    (buf + TAC_PLUS_HDR_SIZE);
  // packet
  char *p = (char *)
    (buf + TAC_PLUS_HDR_SIZE + TAC_AUTHEN_CONT_FIXED_FIELDS_SIZE);
  char *d = (char *)(p + strlen(user_msg));

  hdr->version = TAC_PLUS_VER_0; // version
  hdr->type = TAC_PLUS_AUTHEN; // packet type - AUTHENTICATION
  int tseq_no = getSeqNo();
  hdr->seq_no = ++tseq_no; // sequence number
  setSeqNo(tseq_no);
  hdr->encryption = TAC_PLUS_CLEAR; // encryption
  hdr->session_id = htonl(getSessionId()); // session id
  // packet length
  hdr->datalength = htonl(TAC_AUTHEN_CONT_FIXED_FIELDS_SIZE +
    strlen(user_msg) + strlen(data));
  ask->user_msg_len  = htons(strlen(user_msg));    // length of
  ask->user_data_len = htons(strlen(data));        // data
  // set datas
  if(strlen(user_msg) > 0) strcpy(p, user_msg);
  if(strlen(data) > 0) strcpy(d, data);
  // send packet
  if(write_packet((unsigned char *)buf)) return true;
  return false;
}

//  get_cont (server function)
//  get CONTINUE packet
//
//  -1 - error
//   1 - abort
//   0 - success
int
TacacsServerAuthentication::get_cont(unsigned char *h,unsigned char *pa,char *user_msg,
        char *data, int maxlen) {
  int len;

  if(!pa) {
    startlog(LOG_ERROR)<<"pa==NULL"<<endlog;
    return -1;
  }
  int flag;
  HDR *hdr = (HDR *)h; // header
  struct authen_cont *ask = (struct authen_cont *)pa; // data
  // packet
  char *p = (char *)(pa + TAC_AUTHEN_CONT_FIXED_FIELDS_SIZE);
  char *d = (char *)(p + ntohs(ask->user_msg_len));

  if(hdr->datalength != (int)htonl(TAC_AUTHEN_CONT_FIXED_FIELDS_SIZE +
      ntohs(ask->user_msg_len) + ntohs(ask->user_data_len))) {
    startlog(LOG_TACACS)<<"authen_get_cont: invalid AUTHEN/CONT, check keys"<<endlog;
    return -1;
  }
  flag = ask->flags;
  len = ntohs(ask->user_msg_len);
  if(len > maxlen) {
    startlog(LOG_TACACS)<<"user_msg_len="<<len<<" > maxlen="<<maxlen<<endlog;
    return -1;
  }

  strncpy(user_msg,p,len);
  len = ntohs(ask->user_data_len);
  if(len > DATA_LEN) {
    startlog(LOG_TACACS)<<"user_data_len="<<len<<" > DATA_LEN="<<DATA_LEN<<endlog;
    return -1;
  }
  strncpy(data,d,len);
  return flag;
}


////////////////////////////////////////////////////////////
//
//       authorization
//
////////////////////////////////////////////////////////////

// send request (client finction)
bool
TacacsClient::author_send_request(const int method,
              const int priv_lvl,
              const int authen_type,
              const int authen_service,
              const char *user,
              const char *port,
              char **avpair) {
  int i;
  //char name[100];
  char rem_addr[20];
  int arglens = 0;
  char buf[256];
  bzero(buf,sizeof(buf));
  HDR *hdr = (HDR *)buf;    // header
  // datas
  struct author *auth = (struct author *)
    (buf+TAC_PLUS_HDR_SIZE);
  char *lens = (char *)(buf+TAC_PLUS_HDR_SIZE+
    TAC_AUTHOR_REQ_FIXED_FIELDS_SIZE);

  hdr->version = TAC_PLUS_VER_0;
  hdr->type = TAC_PLUS_AUTHOR; // set packet type to authorization
  int tseq_no = getSeqNo();
  hdr->seq_no = ++tseq_no;
  setSeqNo(tseq_no);
  hdr->encryption = TAC_PLUS_CLEAR;
  hdr->session_id = htonl(getSessionId());

  ipaddr our_addr;
  strncpy(rem_addr,our_addr.get().c_str(),sizeof(rem_addr)-1);
  for(i=0; avpair[i] != NULL; i++) { // count length
    if(strlen(avpair[i]) > 255) // if lenght of AVP>255 set it to 255
      avpair[i][255] = 0;
    arglens += strlen(avpair[i]);
  }
  hdr->datalength = htonl(TAC_AUTHOR_REQ_FIXED_FIELDS_SIZE +
    i + strlen(user) + strlen(port) +
    	strlen(rem_addr) + arglens);
  auth->authen_method = (unsigned char) method;
  auth->priv_lvl      = (unsigned char) priv_lvl;
  auth->authen_type   = (unsigned char) authen_type;
  auth->service       = (unsigned char) authen_service;
  auth->user_len      = (unsigned char) strlen(user);
  auth->port_len      = (unsigned char) strlen(port);
  auth->rem_addr_len  = (unsigned char) strlen(rem_addr);
  auth->arg_cnt       = (unsigned char) i;
  for(i=0; avpair[i]; i++) {
    *lens = (unsigned char) strlen(avpair[i]);
    lens+=1;
  }
  // now filling some data
  if(strlen(user) > 0) {
    strcpy(lens,user);
    lens += strlen(user);
  }
  if(strlen(port) > 0) {
    strcpy(lens,port);
    lens += strlen(port);
  }
  if(strlen(rem_addr) > 0) {
    strcpy(lens,rem_addr);
    lens += strlen(rem_addr);
  }
  for(i=0; avpair[i]; i++) {
    strcpy(lens,avpair[i]);
    lens += (unsigned char)strlen(avpair[i]);
  }
  // now send
  if(write_packet((unsigned char *)buf)) return true;
  return false;
}

////// TacacsServerAuthorization

// constructor
TacacsServerAuthorization::TacacsServerAuthorization(TacacsPacket *ts, bool da) {
  pthread_mutex_init(&mutex_,0);
  lock();
  def_author = da;
  t = ts;
  curService    = 0; // store service=
  curProtocol   = 0; // store protocol=
  added    = 0;
  replaced = 0;
  start_av = 0;
  bavpair[0] = 0; // set start value
  avpair[0]  = 0; // too
  nas_avp = 0; // keep current avpair
  nmatch = 2;
  avps[0] = 0;
  flag1 = false;
  flag2 = false;
  flag3 = false;
  flag4 = false;

  method = 0;
  priv_lvl = 0;
  authen_type = 0;
  authen_service = 0;
  bzero(user,sizeof(user));
  bzero(port,sizeof(port));
  bzero(rem_addr,sizeof(rem_addr));
  unlock();
}

TacacsServerAuthorization::~TacacsServerAuthorization() {
  pthread_mutex_destroy(&mutex_);
}

void
TacacsServerAuthorization::lock() {
  pthread_mutex_lock(&mutex_);
}

void
TacacsServerAuthorization::unlock() {
  pthread_mutex_unlock(&mutex_);
}

// get request (server function)
//
// return false if fails
// true - if sussess
bool
TacacsServerAuthorization::get_request(unsigned char *h,unsigned char *p,
            int *method,
            int *priv_lvl,
            int *authen_type,
            int *authen_service,
            char *user,
            char *port,
            char *rem_addr,
            char **avpair) {
  int arglens = 0;
  int i;
  int l[255];// i think, not more 255 AV-pairs can be requested
  char ss[255];
  HDR *hdr = (HDR *)h;// header
  // data
  struct author *auth = (struct author *)p;
  char *lens = (char *)(p + TAC_AUTHOR_REQ_FIXED_FIELDS_SIZE);

  if(hdr->type != TAC_PLUS_AUTHOR) {     // checking
    startlog(LOG_EVENT)<<"author_get_request - this is no AUTHOR request"<<endlog;
    return false;
  }
  if(hdr->seq_no != 1) {
    startlog(LOG_TACACS)<<"author_get_request - error in sequence in AUTHOR/REQUEST"<<endlog;
    return false;
  }
  //t->session_id = ntohl(hdr->session_id);
  // count length
  for(i=0; i<auth->arg_cnt; i++)
    arglens += (int)*(lens+i);
  if(hdr->datalength != (int)htonl(TAC_AUTHOR_REQ_FIXED_FIELDS_SIZE +
      auth->arg_cnt + auth->user_len + auth->port_len +
        auth->rem_addr_len + arglens)) {
    startlog(LOG_TACACS)<<"author_get_request - error in AUTHOR/REQUEST, check keys"<<endlog;
    return false;
  }
  *method = auth->authen_method;
  *priv_lvl = auth->priv_lvl;
  *authen_type = auth->authen_type;
  *authen_service = auth->service;
  // count length
  for(i=0; i<auth->arg_cnt; i++) {
    l[i] = (int)*lens;
    lens++;
  }
  strncpy(user,lens,auth->user_len);
  user[auth->user_len] = 0;
  lens += auth->user_len;
  strncpy(port,lens,auth->port_len);
  port[auth->port_len] = 0;
  lens += auth->port_len;
  strncpy(rem_addr,lens,auth->rem_addr_len);
  rem_addr[auth->rem_addr_len] = 0;
  lens += auth->rem_addr_len;
  startlog(LOG_TACACS)<<"AUTHORIZ: get "<<auth->arg_cnt<<" arguments, rem_addr_len="<<auth->rem_addr_len<<endlog;
  // reviewing avpairs
  avpair[0] = 0;
  for(i=0 ; i<auth->arg_cnt; i++) {
    strncpy(ss,lens,l[i]);
    lens += l[i];
    ss[l[i]] = 0;       // set 0
    avpair[i] = utl::newstr(ss);
    avpair[i+1] = 0;
  }
  // hmmm, this is strange, but... I think, all...
  return true;
}

// send RESPONSE (server function) *
//
//     false - error
//     true  - success
bool
TacacsServerAuthorization::send_response(const int status,
                const char *server_msg,
                const char *data,
                char **avpair) {
  char buf[512];
  bzero(buf,sizeof(buf));
  HDR *hdr = (HDR *)buf;// header
  // data
  struct author_reply *auth = (struct author_reply *)
    (buf+TAC_PLUS_HDR_SIZE);
  char *lens = (char *)(buf+TAC_PLUS_HDR_SIZE+
    TAC_AUTHOR_REPLY_FIXED_FIELDS_SIZE);
  int arglens;
  arglens = 0;
  int i;
  i = 0;

  hdr->version = TAC_PLUS_VER_0;
  hdr->type = TAC_PLUS_AUTHOR;
  int seq_no = t->getSeqNo();
  hdr->seq_no = ++seq_no;
  t->setSeqNo(seq_no);
  hdr->encryption = TAC_PLUS_CLEAR;
  hdr->session_id = htonl(t->getSessionId());
  // count length
  if(avpair) {
    for(i=0; (avpair[i] != 0) && (strlen(avpair[i]) != 0); i++) {
	arglens += strlen(avpair[i]);
	*lens = (unsigned char)strlen(avpair[i]);
	lens++;
    }
  }
  hdr->datalength = htonl(TAC_AUTHOR_REPLY_FIXED_FIELDS_SIZE +
    i + strlen(server_msg) + strlen(data) + arglens);
  auth->status = (unsigned char)status;
  auth->arg_cnt = (unsigned char)i;
  auth->msg_len = (unsigned int)strlen(server_msg);
  auth->data_len = (unsigned int)strlen(data);
  // lens we are filled above
  strcpy(lens,server_msg);
  lens += strlen(server_msg);
  strcpy(lens,data);
  lens += strlen(data);
  // process avpairs
  if(avpair) {
    for(i=0; avpair[i] != 0; i++) {
      strcpy(lens,avpair[i]);
      lens += strlen(avpair[i]);
    }
  }
  // we can send it to NAS
  if(t->write_packet((unsigned char *)buf)) return true;
  return false;
}

// get RESPONSE (client function)  return status
int
TacacsClient::author_get_response(char *server_msg,
                    char *data,
                    char **avpair) {
  int status;
  char ss[255];
  char *buf = (char*)read_packet();
  if(!buf) return -1;

 try {
  struct author_reply *auth = (struct author_reply *)
    (buf+TAC_PLUS_HDR_SIZE);
  HDR *hdr = (HDR *)buf;
  char *lens = (char *)(buf+TAC_PLUS_HDR_SIZE+
    TAC_AUTHOR_REPLY_FIXED_FIELDS_SIZE);
  int l[255];    // I think, that not more 255 avpairs can be processed
  int arglens = 0;
  int i;

  // Do some checks
  if(hdr->type != TAC_PLUS_AUTHOR) {
    startlog(LOG_TACACS)<<"author_get_response: this is not AUTHOR request"<<endlog;
    throw 0;
  }
  if(hdr->seq_no != 2) {
    startlog(LOG_TACACS)<<"author_get_response: error in sequence in AUTHOR/RESPONSE"<<endlog;
    throw 0;
  }
  //session_id = ntohl(hdr->session_id);
  status = auth->status;
  avpair[0] = NULL;
  if (status == TAC_PLUS_AUTHOR_STATUS_ERROR) return(status);
  // count length
  for(i=0; i < auth->arg_cnt ; i++) {
    arglens += (int)(*(lens+i));
    l[i] = (int)(*(lens+i));
  }
  if(hdr->datalength != (int)htonl(TAC_AUTHOR_REPLY_FIXED_FIELDS_SIZE +
      auth->arg_cnt + auth->msg_len + auth->data_len + arglens)) {
    startlog(LOG_TACACS)<<"author_get_responce: error in AUTHOR/RESPONSE, check key"<<endlog;
    throw status;
  }
  lens = lens + i;
  strncpy(server_msg,lens,auth->msg_len);
  server_msg[auth->msg_len] = 0;
  lens += auth->msg_len;
  strncpy(data,lens,auth->data_len);
  data[auth->data_len] = 0;
  lens += auth->data_len;
  // write avpairs
  for(i=0; i < auth->arg_cnt ; i++) {
    strncpy(ss,lens,l[i]);
    lens = lens+l[i];
    ss[l[i]] = 0;    // set 0
    avpair[i] = utl::newstr(ss);
    avpair[i+1] = NULL;
  }
  throw status;
 }
 catch(int ret) {
  delete [] buf;
  buf = 0;
  return ret;
 }
}

// if this is optional argument (*)
bool
TacacsServerAuthorization::optional(char *str) {
  for(int i=0; str[i] != '\0'; i++)
    if(str[i] == '*') return true;
  return false;
}

// if this is mandatory argument (*)
bool
TacacsServerAuthorization::mandatory(char *str) {
  for(int i=0; str[i] != '\0'; i++)
    if(str[i] == '=') return true;
  return false;
}

// match AV attribute (av names)
bool
TacacsServerAuthorization::match_attr(char *str1,char *str2) {
  for(int i=0; str1[i] != '\0' && str2[i] != '\0'; i++) {
    if((str1[i] == '=' || str1[i] == '*') &&
      (str2[i] == '=' || str2[i] == '*')) return true;
    if(str1[i] != str2[i]) return false;
  }
  return false;
}

// match AV value
bool
TacacsServerAuthorization::match_value(char *str1,char *str2) {
  int i,j;
  for(i=0; str1[i] != '=' && str1[i] != '*' && str1[i] != '\0'; i++);
  for(j=0; str2[j] != '=' && str2[j] != '*' && str2[j] != '\0'; j++);
  if(utl::scomp(str1+i,str2+j)) return true;
  return false;
}

// get AV value
char*
TacacsServerAuthorization::get_value(char *str) {
  int i;
  for(i=0; str[i]!='=' && str[i]!='*' && str[i]!='\0'; i++);
  if(str[i] == '\0') return "";
  return(str+i+1);
}


//////////////////////////////////////////////////
//
//   ACCOUNTING
//
///////////////////////////////////////////////////

// send the accounting REQUEST  (client function)
bool
TacacsClient::account_send_request(const int flag,
          const int method,
          const int priv_lvl,
          const int authen_type,
          const int authen_service,
          const char *user,
          const char *port,
          char **avpair) {
  int i;
  char buf[512];
  bzero(buf,sizeof(buf));
  char rem_addr[20];
  //char name[100];
  HDR *hdr = (HDR *)buf;
  struct acct *acc = (struct acct *)(buf + TAC_PLUS_HDR_SIZE);
  char *lens = (char*)(buf+TAC_PLUS_HDR_SIZE+
    TAC_ACCT_REQ_FIXED_FIELDS_SIZE);
  int arglens = 0;

  //gethostname(name,sizeof(name));            // address
  ipaddr our_addr;
  strncpy(rem_addr,our_addr.get().c_str(),sizeof(rem_addr)-1);

  hdr->version = TAC_PLUS_VER_0;
  hdr->type = TAC_PLUS_ACCT;
  int tseq_no = getSeqNo();
  hdr->seq_no = ++tseq_no;
  setSeqNo(tseq_no);
  hdr->encryption = TAC_PLUS_CLEAR;
  hdr->session_id = htonl(getSessionId());
  for(i=0; avpair[i] != NULL ; i++) {
    if(strlen(avpair[i]) > 255)    // if lenght of AVP>255 set it to 255
      avpair[i][255] = 0;
    arglens += strlen(avpair[i]);
  }
  hdr->datalength = htonl(TAC_ACCT_REQ_FIXED_FIELDS_SIZE +
    i+strlen(user)+strlen(port)+strlen(rem_addr)+arglens);
  acc->authen_method  = (unsigned char)method;
  acc->priv_lvl       = (unsigned char)priv_lvl;
  acc->authen_type    = (unsigned char)authen_type;
  acc->authen_service = (unsigned char)authen_service;
  acc->user_len       = (unsigned char)strlen(user);
  acc->port_len       = (unsigned char)strlen(port);
  acc->rem_addr_len   = (unsigned char)strlen(rem_addr);
  acc->arg_cnt        = (unsigned char)i;
  for(i=0; avpair[i]; i++) {
    *lens = (u_char)strlen(avpair[i]);
    lens = lens+1;
  }
  // filling data
  if(strlen(user) > 0) {
    strcpy(lens,user);
    lens += strlen(user);
  }
  if(strlen(port) > 0) {
    strcpy(lens,port);
    lens += strlen(port);
  }
  if(strlen(rem_addr) > 0) {
    strcpy(lens,rem_addr);
    lens += strlen(rem_addr);
  }
  for(i=0; avpair[i]; i++) {
    strcpy(lens,avpair[i]);
    lens += (u_char)strlen(avpair[i]);
  }
  if (write_packet((unsigned char *)buf)) return true;
  return false;
}


///// TacacsServerAccounting

// constructor
TacacsServerAccounting::TacacsServerAccounting(TacacsPacket *ts) {
  pthread_mutex_init(&mutex_,0);
  t = ts;
}

// free
TacacsServerAccounting::~TacacsServerAccounting() {
  string stmp;
  lock();
  for(list<string>::iterator i=avpair.begin();i!=avpair.end() && !avpair.empty();) {
    stmp = *i;
    i = avpair.erase(i);
    stmp.erase();
  }
  avpair.clear();
  unlock();
  pthread_mutex_destroy(&mutex_);
}

void
TacacsServerAccounting::lock() {
  pthread_mutex_lock(&mutex_);
}

void
TacacsServerAccounting::unlock() {
  pthread_mutex_unlock(&mutex_);
}

//  get the account REQUEST (server function)
//  return accounting flag if sussess
//  0 - fail
int
TacacsServerAccounting::get_request(unsigned char *buf,unsigned char *bbb,
        int *method,
        int *priv_lvl,
        int *authen_type,
        int *authen_service,
        char *user,
        char *port,
        char *rem_addr,
        char *nas) {
  if(!buf) return 0;
  int flag;
  int i;
  int l[255];
  char str[128];
  HDR *hdr = (HDR *)buf;
  struct acct *acc = (struct acct *)bbb;	//(buf + TAC_PLUS_HDR_SIZE);
  char *lens = (char*)(bbb + TAC_ACCT_REQ_FIXED_FIELDS_SIZE);
  int arglens = 0;

  // some checks
  if(hdr->type != TAC_PLUS_ACCT) {
    startlog(LOG_TACACS)<<"account_get_request: this is not ACCOUNT request"<<endlog;
    return 0;
  }
  if(hdr->seq_no != 1) {
    startlog(LOG_TACACS)<<"account_get_request: error sequence in ACCOUNT/REQUEST"<<endlog;
    return 0;
  }
  //t->session_id = ntohl(hdr->session_id);

  strncpy(nas,t->getPeer().c_str(),IP_LEN);
  // count length
  for(i=0; i < acc->arg_cnt; i++)
    arglens += (int)*(lens+i);
  if(hdr->datalength != (int)htonl(TAC_ACCT_REQ_FIXED_FIELDS_SIZE+
    acc->arg_cnt+acc->user_len+acc->port_len+acc->rem_addr_len+
      arglens)) {
    startlog(LOG_TACACS)<<"account_get_request: error in ACCOUNT/REQUEST, check keys"<<endlog;
    return 0;
  }
  flag            = acc->flags;
  *method         = acc->authen_method;
  *priv_lvl       = acc->priv_lvl;
  *authen_type    = acc->authen_type;
  *authen_service = acc->authen_service;
  // count length
  for(i=0; i < acc->arg_cnt; i++) {
    l[i] = (int)(*lens);
    lens++;
  }
  bzero(user,sizeof(user));
  strncpy(user,lens,acc->user_len);
  lens += acc->user_len;
  bzero(port,sizeof(port));
  strncpy(port,lens,acc->port_len);
  lens += acc->port_len;
  bzero(rem_addr,sizeof(rem_addr));
  strncpy(rem_addr,lens,acc->rem_addr_len);
  lens += acc->rem_addr_len;
  // reviewing avpairs
  for(i=0 ; i < acc->arg_cnt; i++) {
    bzero(str,sizeof(str));           // clear
    strncpy(str,lens,l[i]);           // copy
    avpair.push_back(str);
    lens += l[i];                     // move
  }
  return flag;
}

// send the accounting REPLY (server function)
bool
TacacsServerAccounting::send_reply(char *server_msg,
             char *data,
             const int status) {
  char buf[512];
  bzero(buf,sizeof(buf));
  HDR *hdr = (HDR *)buf;
  struct acct_reply *acc = (struct acct_reply *)(buf + TAC_PLUS_HDR_SIZE);
  char *lens = (char*)(buf+TAC_PLUS_HDR_SIZE+
    TAC_ACCT_REPLY_FIXED_FIELDS_SIZE);
  hdr->version = TAC_PLUS_VER_0;
  hdr->type   = TAC_PLUS_ACCT;
  int seq_no = t->getSeqNo();
  hdr->seq_no = ++seq_no;
  t->setSeqNo(seq_no);
  hdr->encryption = TAC_PLUS_CLEAR;
  hdr->session_id = htonl(t->getSessionId());
  hdr->datalength = htonl(TAC_ACCT_REPLY_FIXED_FIELDS_SIZE +
    strlen(server_msg) + strlen(data));
  acc->msg_len  = (unsigned short)strlen(server_msg);
  acc->data_len = (unsigned short)strlen(data);
  acc->status   = status;
  if(strlen(server_msg) > 0) {
    strcpy(lens,server_msg);
    lens += strlen(server_msg);
  }
  if(strlen(data) > 0) {
    strcpy(lens,data);
    lens += strlen(data);
  }
  if(t->write_packet((unsigned char *)buf)) return true;     // that's all
  return false;
}

//   get the accounting REPLY (client function)
//      1  SUCCESS
//      0  FAILURE
int
TacacsClient::account_get_reply(char *server_msg,char *data) {
  int status;
  char *buf = (char*)read_packet();
  if(!buf) return -1;
  HDR *hdr = (HDR *)buf;
  struct acct_reply *acc = (struct acct_reply *)(buf + TAC_PLUS_HDR_SIZE);
  char *lens = (char*)(buf + TAC_PLUS_HDR_SIZE +
    TAC_ACCT_REPLY_FIXED_FIELDS_SIZE);
 try {
  if(hdr->type != TAC_PLUS_ACCT) // some checks
    throw "account_get_reply: this is not ACCOUNT request";
  if(hdr->seq_no != 2)
    throw "account_get_reply: error in sequence in ACCOUNT/REQUEST";
  //session_id = ntohl(hdr->session_id);
  if(hdr->datalength != (int)htonl(TAC_ACCT_REPLY_FIXED_FIELDS_SIZE+
        acc->msg_len + acc->data_len))
    throw "account_get_reply: error in ACCOUNT/REPLY packet, check keys";
  status = acc->status;
  bzero(server_msg,sizeof(server_msg));
  strncpy(server_msg,lens,acc->msg_len);
  lens = lens + acc->msg_len;
  bzero(data,sizeof(data));
  strncpy(data,lens,acc->data_len);
  delete [] buf;
  buf = 0;
  return status;
 }
 catch(char *str) {
  delete [] buf;
  buf = 0;
  startlog(LOG_TACACS)<<"TacacsClient response: "<<str<<endlog;
  return 0;
 }
}

///////////////////// that's all ////////////////////////
