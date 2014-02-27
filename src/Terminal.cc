// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: Terminal.cc
// description: terminal access support

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"

using std::ostringstream;

Terminal::Terminal(int fh, struct in_addr addr) {
  vt = new VtyTerm(fh,addr);

  handle = fh;
  snprintf(address,sizeof(address)-1,"%s",inet_ntoa(addr));

  //command
  pM[string("exitCMDid")]=&Terminal::cmdExit;
  pM[string("help")]=&Terminal::cmdHelp;
  pM[string("shusers")]=&Terminal::cmdShowUsr;
  pM[string("shnas")]=&Terminal::cmdShowNas;
  pM[string("deball'")]=&Terminal::cmdDebug;
  pM[string("debtacacs")]=&Terminal::cmdDebug;
  pM[string("debbilling")]=&Terminal::cmdDebug;
  pM[string("debaaa")]=&Terminal::cmdDebug;
  pM[string("debbd")]=&Terminal::cmdDebug;
  pM[string("debsnmp")]=&Terminal::cmdDebug;
  pM[string("debnetflow")]=&Terminal::cmdDebug;
  pM[string("debserver")]=&Terminal::cmdDebug;
  pM[string("debcc")]=&Terminal::cmdDebug;
  pM[string("debpool")]=&Terminal::cmdDebug;
  pM[string("debtrack")]=&Terminal::cmdDebug;
  pM[string("debhttp")]=&Terminal::cmdDebug;
  pM[string("debtip")]=&Terminal::cmdDebug;

  pM[string("debnoall'")]=&Terminal::cmdNoDebug;
  pM[string("debnotacacs")]=&Terminal::cmdNoDebug;
  pM[string("debnobilling")]=&Terminal::cmdNoDebug;
  pM[string("debnoaaa")]=&Terminal::cmdNoDebug;
  pM[string("debnobd")]=&Terminal::cmdNoDebug;
  pM[string("debnosnmp")]=&Terminal::cmdNoDebug;
  pM[string("debnonetflow")]=&Terminal::cmdNoDebug;
  pM[string("debnoserver")]=&Terminal::cmdNoDebug;
  pM[string("debnocc")]=&Terminal::cmdNoDebug;
  pM[string("debnopool")]=&Terminal::cmdNoDebug;
  pM[string("debnotrack")]=&Terminal::cmdNoDebug;
  pM[string("debnohttp")]=&Terminal::cmdNoDebug;
  pM[string("debnotip")]=&Terminal::cmdNoDebug;

  pM[string("dcreate")]=&Terminal::cmdDbCreate;
  pM[string("dadduser")]=&Terminal::cmdDbAdd;
  pM[string("daddadd")]=&Terminal::cmdDbAdd;
  pM[string("daddaccess")]=&Terminal::cmdDbAdd;
  pM[string("daddavp")]=&Terminal::cmdDbAdd;
  pM[string("daddcommand")]=&Terminal::cmdDbAdd;
  pM[string("daddlog")]=&Terminal::cmdDbAdd;
  pM[string("daddtrfint")]=&Terminal::cmdDbAdd;
  pM[string("daddbilling")]=&Terminal::cmdDbAdd;
  pM[string("daddbillres")]=&Terminal::cmdDbAdd;
  pM[string("daddnfuser")]=&Terminal::cmdDbAdd;

  pM[string("ddeluser")]=&Terminal::cmdDbDel;
  pM[string("ddeladd")]=&Terminal::cmdDbDel;
  pM[string("ddelaccess")]=&Terminal::cmdDbDel;
  pM[string("ddelavp")]=&Terminal::cmdDbDel;
  pM[string("ddelcommand")]=&Terminal::cmdDbDel;
  pM[string("ddellog")]=&Terminal::cmdDbDel;
  pM[string("ddeltrfint")]=&Terminal::cmdDbDel;
  pM[string("ddelbilling")]=&Terminal::cmdDbDel;
  pM[string("ddelbillres")]=&Terminal::cmdDbDel;
  pM[string("ddelnfuser")]=&Terminal::cmdDbDel;

  pM[string("dmoduser")]=&Terminal::cmdDbMod;
  pM[string("dmodadd")]=&Terminal::cmdDbMod;
  pM[string("dmodaccess")]=&Terminal::cmdDbMod;
  pM[string("dmodavp")]=&Terminal::cmdDbMod;
  pM[string("dmodcommand")]=&Terminal::cmdDbMod;
  pM[string("dmodlog")]=&Terminal::cmdDbMod;
  pM[string("dmodtrfint")]=&Terminal::cmdDbMod;
  pM[string("dmodbilling")]=&Terminal::cmdDbMod;
  pM[string("dmodbillres")]=&Terminal::cmdDbMod;
  pM[string("dmodnfuser")]=&Terminal::cmdDbMod;

  pM[string("dshowuser")]=&Terminal::cmdDbShow;
  pM[string("dshowadd")]=&Terminal::cmdDbShow;
  pM[string("dshowaccess")]=&Terminal::cmdDbShow;
  pM[string("dshowavp")]=&Terminal::cmdDbShow;
  pM[string("dshowcommand")]=&Terminal::cmdDbShow;
  pM[string("dshowlog")]=&Terminal::cmdDbShow;
  pM[string("dshowtrfint")]=&Terminal::cmdDbShow;
  pM[string("dshowbilling")]=&Terminal::cmdDbShow;
  pM[string("dshowbillres")]=&Terminal::cmdDbShow;
  pM[string("dshownfuser")]=&Terminal::cmdDbShow;

  pM[string("cluser")]=&Terminal::cmdClearUsr;
  pM[string("clport")]=&Terminal::cmdClearNas;

  pM[string("config")]=&Terminal::cmdConfig;
  pM[string("confShow")]=&Terminal::cmdConfig;
  pM[string("confWrite")]=&Terminal::cmdConfig;
  pM[string("confExit")]=&Terminal::cmdConfig;

  pM[string("confAccessNode")]=&Terminal::cmdConfigAccess;
  pM[string("confAccessNo")]=&Terminal::cmdConfigAccess;
  pM[string("confAccessDeny")]=&Terminal::cmdConfigAccess;
  pM[string("confAccessPermit")]=&Terminal::cmdConfigAccess;
  pM[string("confAccessIntNoDeny")]=&Terminal::cmdConfigAccess;
  pM[string("confAccessIntNoPermit")]=&Terminal::cmdConfigAccess;

  pM[string("confBillingNode")]=&Terminal::cmdConfigBilling;
  pM[string("confBillingNo")]=&Terminal::cmdConfigBilling;
  pM[string("confBillingDescr")]=&Terminal::cmdConfigBilling;
  pM[string("confBillingModule")]=&Terminal::cmdConfigBilling;

  pM[string("confBundleNode")]=&Terminal::cmdConfigBundle;
  pM[string("confBundleNo")]=&Terminal::cmdConfigBundle;
  pM[string("confBundleCrypto")]=&Terminal::cmdConfigBundle;
  pM[string("confBundlePrio")]=&Terminal::cmdConfigBundle;
  pM[string("confBundleTTl")]=&Terminal::cmdConfigBundle;

  pM[string("confDbNo")]=&Terminal::cmdConfigDb;
  pM[string("confDbNode")]=&Terminal::cmdConfigDb;
  pM[string("confDbCrypto")]=&Terminal::cmdConfigDb;
  pM[string("confDbLogin")]=&Terminal::cmdConfigDb;
  pM[string("confDbPasswd")]=&Terminal::cmdConfigDb;
  pM[string("confDbModule")]=&Terminal::cmdConfigDb;
  pM[string("confDbShutdown")]=&Terminal::cmdConfigDb;

  pM[string("confDebugNoFile")]=&Terminal::cmdConfigDebug;
  pM[string("confDebugNoIP")]=&Terminal::cmdConfigDebug;
  pM[string("confDebugNodeFile")]=&Terminal::cmdConfigDebug;
  pM[string("confDebugNodeIP")]=&Terminal::cmdConfigDebug;
  pM[string("confDebugF")]=&Terminal::cmdConfigDebug;
  pM[string("confDebugFIntNo")]=&Terminal::cmdConfigDebug;
  pM[string("confDebugIP")]=&Terminal::cmdConfigDebug;
  pM[string("confDebugIPIntNo")]=&Terminal::cmdConfigDebug;

  pM[string("confDeviceNode")]=&Terminal::cmdConfigDevice;
  pM[string("confDeviceNo")]=&Terminal::cmdConfigDevice;
  pM[string("confDeviceMan")]=&Terminal::cmdConfigDevice;
  pM[string("confDeviceDescr")]=&Terminal::cmdConfigDevice;
  pM[string("confDeviceTKey")]=&Terminal::cmdConfigDevice;
  pM[string("confDeviceSnmpComm")]=&Terminal::cmdConfigDevice;
  pM[string("confDeviceLogin")]=&Terminal::cmdConfigDevice;
  pM[string("confDevicePwd")]=&Terminal::cmdConfigDevice;
  pM[string("confDeviceDauth")]=&Terminal::cmdConfigDevice;
  pM[string("confDeviceModule")]=&Terminal::cmdConfigDevice;
  pM[string("confDevicePollDelay")]=&Terminal::cmdConfigDevice;
  pM[string("confDeviceSnmpPolling")]=&Terminal::cmdConfigDevice;
  pM[string("confDeviceIcmpPolling")]=&Terminal::cmdConfigDevice;
  pM[string("confDeviceIntrfCount")]=&Terminal::cmdConfigDevice;
  pM[string("confDeviceShut")]=&Terminal::cmdConfigDevice;

  pM[string("confListenerNo")]=&Terminal::cmdConfigListner;
  pM[string("confListenerNode")]=&Terminal::cmdConfigListner;
  pM[string("confListenerTac")]=&Terminal::cmdConfigListner;
  pM[string("confListenerHttp")]=&Terminal::cmdConfigListner;
  pM[string("confListenerTerm")]=&Terminal::cmdConfigListner;
  pM[string("confListenerNetf")]=&Terminal::cmdConfigListner;
  pM[string("confListenerBundle")]=&Terminal::cmdConfigListner;
  pM[string("confListenerPeer")]=&Terminal::cmdConfigListner;
  pM[string("confListenerTraps")]=&Terminal::cmdConfigListner;
  pM[string("confListenerSnmp")]=&Terminal::cmdConfigListner;
  pM[string("confListenerTci")]=&Terminal::cmdConfigListner;
  pM[string("confListenerRad")]=&Terminal::cmdConfigListner;
  pM[string("confListenerRadacc")]=&Terminal::cmdConfigListner;
  pM[string("confListenerSyslog")]=&Terminal::cmdConfigListner;
  pM[string("confListenerMaxconnect")]=&Terminal::cmdConfigListner;
  pM[string("confListenerAcl")]=&Terminal::cmdConfigListner;

  pM[string("confManagerNo")]=&Terminal::cmdConfigManager;
  pM[string("confManagerNode")]=&Terminal::cmdConfigManager;
  pM[string("confManagerPwd")]=&Terminal::cmdConfigManager;
  pM[string("confManagerCPwd")]=&Terminal::cmdConfigManager;
  pM[string("confManagerAcl")]=&Terminal::cmdConfigManager;
  pM[string("confManagerHttp")]=&Terminal::cmdConfigManager;
  pM[string("confManagerTerm")]=&Terminal::cmdConfigManager;
  pM[string("confManagerTci")]=&Terminal::cmdConfigManager;

  pM[string("confPeerNo")]=&Terminal::cmdConfigPeer;
  pM[string("confPeerNode")]=&Terminal::cmdConfigPeer;
  pM[string("confPeerDescr")]=&Terminal::cmdConfigPeer;
  pM[string("confPeerMask")]=&Terminal::cmdConfigPeer;
  pM[string("confPeerNoMask")]=&Terminal::cmdConfigPeer;
  pM[string("confPeerCrypto")]=&Terminal::cmdConfigPeer;

  pM[string("confPoolNo")]=&Terminal::cmdConfigPool;
  pM[string("confPoolNode")]=&Terminal::cmdConfigPool;
  pM[string("confPoolNoAddr")]=&Terminal::cmdConfigPool;
  pM[string("confPoolAddr")]=&Terminal::cmdConfigPool;

  const int cli_ml_len = 200000;
  char *cli_ml = new char[cli_ml_len];
  bzero(cli_ml,sizeof(char) * cli_ml_len);
  char xmlfile[100];
  char *cli_ml_tmp = cli_ml;
  snprintf(xmlfile,sizeof(xmlfile)-1,"%s/etc/tacppd.cf.xml",::MAINDIR);
  std::ifstream xmlf;
  xmlf.open(xmlfile,std::ios::in);
  if(xmlf.good()) {
    char c;
    while(!xmlf.eof()) {
      xmlf.get(c);
      *cli_ml_tmp = c;
      cli_ml_tmp++;
    }
  }
  xmlf.close();
  cl = new Cli(cli_ml);
  delete [] cli_ml;
}

