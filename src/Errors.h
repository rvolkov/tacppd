// error descriptions
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#ifndef __ERRORS_H__
#define __ERRORS_H__

#include "global.h"

namespace tacpp {

// this is exit type - for use with throw/catch
class TacppErr {}; //

/**
@short CmdErr & VtyErr shelps in error reporting mechanizm for virtual terminal.
The errCmd is the member that exeists in all class @see Command.
*/
class VtyErr:public TacppErr {
 public:
  /**
  commandStatus */
  int execStat;
  /**
  additional comment */
  string addComment;
  /**
   */
  VtyErr(){};
  /**
   */
  VtyErr(int eS,string aC):execStat(eS),addComment(aC){};
  /**
   */
  VtyErr(int eS,const char * aC):execStat(eS),addComment(aC){};
};

/**
*/
class CmdErr:public VtyErr {
 public:
  /**
   */
  int errPos;
  /**
   */
  CmdErr(int eP,int eS,string aC):VtyErr(eS,aC),errPos(eP){};
  /**
   */
  CmdErr(int eP,int eS,const char * aC):VtyErr(eS,aC),errPos(eP){};
  /**
   */
  CmdErr(){};
};

};

#endif //__ERRORS_H__
