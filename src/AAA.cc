// tacppd TACACS+/RADIUS universal AAA code
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

#include "global.h"

////////////////// AUTHENTICATION /////////////////////

AAA::AAA() {
  pthread_mutex_init(&mutex_,0);
}

AAA::~AAA() {
  pthread_mutex_destroy(&mutex_);
}

void
AAA::lock() {
  pthread_mutex_lock(&mutex_);
}

void
AAA::unlock() {
  pthread_mutex_unlock(&mutex_);
}

// Compare passwords
// cpass(password from base,password from nas)
//   return false - not equal, true - equal
bool
AAA::cpass(const string pwdu,const string pwdn) {
  char *chal, digest[MD5_LEN];
  unsigned char *mdp, *md_context;
  char id;
  int chal_len, inlen;
  MD5_CTX mdcontext;
  char pwduser[DB_PASSWORD_LEN];
  char pwdnas[DB_PASSWORD_LEN];

  if(pwdu.size()==0 || pwdn.size()==0) return false;

  // !!! possible security problem - If we have crypted database
  // data and user insert password in form of hash?
  if(pwdu == pwdn) return true; // cleartext pwds compared

  snprintf(pwduser,sizeof(pwduser),"%s",pwdu.c_str());
  snprintf(pwdnas,sizeof(pwdnas),"%s",pwdn.c_str());
  bzero(digest,sizeof(digest));
  id = pwdnas[0];
  chal_len = strlen(pwdnas)-1-MD5_LEN;
  if(chal_len < 0) return false;
  // We now have the secret, the id, and the challenge value. Put them all
  // together, and run them through the MD5 digest algorithm.
  inlen = sizeof(unsigned char) + strlen(pwduser) + chal_len;

  md_context = new unsigned char[inlen];  // i'd like use new/delete
  mdp = md_context;

  mdp[0] = id;
  bcopy(pwduser, &mdp[1], strlen(pwduser));
  chal = pwdnas + 1;
  bcopy(chal, mdp + strlen(pwduser) + 1, chal_len);
  MD5_Init(&mdcontext);
  MD5_Update(&mdcontext, mdp, inlen);
  MD5_Final((unsigned char *)digest, &mdcontext);

  delete [] md_context;
  // Now compare the received response value with the just calculated
  // digest value.  If they are equal, it's a pass, otherwise it's a
  // failure
  if(bcmp(digest, pwdnas + 1 + chal_len, MD5_LEN)) {
    startlog(LOG_AAA)<<"cpass: user password CHAP check false"<<endlog;
    return false;
  } else {
    startlog(LOG_AAA)<<"cpass: user password CHAP check true"<<endlog;
    return true;
  }
}