Terminal::~Terminal() {
  delete vt;
  delete cl;
//std::cerr<<"Terminal::~Terminal destructor"<<std::endl;
}

// terminal user authentication
bool
Terminal::auth() {
  char pass[100];
  char str[100];
  vt->echo(true);
  vt->local_echo = false;
  vt->twrite("\nTACPPD user access verification\n\nUsername: ");
  strncpy(username,vt->readstr(),sizeof(username)-1);
  vt->twrite("Password: ");
  vt->echo(false);
  strncpy(str,vt->readstr(),sizeof(str)-1);
  bzero(pass,sizeof(pass));
  strncpy(pass,str,sizeof(pass));
  vt->local_echo = true;

  vt->vty_cleanup();
//cerr<<"Read username="<<username<<" password="<<pass<<" address="<<address<<endl;
  return ::coreData->getManager()->check(username,pass,address,"terminal");
}

commandStatus
Terminal::makeAction(char *const* mask, const string cmdname,const list<string>paramList, Command * up) throw (CmdErr) {
  commandStatus ret = CMD_NO_ACTION;
//std::cerr<<"trying to execute ="<<cmdname<<std::endl;
  if(pM.find(cmdname) != pM.end())
    ret = (this->*pM[cmdname])(mask,cmdname,paramList,up);
//  else throw CmdErr(0,CMD_UNKNOWN,"Please BUGreport to developer, there is no such function or method named:"+cmdname);
//  cerr<<"ret ="<<ret<<endl;
  if(ret == CMD_EXIT) return CMD_EXIT;
//  cerr<<"return CMD_EXECUTED ="<<CMD_EXECUTED<<endl;
  return CMD_EXECUTED;
}

