// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: Terminal.h
// description:

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

// all comments which start from /** - this is comments for KDoc */
//  for classes
//    @short [short description of the class]
//    @author [class author]
//    @version [class version]
//    @see [references to other classes or methods]
//  for methods
//    @see [references]
//    @return [sentence describing the return value]
//    @exception [list the exeptions that could be thrown]
//    @param [name id] [description]	- can be multiple

#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include "global.h"

namespace tacpp {

#define TERMINAL_TIMEOUT	10000
class Command;
class Vty;
class Cli;

/**
@short Terminal
@author lsk
@version 1
*/
class Terminal : public commandFacility {
  typedef commandStatus (Terminal::*MethodPointer)(char *const* mask,const string,const list<string> ,Command * up);

 private:
  /**
   */
  int handle;
  /**
   */
  struct in_addr in_address;

 protected:
  /**
   */
  map<string,MethodPointer,less<string> > pM;
//  string configKeyWord;
//  string configKeyValue;
  /**
   */
  Cli *cl;

 public:
  /**
   */
  char username[100];
  /**
   */
  char address[100];
  /**
   */
  VtyTerm *vt;
  /**
   */
  Terminal(int,struct in_addr);
  /**
  destructor */
  ~Terminal();
  /**
   */
  void process();
  /**
   */
  void process(string in) throw(VtyErr,CmdErr);
  /**
  process file with commands */
  void process(const char *) throw(VtyErr,CmdErr);
  /**
   */
  commandStatus makeAction(char *const* mask,const string cmdname,const list<string>paramList,Command * up)throw (CmdErr);
  /**
  terminal user authentication */
  bool auth();
  /**
   */
  commandStatus cmdExit(char *const* mask,const string cmdname, const list< string > paramlist,Command * up);
  /**
   */
  commandStatus cmdHelp(char *const* mask,const string cmdname,const list<string>paramList,Command * up);
  /**
   */
  commandStatus cmdShowUsr(char *const* mask,const string cmdID, const list <string> paramlist,Command * up);
  /**
   */
  commandStatus cmdShowNas(char *const* mask,const string cmdID, const list <string> paramlist,Command * up);
  /**
   */
  commandStatus cmdClearUsr(char *const* mask,const string cmdID, const list <string> paramlist,Command * up);
  /**
   */
  commandStatus cmdClearNas(char *const* mask,const string cmdID, const list <string> paramlist,Command * up);
  /**
   */
  commandStatus cmdDbCreate(char *const* mask,const string cmdID, const list <string> paramlist,Command * up);
  /**
   */
  commandStatus cmdDbAdd(char *const* mask,const string cmdID, const list <string> paramlist,Command * up);
  /**
   */
  commandStatus cmdDbDel(char *const* mask,const string cmdID, const list <string> paramlist,Command * up);
  /**
   */
  commandStatus cmdDbMod(char *const* mask,const string cmdID, const list <string> paramlist,Command * up);
  /**
   */
  commandStatus cmdDbShow(char *const* mask,const string cmdID, const list <string> paramlist,Command * up);
  /**
   */
  commandStatus cmdNoDebug(char *const* mask,const string cmdID, const list <string> paramlist,Command * up);
  /**
   */
  commandStatus cmdDebug(char *const* mask,const string cmdID, const list <string> paramlist,Command * up);
  /**
   */
  commandStatus cmdConfig(char *const* mask,const string cmdID, const list <string> paramlist,Command * up);
//  commandStatus cmdConfigNo(char * const * mask,const string cmdID, const list <string> paramlist,Command * up);
//  commandStatus cmdConfigOther(char * const * mask,const string cmdID, const list <string> paramlist,Command * up);
//  commandStatus cmdConfigOtherNo(char * const * mask,const string cmdID, const list <string> paramlist,Command * up);
//  commandStatus cmdConfigSection(char * const * mask,const string cmdID, const list <string> paramlist,Command * up);
  /**
   */
  commandStatus cmdConfigAccess(char * const * mask,const string cmdID, const list<string> paramlist,Command * up);
  /**
   */
  commandStatus cmdConfigBilling(char * const * mask,const string cmdID, const list<string> paramlist,Command * up);
  /**
   */
  commandStatus cmdConfigBundle(char * const * mask,const string cmdID, const list<string> paramlist,Command * up);
  /**
   */
  commandStatus cmdConfigDb(char * const * mask,const string cmdID, const list<string> paramlist,Command * up);
  /**
   */
  commandStatus cmdConfigDebug(char * const * mask,const string cmdID, const list<string> paramlist,Command * up);
  /**
   */
  commandStatus cmdConfigDevice(char * const * mask,const string cmdID, const list<string> paramlist,Command * up);
  /**
   */
  commandStatus cmdConfigListner(char * const * mask,const string cmdID, const list<string> paramlist,Command * up);
  /**
   */
  commandStatus cmdConfigManager(char * const * mask,const string cmdID, const list<string> paramlist,Command * up);
  /**
   */
  commandStatus cmdConfigPeer(char * const * mask,const string cmdID, const list<string> paramlist,Command * up);
  /**
   */
  commandStatus cmdConfigPool(char * const * mask,const string cmdID, const list<string> paramlist,Command * up);
};

};

#endif //__TERMINAL_H__