// authentication via database
bool
AAA::db_authentication(const string dba_user,
  const string dba_password,
    const string dba_rem_addr,
      const ipaddr dba_nas_addr,
        const string dba_port,
          const string dba_enable) {
  startlog(LOG_INFORM)<<"Authenticate user "<<dba_user<<endlog;
//  lock();
 try {
//cerr<<"db_auth point 1"<<endl;
  db_user data = ::userTrack.get2add(dba_user,dba_nas_addr,dba_port,dba_rem_addr); // fill users data
//cerr<<"db_auth point 2"<<endl;
  if(!data.getResult()) {
    startlog(LOG_AAA)<<"For entry in DB "<<dba_user<<":"<<data.getResultMsg()<<endlog;
    startlog(LOG_INFORM)<<
      "Tryed username "<<dba_user<<" with message "<<data.getResultMsg()<<
          " from "<<dba_nas_addr<<"/"<<dba_rem_addr<<"/"<<dba_port<<endlog;
    throw 0;
  }
  if(data.getMaxsess() != 0) {
    int cou = ::coreData->getDevice()->numsess(dba_user);
    if(cou >= data.getMaxsess()) {
      startlog(LOG_INFORM)<<"Too many sessions open ("<<
          cou<<") for user "<<dba_user<<"/"<<dba_port<<
            " (allowed "<<data.getMaxsess()<<")"<<endlog;
      throw 0;
    }
  }
  // check by open/close time
  time_t t = time(0); // get computer time
  if(data.getOpen()>t || (data.getClose()<t && data.getClose()>100)) {
    startlog(LOG_ERROR)<<"Cant do access for "<<dba_user<<
        " due open/close time mismatch"<<endlog;
    throw 0;
  }
//std::cerr<<"db check for user="<<dba_user.c_str()<<" pwd="<<dba_password.get()<<" dpwd="<<data.password.get()<<" en=\"<<dba_enable.get()<<" den="<<data.enable.get()<<std::endl;

  if(utl::scomp(data.getAuthsrc().c_str(),"db",2)) { // check for password
    if(dba_password == "(null)") throw 0; // this is try for "nopassword"
    if(!cpass(data.getPassword(),dba_password) && !cpass(data.getEnable(),dba_enable)) {
//std::cerr<<"Wrong password compare for "<<dba_user<<"/"<<dba_password<<" with "<<data.password<<std::endl;
      startlog(LOG_INFORM)<<"Wrong user/password "<<dba_user<<
          "/"<<dba_password<<" from "<<dba_nas_addr<<"/"<<
            dba_rem_addr<<"/"<<dba_port<<endlog;
      throw 0;
    }
  } else if(utl::scomp(data.getAuthsrc().c_str(),"cpwd",4)) { // clear text password
    if(data.getPassword()!=dba_password && data.getEnable()!=dba_enable) {
      startlog(LOG_INFORM)<<"Wrong user/clear text password "<<dba_user<<
        "/"<<dba_password<<" from "<<dba_nas_addr<<"/"<<dba_rem_addr<<
          "/"<<dba_port<<endlog;
      throw 0;
    }
  } else if(utl::scomp(data.getAuthsrc().c_str(),"ccf",3)) {
    // cryptocard file
  } else if(utl::scomp(data.getAuthsrc().c_str(),"ccd",3)) {
    // cryptocard database table
  } else if(!utl::scomp(data.getAuthsrc().c_str(),"nopwd",5)) throw 0;
  bool flag  = true;
  bool flag1 = false;
  bool flag2 = false;
  bool flag3 = false;
  bool flag4 = false;
  regex_t preg;
  size_t nmatch=2;
  regmatch_t pmatch[2];
  // check by table checkfrom
  db_user_acc acc;
  for(int k=0; k < data.acc_size(); k++) {
    acc = data.acc_get(k);
    // check by rem_addr (phone)
    if(regcomp(&preg,acc.getPhone().c_str(),REG_EXTENDED) == 0) {
      if(regexec(&preg,dba_rem_addr.c_str(),nmatch,pmatch,0) == 0) {
        flag1 = true;
        startlog(LOG_AAA)<<" - check by rem_addr(phone) ("<<acc.getPhone()<<
            " with "<<dba_rem_addr<<") success"<<endlog;
      } else {
        startlog(LOG_AAA)<<" - check by rem_addr(phone) ("<<acc.getPhone()<<
            " with "<<dba_rem_addr<<") failed"<<endlog;
      }
      regfree(&preg);
    }
    // check by nas_addr (ip addr of nas)
    if(regcomp(&preg,acc.getNas().c_str(),REG_EXTENDED) == 0) {
      if(regexec(&preg,((ipaddr)dba_nas_addr).get().c_str(),nmatch,pmatch,0) == 0) {
        flag2 = true;
        startlog(LOG_AAA)<<" - check by nas_addr ("<<acc.getNas()<<
          " with "<<dba_nas_addr<<") success"<<endlog;
      } else {
        startlog(LOG_AAA)<<" - check by nas_addr ("<<acc.getNas()<<
          " with "<<dba_nas_addr<<") failed"<<endlog;
      }
      regfree(&preg);
    }
    // check by port
    if(dba_port.size() > 0) {
      if(regcomp(&preg,acc.getPort().c_str(),REG_EXTENDED) == 0) {
        if(regexec(&preg,dba_port.c_str(),nmatch,pmatch,0) == 0) {
          flag3 = true;
          startlog(LOG_AAA)<<" - check by port ("<<acc.getPort()<<
            " with "<<dba_port<<") success"<<endlog;
        } else {
          startlog(LOG_AAA)<<" - check by port ("<<acc.getPort()<<
            " with "<<dba_port<<") failed"<<endlog;
        }
        regfree(&preg);
      }
    } else flag3 = true;
    if(acc.getTime().size()>0) {
      if(utl::check_time(acc.getTime().c_str())) flag4 = true;
      if(flag4 == true)
        startlog(LOG_AAA)<<" - check by time "<<acc.getTime()<<" with true"<<endlog;
      else
        startlog(LOG_AAA)<<" - check by time "<<acc.getTime()<<" with false"<<endlog;
    } else flag4 = true;
    if(flag1 && flag2 && flag3 && flag4) {
      flag = true;
      break;
    } else flag=false;
  }
  if(!flag) {
    startlog(LOG_INFORM)<<
      "Try to access from deny source "<<dba_rem_addr<<" from "<<dba_nas_addr<<
        "/"<<dba_port<<" for "<<dba_user<<endlog;
    startlog(LOG_AAA)<<
      "Deny source "<<dba_rem_addr<<" from "<<dba_nas_addr<<
        "/"<<dba_port<<" for "<<dba_user<<", Authen failed"<<endlog;
    throw 0;
  }
  throw 1;
 }
 catch(int a) {
//  unlock();
  if(a != 0) return true;
  return false;
 }
}