// process:
void
Terminal::process() {
  vt->tset();
  vt->local_echo = false;

  if(!auth()) {
    vt->twrite("\nAuthentication failed\n\n");
    return;
  }

  vt->twrite("\nAuthentication success\n\n");
  ::logTermSystem->add(vt->get_handle());   // create debug output device
  vt->linemode(false);
  vt->echo(false);
  vt->local_echo = true;
//  process(string(""));

  cl->getRoot()->setCFVty(vt,this);
  while(cl->getRoot()->makeAction() != CMD_EXECUTED);
  ::logTermSystem->del(vt->get_handle());   // delete debug output device
}

// process:
void
Terminal::process(string in) throw(VtyErr,CmdErr) {
/*
  in="\
  config\n\
     debug 10.1.1.2 \n\
      facility aaa\n\
      facility db\n\
    exit\n\
\n\
    access 20\n\
      permit ^127.1.1.1\n\
exit\n\
exit\n\
";
*/
 try {
  VtyStream vs(in);
  cl->getNiRoot()->setCFVty(&vs,this);
  while(cl->getNiRoot()->makeAction() != CMD_EXECUTED);
 }
 catch(VtyErr t){
  cerr<<"Vty err"<<t.addComment<<endl;
 }
 catch(CmdErr c){
  cerr<<"Cmd err"<<c.addComment<<endl;
 }
}

void
Terminal::process(const char * in) throw(VtyErr,CmdErr) {
 try {
  VtyStream vs(in);
  cl->getNiRoot()->setCFVty(&vs,this);
  while(cl->getNiRoot()->makeAction() != CMD_EXECUTED);
 }
 catch(VtyErr t){
  cerr<<"Vty err"<<t.addComment<<endl;
 }
 catch(CmdErr c){
  cerr<<"Cmd err"<<c.addComment<<endl;
 }
}

/////////////////////////////////// Commands of terminal
commandStatus
Terminal::cmdExit(char * const * mask,const string cmdID,const list<string> paramlist,Command * up) {
  return CMD_EXIT;
}

commandStatus
Terminal::cmdHelp(char * const * mask,const string cmdID,const list<string> paramlist,Command * up) {
  vt->twrite("\n\
  ^P\t\tPrevious command in list.\n\
  ^N\t\tNext command in list.\n\n\
  ^F\t\tMove forward one character\n\
  ^B\t\tMove backward one character\n\
  ^A\t\tMove to the begining of the line.\n\
  ^E\t\tMove to the end of the line.\n\n\
  ^H\t\tDelete backward character.\n\
  ^D\t\tDelete character in cursor position.\n\n\
  ^U\t\tClear line from home to cursor position.\n\n\
  ^K\t\tClear line.\n\n\
  <tab>\t\tCompleate command.\n");
//  return CMD_EXIT;
  return CMD_EXECUTED;
}

