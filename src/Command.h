// tacppd CLI commands processing
// (c) Copyright 2000-2007 by tacppd team and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// all comments which start from /**<CR> - this is comments for KDoc */
// for classes
//   @short [short description of the class]
//   @author [class author]
//   @version [class version]
//   @see [references to other classes or methods]
// for methods
//   @see [references]
//   @return [sentence describing the return value]
//   @exception [list the exeptions that could be thrown]
//   @param [name id] [description]	- can be multiple

#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "global.h"

namespace tacpp {

enum commandStatus {
  CMD_NO_HELP_AVAIL=0,
  CMD_COMMAND_HELP_AVAIL,
  CMD_PARAM_HELP_AVAIL,

  CMD_AMBIGUOUS,
  CMD_UNKNOWN,

  CMD_PARAM_ERROR,
  CMD_PARAM_COUNT_ERROR,

  CMD_PARSE_SUCCESS,

  CMD_COMPLETED,
  CMD_COMPLETED_BY_CMD_PART,
  CMD_NOT_COMPLETED,
  CMD_NOT_COMPLETED_PARAM_ENCOUNTERED,

  CMD_EXECUTED,
  CMD_MAKEACTION_FAILED,
  CMD_FAILED,
  CMD_NO_ACTION,
  CMD_EXIT,

  CMD_END_OF_STREAM,
  CMD_ERR_OPEN_FSTREAM,
  CMD_TIMEOUT
};
class Terminal;
class Command;
class Vty;

/**
*/
class cmdComplete {
 public:
  /**
   */
  bool couldComplete;
  /**
   */
  int paramNum;
  /**
   */
  int paramVtyLen;	
  /**
   */
  char * cmd;
};

/**
*/
class commandFacility {
 public:
  /**
   */
  virtual commandStatus makeAction(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd=NULL)throw (CmdErr)=0 ;
  /**
   */
  virtual ~commandFacility() {};
};

/**
@short class Command a primitive of command that represent's single line command for example:
show line status*/
class Command {
 private:
 protected:
  /**
  masks of command such as SHow */
  char **command;
  /**
   */
  char **help;
  /**
   */
  string cmdID;
  /**
  @param depthH expected to be +1 of @param depthC*/
  int depthH;
  /**
   */
  int depthC;
  /**
   */
  CmdErr errCmd;
  /**
   */
  cmdComplete completeStat;
  /**
   */
  Vty *vt;
  /**
   */
  commandFacility *cf;

 public:
  /**
  */
  Command *upNode;
  /**
  */
  list<string> paramList;
  /**
  */
  Command(char **cmd,char **hlp,char * cmdID,Command *up=NULL); //{{"SHow"},{"EXxit"},{"STAtus"},{'\0'}};
  /**
  */
  virtual ~Command();
  /**
  */
  CmdErr getCmdErr();
  /**
  */
  cmdComplete getCmdComplete();
  /**
  */
  const char *getCommand(int cmdNum);
  /**
  */  
  const char *getCommandHlp(int cmdNum);
  /**
  */
  int getCommandDepth();
  /**
  */
  virtual void setCFVty(Vty*,commandFacility*);
  /**
  */
  virtual commandStatus makeHelp();
  /**
  */
  virtual commandStatus completeCommand();
  /**
  */
  virtual commandStatus makeAction();
  /**
  */
  virtual commandStatus parseCommand();
  /**
  */
  virtual commandStatus executeCommand();
};

const int maxHistSize=10;
/**
@param maxInvStr _Full_ invite str length, not only in one CommandNode but lenght invStr from rootNode
*/
const int maxInvStr=256;

/**
*/
class NonInteractiveCommandNode: public Command {
 protected:
  /**
   */
  list<Command*>  cmdList;
  /**
   */
  list<CmdErr> execErrors;

 public:
  /**
   */
  NonInteractiveCommandNode(char **cmd,char **hlp,char*cmdi,char **cmdExit,char **hlpExit,char*ecmdi,Command *up=NULL);
  /**
   */
  ~NonInteractiveCommandNode();
  /**
   */
  NonInteractiveCommandNode *insertCommand(Command* cmd);
  /**
   */
  virtual void setCFVty(Vty*,commandFacility *);
  /**
   */
  virtual commandStatus makeAction() throw(CmdErr,VtyErr);
  /**
   */
  virtual void errReport(commandStatus) throw(CmdErr,VtyErr);
};

/**
*/
class CommandNode: public Command {
 protected:
  /**
   */
  char inviteStr[maxInvStr+1];
  /**
   */
  char inviteStrM[maxInvStr+1];
  /**
   */
  list<Command*> cmdList;
  /**
   */
  list<string> histList;
  /**
   */
  list<string>::iterator histIt;
  /**
   */
  list<CmdErr> execErrors;
  /**
   */
  list<cmdComplete> completeStatus;
  /**
   */
  VtyTerm *vtt;

 public:
  /**
   */
  CommandNode(char **cmd,char **hlp,char*cmdi,char **cmdExit,char **hlpExit,char*ecmdi,char *invStr,char *invStrM,Command *up=NULL);
  /**
   */
  ~CommandNode();
  /**
   */
  char *invStr();
  /**
   */
  CommandNode *insertCommand(Command *cmd);
  /**
   */
  void insertHistory();
  /**
   */
  void getHistory(bool next);
  virtual void setCFVty(Vty*,commandFacility *);
  /**
   */
  virtual commandStatus makeHelpNode();
  /**
   */
  virtual commandStatus makeHelp();
  /**
   */
  virtual commandStatus completeNodeCommand();
  /**
   */
  virtual commandStatus makeAction();
  /**
   */
  virtual void errReport(commandStatus);
};

/**
@short All dirty work for split terminal line into command part's is in class VtyLineSplitter.
It used in @see Command for help, execute and complete command request processing.
While processing used spaces as delimetr character's single quotas as   quota's ;-) char and single \ as escape char.
*/
class VtyLineSplitter {
 private:
  /**
   */
  class mypair {
   public:
    /**
     */
    int first;
    /**
     */
    int second;
    /**
     */
    mypair(int fi,int se){first=fi;second=se;};
    /**
     */
    mypair(){first=0;second=0;};
  };
  /**
   */
  Vty *vt;
  /**
   */
  vector<mypair> splitLine;
  /**
   */
  mypair comment; //comment could be '!','#',';' if comment.first=true there is comment inside, second is start position
  /**
  @param cmdUnderCursor.first=true if cursor belong to command ant it near after or inside cmdParam*/
  mypair  cmdUnderCursor;
  /**
  @see split() is called when bool isChanged from @see Vty is changed. */
  void split();

 public:
  /**
   */
  VtyLineSplitter(Vty *vtt);
  /**
  @see getCmdPartPos return start and len but this len do include escape characters because I  deal only with
  positions in vty string and not with real command part's */
  bool getCmdPartPos(int cmdPartnum, int &start,int & len);
  /**
   */
  bool getCmdUnderCursorNum(int& cmdPartnum);
  /**
   */
  bool getCommentPos(int &start,int  &len);
  /**
   */
  int getCmdPartCount();
};

};

#endif //__COMMAND_H__