// Authorization DB function
db_user
AAA::db_authorization(const string user,const ipaddr peer,const string port,const string rem_addr) {
//  lock();
  db_user ret;
  db_user dbd;
  dbd = ::userTrack.get2add(user,peer,port,rem_addr);
  if(!dbd.getResult()) {
    startlog(LOG_INFORM)<<
      "Tryed authorize username "<<user<<" with message "<<dbd.getResultMsg()<<
          " from "<<peer<<"/"<<rem_addr<<"/"<<port<<endlog;
//    unlock();
    return dbd;
  }
  // we should get rem_addr from userTrack system - it can
  // have more actual information
  DeviceUser_data tmp;
  string ra;
  int i = 0;
  while(true) {
    tmp = ::coreData->getDevice()->getuser(i);
    if(!tmp.isPresent()) break;
    if(tmp.getNasAddr()==peer && tmp.getPort()==port) {
      ra = tmp.getPhone();
      break;
    }
    i++;
  }
//std::cerr<<"Call modifyer with user="<<user<<" peer="<<peer.get()<<" port="<<port<<" rem_addr="<<rem_addr<<std::endl;
  AModify *avpm = new AModify(user, peer, port, ra); // create modifyer object
  ret = avpm->modifyer(dbd);// call modify function
  delete avpm;
//  unlock();
  return ret;
}

/////////////////// ACCOUNTING ////////////////////////

// logging to db and logfile
void
AAA::db_accounting(list<string> logavp) {
//  lock();
  string logstr;
  log_user *l = new log_user;
  string str;
  for(list<string>::iterator i=logavp.begin(); i!=logavp.end() && !logavp.empty();) {
    str = *i;
    i = logavp.erase(i);
    if(str.size()>0) {
      logstr = logstr + str;
      logstr = logstr + "\t";
    }
    if(utl::scomp((char*)str.c_str(),"action=",7)) {
      if(utl::scomp((char*)str.c_str()+7,"START")) l->action = 1;
      if(utl::scomp((char*)str.c_str()+7,"UPDATE")) l->action = 2;
      if(utl::scomp((char*)str.c_str()+7,"STOP")) l->action = 3;
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"user=",5)) {
      l->username = (char*)str.c_str()+5;
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"port=",5)) {
      l->port = (char*)str.c_str()+5;
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"from=",5)) {
      l->from = (char*)str.c_str()+5;
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"nas=",4)) {
      l->nas.fill((char*)str.c_str()+4);
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"service=",8)) {
      l->service = (char*)str.c_str()+8;
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"protocol=",9)) {
      l->protocol = (char*)str.c_str()+9;
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"address=",8)) {
      l->ip.fill((char*)str.c_str()+8);
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"addr=",5)) {
      l->ip.fill((char*)str.c_str()+5);
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"bytes_in=",9)) {
      l->bytes_in = atoi((char*)str.c_str()+9);
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"bytes_out=",10)) {
      l->bytes_out = atoi((char*)str.c_str()+10);
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"elapsed_time=",13)) {
      l->elapsed = atoi((char*)str.c_str()+13);
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"disc-cause=",11)) {
      l->disc_cause = atoi((char*)str.c_str()+11);
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"disc-cause-ext=",15)) {
      l->disc_cause_ext = atoi((char*)str.c_str()+15);
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"nas-rx-speed=",13)) {
      l->rx_speed = atoi((char*)str.c_str()+13);
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"nas-tx-speed=",13)) {
      l->tx_speed = atoi((char*)str.c_str()+13);
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"start_time=",11)) {
      l->start_time = (time_t)atol((char*)str.c_str()+11);
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"task_id=",8)) {
      l->task_id = atoi((char*)str.c_str()+8);
      continue;
    }
    // VoIP avpairs processing
    if(utl::scomp((char*)str.c_str(),"h323-call-origin=",17)) {
      l->h323_call_origin = (char*)str.c_str()+17;
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"h323-call-type=",15)) {
      l->h323_call_type = (char*)str.c_str()+15;
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"h323-disconnect-cause=",22)) {
      l->h323_disconnect_cause = atoi((char*)str.c_str()+22);
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"h323-voice-quality=",19)) {
      l->h323_voice_quality = atoi((char*)str.c_str()+19);
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"h323-remote-address=",20)) {
      l->h323_remote_address.fill((char*)str.c_str()+20);
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"h323-connect-time=",18)) {
      l->h323_connect_time = utl::h323time2time((char*)str.c_str()+18);
      continue;
    }
    if(utl::scomp((char*)str.c_str(),"h323-disconnect-time=",21)) {
      l->h323_disconnect_time = utl::h323time2time((char*)str.c_str()+21);
      continue;
    }