commandStatus
Terminal::cmdShowUsr(char * const * mask,const string cmdID,const list<string> paramlist,Command *up) {
  char *ret;
  Device_data *dtmp = 0;
  DeviceUser_data utmp;
  ret = new char[30000];
  bzero(ret,sizeof(char)*30000);
  char str[200];
  int k = 0;
  snprintf(str,sizeof(str),"\n Username\t\tPort\tPhone\t\tIP Addr\n");
  k += strlen(str);
  strcat(ret,str);
//cerr<<"Point 1"<<endl;
  for(int i=0; (dtmp = ::coreData->getDevice()->get(i)) != 0; i++) {
//cerr<<"Point 2"<<endl;
    snprintf(str,sizeof(str)," device: %s (%s)\n",
      dtmp->getHostaddr().getn().c_str(),dtmp->getDescr().c_str());
//cerr<<"Point 3"<<endl;
    k += strlen(str);
    if(k >= 30000) break;
    strcat(ret,str);
    int j = 0;
    while(true) {
//cerr<<"Point 4"<<endl;
      utmp = dtmp->gets(j);
//cerr<<"Point 5"<<endl;
      if(!utmp.isPresent()) break;
      snprintf(str,sizeof(str),"%s%s%s\t%s\t%s\n",
        utmp.getUsername().c_str(),utmp.getUsername().size()>6?"\t\t":"\t\t\t",
        utmp.getPort().c_str(),utmp.getPhone().c_str(),utmp.getUserAddr().get().c_str());
      k += strlen(str);
      if(k >= 30000) break;
      strcat(ret,str);
      j++;
    }
  }
//std::cerr<<"cmdShowUsr has "<<i<<" users"<<std::endl;
  vt->pager(ret);
  delete [] ret;
  return CMD_EXECUTED;
}

commandStatus
Terminal::cmdShowNas(char * const * mask,const string cmdID, const list<string> paramlist,Command * up) {
  Device_data *tmp = 0;
  ostringstream oStr;
  oStr<<"\n Nas\t\t\tIP\t\tStatus\n";

  for(int j = 0; (tmp = ::coreData->getDevice()->get(j))!=0; j++)
 oStr<<" "<<  tmp->getDescr()<<"\t\t"<<tmp->getHostaddr().getn()<<"\t\t"<<(tmp->getStatus()?"Up":"Down")<<endl;

  vt->pager(oStr.str().c_str());
  return CMD_EXECUTED;	
}

commandStatus
Terminal::cmdClearUsr(char * const * mask,const string cmdID, const list<string> paramlist,Command * up) {
  //char *ret;
  Device_data *dtmp = 0;
  DeviceUser_data utmp;
  char usrnam[100];
  strncpy(usrnam,paramlist.begin()->c_str(),sizeof(usrnam)-1);

  // search for users and drop all ports with this username
  for(int i=0; (dtmp=::coreData->getDevice()->get(i)) != 0; i++) {
    int j = 0;
    while(true) {
      utmp = dtmp->gets(j);
      if(!utmp.isPresent()) break;
      if(utmp.getUsername() == usrnam) {
        ::snmpSystem.drop((char*)dtmp->getHostaddr().get().c_str(),(char*)utmp.getPort().c_str());
      }
    }
  }
  char sss[100];
  snprintf(sss,sizeof(sss),"User %s have dropped\n",paramlist.begin()->c_str());
  vt->twrite(sss);
  return CMD_EXECUTED;	
}

commandStatus
Terminal::cmdClearNas(char *const*mask,const string cmdID, const list<string> paramlist,Command * up) {
  char nas[100];
  char port[100];
  strncpy(nas,paramlist.begin()->c_str(),sizeof(nas)-1);
  strncpy(port,(++paramlist.begin())->c_str(),sizeof(port)-1);
  //ret = utl::newstr("You should set nas and port\n");
  if(!utl::isIp(nas)) {
    vt->twrite("Nas should be a valid IP address\n");
    return CMD_PARAM_ERROR;
  }
  ::snmpSystem.drop(nas,port);      // drop port
  char sss[100];
  snprintf(sss,sizeof(sss),"Nas %s port %s have dropped\n",nas,port);
  vt->twrite(sss);
  return CMD_EXECUTED;
}

commandStatus
Terminal::cmdDebug(char * const * mask,const string cmdID, const list<string> paramlist,Command * up) {
  for(int r=1; ::coreData->getDebug()->fname(r).size()>0; r++) {
    if(utl::matchNstrict(utl::split(vt->vty_get_line().c_str(),1).c_str(),"ALl") || utl::match(utl::split(vt->vty_get_line().c_str(),1).c_str(),::coreData->getDebug()->fname(r).c_str())) {
      ::logTermSystem->add_debug_fc(vt->get_handle(),::coreData->getDebug()->fnum((char*)::coreData->getDebug()->fname(r).c_str()));
      char sss[100];
      snprintf(sss,sizeof(sss),"\ndebug for %s now is ON",utl::lower(::coreData->getDebug()->fname(r)).c_str());
      vt->twrite(sss);
    }
  }
  return CMD_EXECUTED;
}

commandStatus
Terminal::cmdNoDebug(char * const * mask,const string cmdID, const list<string> paramlist,Command * up) {
  char facility[100];
  strncpy(facility,utl::split(vt->vty_get_line().c_str(),2).c_str(),sizeof(facility)-1);

  for(int r=1; ::coreData->getDebug()->fname(r).size()>0; r++) {
    if(utl::matchNstrict(facility,"ALl") || utl::match(facility,::coreData->getDebug()->fname(r).c_str())) {
      logTermSystem->del_debug_fc(vt->get_handle(),::coreData->getDebug()->fnum((char*)::coreData->getDebug()->fname(r).c_str()));
      char sss[100];
      snprintf(sss,sizeof(sss),"\ndebug for %s now is OFF",utl::lower(::coreData->getDebug()->fname(r)).c_str());
      vt->twrite(sss);
    }
  }
  return CMD_EXECUTED;
}

commandStatus
Terminal::cmdDbCreate(char * const * mask,const string cmdID, const list<string> paramlist,Command * up) {
  vt->pager((char*)::coreData->getDb()->create_all().c_str());    // create tables
  return CMD_EXECUTED;
}

commandStatus
Terminal::cmdDbAdd(char * const * mask,const string cmdID,const list<string> paramlist,Command *up) {
  // get table name
  char name[32];
  //const char *s = vt->vty_get_line().c_str();
  snprintf(name,sizeof(name),"%s",mask[1]);
  if(strlen(name) == 0) {
    vt->twrite("% no table name available");
    return CMD_PARAM_ERROR;
  }
  // check database table name
  bool tpresent = false;
  int tablenum = 0;
  char tablename[100];
  for(int i=0; ::coreData->getDb()->get_table_name(i).size() > 0;i++) {
    if(utl::match(utl::lower(name).c_str(),::coreData->getDb()->get_table_mask(i).c_str())) {
      tpresent = true;
      tablenum = i;  // this is number of matched database table
      break;
    }
  }
  if(!tpresent) {
    vt->twrite("% no such table");
    return CMD_PARAM_ERROR;
  }
  snprintf(tablename,sizeof(tablename),"%s",
    ::coreData->getDb()->get_table_name(tablenum).c_str());

  list<string> tmp;
  list<string> tmp2;

  // run throughout all fields
  for(int i=0; i < ::coreData->getDb()->get_field_num(tablenum); i++) {
    char sss[100];
    char ccc[100];
    if(::coreData->getDb()->get_field_type(tablenum,i) == 5)  // time_t type
      snprintf(sss,sizeof(sss),"%s [%s]:",
        ::coreData->getDb()->get_field_descr(tablenum,i).c_str(),
          utl::time2iso((time_t)atoi(::coreData->getDb()->get_field_defv(tablenum,i).c_str())).c_str());
    else
      snprintf(sss,sizeof(sss),"%s [%s]:",
        ::coreData->getDb()->get_field_descr(tablenum,i).c_str(),
          ::coreData->getDb()->get_field_defv(tablenum,i).c_str());
    vt->twrite("\n");
    vt->twrite(sss);
    snprintf(ccc,sizeof(ccc),"%s",
      ::coreData->getDb()->get_field_name(tablenum,i).c_str()); // store
    snprintf(sss,sizeof(sss),"%s",vt->readstr()); // store
    if(strlen(sss)==0)
      snprintf(sss,sizeof(sss),"%s",
        ::coreData->getDb()->get_field_defv(tablenum,i).c_str());
    if(strlen(sss) > 0) {
      tmp.push_back(sss);
      tmp2.push_back(ccc);
    }
  }
  vt->twrite("\nAre you shure? [y/n]:");
  if(utl::scomp(vt->readstr(),"y")) {
    vt->twrite("\n");
    //str<<
    ::coreData->getDb()->add_data(tablename,tmp2,tmp);
    //<<end();
//No error procession
    for(list<string>::iterator i=tmp.begin();i!=tmp.end() && tmp.size()>0;) i = tmp.erase(i);
    for(list<string>::iterator i=tmp2.begin();i!=tmp2.end() && tmp2.size()>0;) i = tmp2.erase(i);
    return CMD_EXECUTED;
  }
  for(list<string>::iterator i=tmp.begin();i!=tmp.end() && tmp.size()>0;) i = tmp.erase(i);
  for(list<string>::iterator i=tmp2.begin();i!=tmp2.end() && tmp2.size()>0;) i = tmp2.erase(i);
  vt->twrite("% add canceled\n");
  return CMD_EXECUTED;
}

commandStatus
Terminal::cmdDbDel(char * const * mask,const string cmdID, const list<string> paramlist,Command * up) {
  const char *s = vt->vty_get_line().c_str();
  // get table name
  char name[32];
  snprintf(name,sizeof(name),"%s",utl::split(s,1).c_str());
  if(strlen(name) == 0) {
    vt->twrite("% no table name available");
    return CMD_PARAM_ERROR;
  }
  // check database table name
  bool tpresent = false;
  int tablenum = 0;
  char tablename[100];
  for(int i=0; ::coreData->getDb()->get_table_name(i).size() > 0;i++) {
    if(utl::match(name,::coreData->getDb()->get_table_mask(i).c_str())) {
      tpresent = true;
      tablenum = i;     // this is number of matched database table
      break;
    }
  }
  if(!tpresent) {
    vt->twrite("% no such table");
    return CMD_PARAM_ERROR;
  }
  snprintf(tablename,sizeof(tablename),"%s",
    ::coreData->getDb()->get_table_name(tablenum).c_str());
  vt->twrite("\nThere are will be deleted all entries matched\n");

  list<string> tmp;
  list<string> tmp2;

  // run throughout all fields
  for(int i=0; i < ::coreData->getDb()->get_field_num(tablenum); i++) {
    char sss[100];
    char ccc[100];
    snprintf(sss,sizeof(sss),"%s:",
      ::coreData->getDb()->get_field_descr(tablenum,i).c_str());
    vt->twrite("\n");
    vt->twrite(sss);
    snprintf(ccc,sizeof(ccc),"%s",::coreData->getDb()->get_field_name(tablenum,i).c_str());
    snprintf(sss,sizeof(sss),"%s",vt->readstr()); // store
    if(strlen(sss) > 0) {
      tmp.push_back(sss);
      tmp2.push_back(ccc);
    }
  }
  vt->twrite("\nAre you shure? [y/n]:");
  if(utl::scomp(vt->readstr(),"y")) {
    vt->twrite("\n");
    //str<<
    ::coreData->getDb()->del_data(tablename,tmp2,tmp);
    //<<end();
    for(list<string>::iterator i=tmp.begin();i!=tmp.end() && tmp.size()>0;) i = tmp.erase(i);
    for(list<string>::iterator i=tmp2.begin();i!=tmp2.end() && tmp2.size()>0;) i = tmp2.erase(i);
    return CMD_EXECUTED;
    //return str.get();   // ??? how we will see error messages?
  }
  for(list<string>::iterator i=tmp.begin();i!=tmp.end() && tmp.size()>0;) i = tmp.erase(i);
  for(list<string>::iterator i=tmp2.begin();i!=tmp2.end() && tmp2.size()>0;) i = tmp2.erase(i);
  vt->twrite("% delete canceled\n");
  return CMD_EXECUTED;
}