//    if(utl::scomp((char*)str.c_str(),"faxrelay-start-time=",20)) {
//      l->h323_disconnect_time = utl::h323time2time((char*)str.c_str()+21);
//      continue;
//    }
  }
  logavp.clear();

  TLOG2.out_logging(logstr);  // write accounting to logfile

  if(l->username.size()==0 || l->action==0) {  // no username or unknown action
    delete l;
//    unlock();
    return;
  }
  // create TIP system
  TipSystem *tipSystem = new TipSystem;
  tipSystem->clear_ut();

  // do portConvert twice for remove spaces and do port form "Async33","Serial4"
  string prt = utl::portConvert(utl::portConvert(l->port));
  if(l->action == 1) { // this is START packet
    // add user to user track session
    if(::userTrack.add(l->username,l->nas,prt,l->from,l->ip)) {
      // add user to bundle if user added
      tipSystem->add_ut(l->username,l->nas,l->ip,prt,l->from,true);
    }
    startlog(LOG_INFORM)<<"START user "<<l->username<<"/"<<l->nas<<"/"<<prt<<"/"<<l->ip<<"/"<<l->from<<endlog;
  } else if(l->action == 3) { // this is STOP packet (delete user from session)
    if(l->ip != UNKNOWN_IP) {
      ::coreData->getPool()->ret(l->ip); // free address in addr pool subsytem
      startlog(LOG_AAA)<<"POOL manager try to free ip: "<<l->ip<<endlog;
    }
    l->servtime = time(0); // currenttime for log entry
    //
    ::coreData->getDb()->add_log(l); // !!! add to database log !!!
    //
    // call to manager to delete user from tracking system
    if(::userTrack.del(l->username,l->nas,prt)) {
      // delete user from bundle
      tipSystem->add_ut(l->username,l->nas,l->ip,prt,l->from,false);
    }
    startlog(LOG_INFORM)<<"STOP user "<<l->username<<"/"<<l->nas<<"/"<<prt<<"/"<<l->ip<<"/"<<l->from<<endlog;
  } else if(l->action == 2) { // UPDATE-packet
    // update-packet - renew user information in billing subsystem
    // so we can use "aaa update periodic" for get mostly recent information
    // and do not use SNMP polling. Or use both.
    if(::userTrack.add(l->username,l->nas,prt,l->from,l->ip)) {
      // add/update user to bundle
      tipSystem->add_ut(l->username,l->nas,l->ip,prt,l->from,true);
    }
    startlog(LOG_INFORM)<<"UPDATE user "<<l->username<<"/"<<l->nas<<"/"<<prt<<"/"<<l->ip<<"/"<<l->from<<endlog;
  }
  tipSystem->send_ut(); // send usertrack packet to bundle
  tipSystem->clear_ut(); // clear packet preparing queue
  delete l;
  delete tipSystem;
//  unlock();
}

///////////////////// that's all ////////////////////////