commandStatus
Terminal::cmdDbMod(char * const * mask,const string cmdID, const list<string> paramlist,Command * up) {
  // get table name
  const char *s = vt->vty_get_line().c_str();
  char name[32];
  snprintf(name,sizeof(name),"%s",utl::split(s,1).c_str());
  if(strlen(name) == 0) {
    vt->twrite("% no table name available");
    return CMD_PARAM_ERROR;
  }
  // check database table name
  bool tpresent = false;
  int tablenum = 0;
  char tablename[100];
  for(int i=0; ::coreData->getDb()->get_table_name(i).size() > 0;i++) {
    if(utl::match(name,::coreData->getDb()->get_table_mask(i).c_str())) {
      tpresent = true;
      tablenum = i;       // this is number of matched database table
      break;
    }
  }
  if(!tpresent) {
    vt->twrite("% no such table");
    return CMD_PARAM_ERROR;
  }
  snprintf(tablename,sizeof(tablename),"%s",
    ::coreData->getDb()->get_table_name(tablenum).c_str());

  vt->twrite("\nEntry identification data:");
  list<string> tmp;
  list<string> tmp2;
  // run throughout all fields
  for(int i=0; i < ::coreData->getDb()->get_field_num(tablenum); i++) {
    char sss[100];
    char ccc[100];
    snprintf(sss,sizeof(sss),"%s:",
      ::coreData->getDb()->get_field_descr(tablenum,i).c_str());
    vt->twrite("\n");
    vt->twrite(sss);
    snprintf(sss,sizeof(sss),"%s",
      ::coreData->getDb()->get_field_name(tablenum,i).c_str()); // store
    snprintf(ccc,sizeof(ccc),"%s",vt->readstr()); // store
    if(strlen(ccc) > 0) {
      tmp.push_back(ccc);
      tmp2.push_back(sss);
    }
  }
  list<string> aaa = ::coreData->getDb()->sshow_data(tablename,tmp2,tmp);
  vt->twrite("\nThere are will be modifyed all entries matched");
  list<string> tmp3;
  list<string> tmp4;
  int j=0;
  for(list<string>::iterator i=aaa.begin();i!=aaa.end() && aaa.size()>0;j++) {
    string val = *i;
    char sss[100];
    char ccc[100];
    snprintf(sss,sizeof(sss),"%s [%s]:",
      ::coreData->getDb()->get_field_descr(tablenum,j).c_str(),val.c_str());
    i = aaa.erase(i);
    vt->twrite("\n");
    vt->twrite(sss);
    snprintf(sss,sizeof(sss),"%s",
      ::coreData->getDb()->get_field_name(tablenum,j).c_str()); // store
    snprintf(ccc,sizeof(ccc),"%s",vt->readstr()); // store
    if(strlen(ccc) > 0) {
      tmp3.push_back(ccc);
      tmp4.push_back(sss);
    }
  }
  vt->twrite("\nAre you shure? [y/n]:");
  if(utl::scomp(vt->readstr(),"y")) {
    vt->twrite("\n");
    //str<<
    ::coreData->getDb()->modify_data(tablename,tmp2,
      tmp,tmp4,tmp3);
    //<<end();
    for(list<string>::iterator i=tmp.begin();i!=tmp.end() && tmp.size()>0;) i = tmp.erase(i);
    for(list<string>::iterator i=tmp2.begin();i!=tmp2.end() && tmp2.size()>0;) i = tmp2.erase(i);
    for(list<string>::iterator i=tmp3.begin();i!=tmp3.end() && tmp3.size()>0;) i = tmp3.erase(i);
    for(list<string>::iterator i=tmp4.begin();i!=tmp4.end() && tmp4.size()>0;) i = tmp4.erase(i);
    return CMD_EXECUTED;
    //return str.get();
  }
  for(list<string>::iterator i=tmp.begin();i!=tmp.end() && tmp.size()>0;) i = tmp.erase(i);
  for(list<string>::iterator i=tmp2.begin();i!=tmp2.end() && tmp2.size()>0;) i = tmp2.erase(i);
  for(list<string>::iterator i=tmp3.begin();i!=tmp3.end() && tmp3.size()>0;) i = tmp3.erase(i);
  for(list<string>::iterator i=tmp4.begin();i!=tmp4.end() && tmp4.size()>0;) i = tmp4.erase(i);
  vt->twrite("\n% modify command canceled");
  return CMD_EXECUTED;
}

commandStatus
Terminal::cmdDbShow(char * const * mask,const string cmdID,const list<string> paramlist,Command *up) {
  // get table name
  const char *s = vt->vty_get_line().c_str();	
  char name[32];
  snprintf(name,sizeof(name),"%s",utl::split(s,1).c_str());
  if(strlen(name) == 0) {
    vt->twrite("% no table name available");
    return CMD_PARAM_ERROR;
  }
  // check database table name
  bool tpresent = false;
  int tablenum = 0;
  char tablename[100];
  for(int i=0; ::coreData->getDb()->get_table_name(i).size() > 0;i++) {
    if(utl::match(name,::coreData->getDb()->get_table_mask(i).c_str())) {
      tpresent = true;
      tablenum = i;     // this is number of matched database table
      break;
    }
  }
  if(!tpresent) {
    vt->twrite("% no such table");
    return CMD_PARAM_ERROR;
  }
  snprintf(tablename,sizeof(tablename),"%s",
    ::coreData->getDb()->get_table_name(tablenum).c_str());
  list<string> tmp;          // temporary data
  list<string> tmp2;         // second temporary data
  //int j = 0;
  string vtout;
  vtout = vtout+"\nThere are will be shown all entryes matched (table="+tablename+")";
  //vt->twrite("\nThere are will be shown all entryes matched");
  vt->twrite(vtout.c_str());
//cerr<<"cmdDbShow point 1"<<endl;

//cerr<<"cmdDbShow point 2"<<endl;
  // run throughout all fields
  for(int i=0; i < ::coreData->getDb()->get_field_num(tablenum); i++) {
//cerr<<"cmdDbShow point 3"<<endl;
    char sss[100];
    char ccc[100];
    snprintf(sss,sizeof(sss),"%s (%s):",
      ::coreData->getDb()->get_field_descr(tablenum,i).c_str(),
        ::coreData->getDb()->get_field_name(tablenum,i).c_str());
//cerr<<"cmdDbShow point 4"<<endl;
    vt->twrite("\n");
    vt->twrite(sss);
//cerr<<"cmdDbShow point 5"<<endl;
    snprintf(sss,sizeof(sss),"%s",
      ::coreData->getDb()->get_field_name(tablenum,i).c_str()); // store
//cerr<<"cmdDbShow point 6"<<endl;
    snprintf(ccc,sizeof(ccc),"%s",vt->readstr()); // store
    if(strlen(ccc) > 0) {
//cerr<<"cmdDbShow point 7"<<endl;
      tmp.push_back(ccc);
      tmp2.push_back(sss);
    }
//cerr<<"cmdDbShow point 8"<<endl;
  }
//cerr<<"cmdDbShow point 9"<<endl;
  vt->twrite("\nAre you shure? [y/n]:");
  if(utl::scomp(vt->readstr(),"y")) {
    vt->twrite("\n");
    //str<<
    string sss;
    sss = ::coreData->getDb()->show_data(tablename,tmp2,tmp);
    for(list<string>::iterator i=tmp.begin();i!=tmp.end() && tmp.size()>0;) i = tmp.erase(i);
    for(list<string>::iterator i=tmp2.begin();i!=tmp2.end() && tmp2.size()>0;) i = tmp2.erase(i);
    vt->twrite("\n");
    //vt->twrite(sss.c_str());
    vt->pager(sss.c_str());
    //<<end();
    return CMD_EXECUTED;
  }
  for(list<string>::iterator i=tmp.begin();i!=tmp.end() && tmp.size()>0;) i = tmp.erase(i);
  for(list<string>::iterator i=tmp2.begin();i!=tmp2.end() && tmp2.size()>0;) i = tmp2.erase(i);
  vt->twrite("\n% show canceled");
  return CMD_EXECUTED;
}

commandStatus
Terminal::cmdConfig(char * const * mask,const string cmdID, const list<string> paramlist,Command * up) {
  //commandStatus ret=CMD_UNKNOWN;
  //const char *s=vt->vty_get_line().c_str();	
  string str;
  char spath[256];
  if(utl::matchNstrict(mask[0],"EXit")) {
    return CMD_EXIT;
  }
//std::cerr<<"Terminal::cmdConfig mask[0]="<<mask[0]<<std::endl;
  if(utl::match(mask[0],"SHow")) {
//std::cerr<<"Terminal::cmdConfig buildng configuration"<<std::endl;
    vt->twrite("\nBuilding configuration ... ");
    string a = ::coreData->showcf(this->username);
//std::cerr<<"Terminal::cmdConfig configuration builded"<<std::endl;
    vt->twrite("Ok\n\n");
    if(a.size() > 0) {
      vt->pager((char*)a.c_str());
    }
    return CMD_EXECUTED;
  }
  if(utl::match(mask[0],"WRite")) {
    snprintf(spath,sizeof(spath),"%s/etc/tacppd.conf",::MAINDIR);
    vt->twrite("\nWrite config to file ... ");
    str = str + ::coreData->writecf(spath,this->username);
    vt->twrite("Ok\n");
    return CMD_EXECUTED;
  }
  return CMD_EXECUTED;
}
/*
commandStatus
Terminal::cmdConfigNo(char * const * mask,const string cmdID, const list<string> paramlist,Command * up) {
   Core_data * tmp;
   tmp=::coreData->get(utl::lower(mask[1]).c_str());//no access-list #1
   if(!tmp);//No such chapter, internal error or misconfiguration FIXME should be error reporting!
   if(paramlist.size()==1)tmp->applycf(mask[0],mask[1],paramlist.begin()->c_str(),"","");
   else  tmp->applycf(mask[0],mask[1],mask[2],"","");
  return CMD_EXECUTED;
}

//  string configKeyWord;
//  string configKeyValue;
commandStatus
Terminal::cmdConfigSection(char * const * mask,const string cmdID, const list<string> paramlist,Command * up) {
   configKeyWord=string(utl::lower(mask[0]).c_str());
   if(paramlist.size()==1)configKeyValue=string(paramlist.begin()->c_str());
   else configKeyValue=string(utl::lower(mask[1]).c_str());
  return CMD_EXECUTED;
}

commandStatus
Terminal::cmdConfigOther(char * const * mask,const string cmdID, const list<string> paramlist,Command * up) {
   Core_data * tmp;
   tmp=::coreData->get(configKeyWord.c_str());//no access-list #1

   if(paramlist.size()==1)tmp->applycf("",configKeyWord.c_str(),configKeyValue.c_str(),mask[0],paramlist.begin()->c_str());
   else tmp->applycf("",configKeyWord.c_str(),configKeyValue.c_str(),mask[0],mask[1]);

  return CMD_EXECUTED;
}

commandStatus
Terminal::cmdConfigOtherNo(char * const * mask,const string cmdID, const list<string> paramlist,Command * up) {
   Core_data * tmp;
   tmp=::coreData->get(configKeyWord.c_str());//no access-list #1

   if(paramlist.size()==1)tmp->applycf(mask[0],configKeyWord.c_str(),configKeyValue.c_str(),mask[1],paramlist.begin()->c_str());
   else  tmp->applycf(mask[0],configKeyWord.c_str(),configKeyValue.c_str(),mask[1],mask[2]);

  return CMD_EXECUTED;
}
*/
commandStatus
Terminal::cmdConfigBilling(char * const * mask,const string cmdID, const list<string> paramlist,Command * up) {
  Core_data * tmp;
  tmp = ::coreData->get(BILLING_TYPE);

  string errStr=tmp->applycf(mask,cmdID, paramlist, up);
  if(errStr.size()==0)  return CMD_EXECUTED;
//  cerr << "---->>--->cmdConfigBilling="<<errStr<<endl;
  return CMD_NO_ACTION;
}

commandStatus
Terminal::cmdConfigBundle(char * const * mask,const string cmdID, const list<string> paramlist,Command * up) {
  Core_data * tmp;
  tmp = ::coreData->get(BUNDLE_TYPE);

  string errStr=tmp->applycf(mask,cmdID, paramlist, up);
  if(errStr.size()==0)  return CMD_EXECUTED;
//  cerr << "---->>--->cmdConfigBundle="<<errStr<<endl;
  return CMD_NO_ACTION;
}

commandStatus
Terminal::cmdConfigAccess(char * const * mask,const string cmdID, const list<string> paramlist,Command * up) {
  Core_data * tmp;
  tmp = ::coreData->get(ACCESS_TYPE);//no access-list #1

  string errStr=tmp->applycf(mask,cmdID, paramlist, up);
  if(errStr.size()==0)  return CMD_EXECUTED;
//  cerr << "---->>--->cmdConfigAccess="<<errStr<<endl;
  return CMD_NO_ACTION;
}

commandStatus
Terminal::cmdConfigDb(char * const * mask,const string cmdID, const list<string> paramlist,Command * up) {
  Core_data * tmp;
  tmp = ::coreData->get(DB_TYPE);
  string errStr = tmp->applycf(mask,cmdID, paramlist, up);
  if(errStr.size() == 0)  return CMD_EXECUTED;
//  cerr << "---->>--->cmdConfigDb="<<errStr<<endl;
  return CMD_NO_ACTION;
}

commandStatus
Terminal::cmdConfigDebug(char * const * mask,const string cmdID, const list<string> paramlist,Command *up) {
  Core_data * tmp;
  tmp = ::coreData->get(DEBUG_TYPE);
  string errStr = tmp->applycf(mask,cmdID, paramlist, up);
  if(errStr.size()==0)  return CMD_EXECUTED;
//  cerr << "---->>--->cmdConfigDebug="<<errStr<<endl;
  return CMD_NO_ACTION;
}

commandStatus
Terminal::cmdConfigDevice(char * const * mask,const string cmdID, const list<string> paramlist,Command *up) {
  Core_data * tmp;
  tmp = ::coreData->get(DEVICE_TYPE);

  string errStr = tmp->applycf(mask, cmdID, paramlist, up);
  if(errStr.size() == 0) return CMD_EXECUTED;
//  cerr << "---->>--->cmdConfigDevice="<<errStr<<endl;
  return CMD_NO_ACTION;
}

commandStatus
Terminal::cmdConfigListner(char * const * mask,const string cmdID,const list<string> paramlist,Command *up) {
  Core_data *tmp;
  tmp = ::coreData->get(LISTENER_TYPE);
  string errStr = tmp->applycf(mask, cmdID, paramlist, up);
  if(errStr.size()==0) return CMD_EXECUTED;
//  cerr << "---->>--->cmdConfigListner="<<errStr<<endl;
  return CMD_NO_ACTION;
}

commandStatus
Terminal::cmdConfigManager(char * const * mask,const string cmdID, const list<string> paramlist,Command *up) {
  Core_data * tmp;
  tmp = ::coreData->get(MANAGER_TYPE);

  string errStr=tmp->applycf(mask,cmdID, paramlist, up);
  if(errStr.size()==0)  return CMD_EXECUTED;
//  cerr << "---->>--->cmdConfigManager="<<errStr<<endl;
  return CMD_NO_ACTION;
}

commandStatus
Terminal::cmdConfigPeer(char * const * mask,const string cmdID, const list<string> paramlist,Command *up) {
  Core_data * tmp;
  tmp = ::coreData->get(PEER_TYPE);

  string errStr=tmp->applycf(mask,cmdID, paramlist, up);
  if(errStr.size()==0)  return CMD_EXECUTED;
//  cerr << "---->>--->cmdConfigPeer="<<errStr<<endl;
  return CMD_NO_ACTION;
}

commandStatus
Terminal::cmdConfigPool(char * const * mask,const string cmdID, const list<string> paramlist,Command *up) {
  Core_data * tmp;
  tmp = ::coreData->get(POOL_TYPE);

  string errStr=tmp->applycf(mask,cmdID, paramlist, up);
  if(errStr.size()==0)  return CMD_EXECUTED;
//  cerr << "---->>--->cmdConfigPool="<<errStr<<endl;
  return CMD_NO_ACTION;
}

///////////// this is all //////////////
