// tacppd cli commands handler
// (c) Copyright 2000-2007 by tacppd team and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

#include "global.h"
#include <string>
#include <algorithm>
#include <map>
#include <set>

using std::set;
using std::pair;

bool
compCmdErr(CmdErr x,CmdErr y) {
  return x.errPos < y.errPos;
}

Command::Command(char **cmd,char **hlp,char *cmdi,Command *up) {
  upNode = up;
  cmdID = string(cmdi);
  depthH = 0;
  depthC = 0;
  if(hlp != 0) while(hlp[depthH] != '\0') depthH++;
  if(cmd != 0) while(cmd[depthC] != '\0') depthC++;

  help = new char_ptr[depthH];
  for(int ii=0;ii < depthH;ii++) {
    help[ii] = new char[strlen(hlp[ii])+1];
    strcpy(help[ii],hlp[ii]);
  }
  command = new char_ptr[depthC];
  for(int ii=0;ii < depthC;ii++) {
    command[ii] = new char[strlen(cmd[ii])+1];
    strcpy(command[ii],cmd[ii]);
  }
}

Command::~Command() {
  for(int ii=0;ii < depthH;ii++) {
    delete [] help[ii];
  }
  for(int ii=0;ii < depthC;ii++) {
    delete [] command[ii];
  }
  delete [] help;
  delete [] command;
}

void
Command::setCFVty(Vty *vtt,commandFacility *cff) {
  vt = vtt;
  cf = cff;
}

CmdErr
Command::getCmdErr() {
  return errCmd;
}

cmdComplete
Command::getCmdComplete() {
  return completeStat;
}

const char *
Command::getCommand(int cmdNum) {
  return command[cmdNum];
}

const char *
Command::getCommandHlp(int cmdNum) {
  return help[cmdNum];
}

int
Command::getCommandDepth() {
  return depthC;
}

commandStatus
Command::executeCommand() {
//  vt->twrite("Command::executeCommand(Vty*vt)");
  return cf->makeAction(command,cmdID,paramList,this);
}

commandStatus
Command::makeAction() {
  if(paramList.size() > 0) paramList.erase(paramList.begin(),paramList.end());
  commandStatus stat = parseCommand();
  if(stat == CMD_PARSE_SUCCESS) {
    int pi = -1;
    if(paramList.size() > 0) {
      for(list<string>::iterator ii=paramList.begin();ii != paramList.end();++ii) {
        while(getCommand(++pi)[0] != '{');
        if(!utl::isCorrespondW(getCommand(pi),ii->c_str())) {
          int param_next_pos = 0, len = 0;
          vt->vtSplit->getCmdPartPos(pi, param_next_pos, len);
          errCmd.errPos = param_next_pos + len;
          errCmd.execStat = CMD_PARAM_ERROR;
          return CMD_PARAM_ERROR;
        }
        //vt->twrite("param =");
        //vt->twrite(ii->c_str());
        //vt->twrite("\n");
//std::cerr<<"param="<<ii->c_str()<<std::endl;
      }
    }
    stat = executeCommand();
  }
  return stat;
}

commandStatus
Command::parseCommand() {
  int param_next_pos=0,len=0;
  int paramcount = 0;
  //const char *vt_line = vt->vty_get_line().c_str(); // get string from config or from manual enter
  char vt_line[300];
  snprintf(vt_line,sizeof(vt_line)-1,"%s",vt->vty_get_line().c_str());

  if(vt->vtSplit->getCmdPartCount() == 0) { // if no parameters entered
    errCmd.errPos = param_next_pos;
    errCmd.execStat = CMD_NO_ACTION;
    return CMD_NO_ACTION;
  }
  for(paramcount=0;paramcount < vt->vtSplit->getCmdPartCount() && paramcount < depthC;paramcount++) {
    vt->vtSplit->getCmdPartPos(paramcount, param_next_pos, len);
    //special sign '<' in command parametr
    //mask {config},{line},{<1-10>}
    if(command[paramcount][0] == '{') {
      string s(vt_line+param_next_pos,len);
      paramList.insert(paramList.end(),s);
      continue;
    }
    if(!utl::matchNstrict(vt_line+param_next_pos,command[paramcount])) {
      if(paramcount == 0) {
        errCmd.errPos = param_next_pos;
        errCmd.execStat = CMD_UNKNOWN;
        return CMD_UNKNOWN;
      }
      errCmd.errPos = param_next_pos;
      errCmd.execStat = CMD_PARAM_ERROR;
      return CMD_PARAM_ERROR;
    }
  }
/*  if(vt->vtSplit->getCmdPartCount()>depthC) {
    vt->vtSplit->getCmdPartPos(depthC,  param_next_pos, len);
    errCmd.errPos=param_next_pos;
    errCmd.execStat=CMD_PARAM_COUNT_ERROR;
    return CMD_PARAM_COUNT_ERROR;
  }*/

  if(vt->vtSplit->getCmdPartCount() != depthC) {
    vt->vtSplit->getCmdPartPos(paramcount, param_next_pos, len);
    errCmd.errPos = param_next_pos+len;
    errCmd.execStat = CMD_PARAM_COUNT_ERROR;
    return CMD_PARAM_COUNT_ERROR;
  }

  return CMD_PARSE_SUCCESS;
}

commandStatus
Command::makeHelp() {
  int param_next_pos,len;
  int paramcount;

  int paramUnderCursor;
  //const char *vt_line = vt->vty_get_line().c_str();
  char vt_line[300];
  snprintf(vt_line,sizeof(vt_line)-1,"%s",vt->vty_get_line().c_str());

  vt->vtSplit->getCmdUnderCursorNum(paramUnderCursor);

  if(vt->vtSplit->getCmdPartCount()>depthC) return CMD_PARAM_COUNT_ERROR;

  for(paramcount=0;paramcount<vt->vtSplit->getCmdPartCount();paramcount++) {
    vt->vtSplit->getCmdPartPos(paramcount, param_next_pos, len);
    //special sign '<' in command parametr
    //mask {config},{line},{<1-10>}
    if(command[paramcount][0] == '{') continue;

    if(!utl::matchNstrict(vt_line+param_next_pos, command[paramcount])) {
      if(paramcount == paramUnderCursor) {
        if(strncasecmp(vt_line+param_next_pos,command[paramcount],strlen(vt_line+param_next_pos))==0) {
//          vt->twrite(help[paramcount]);
          return CMD_PARAM_HELP_AVAIL;
        }
      }
      return CMD_NO_HELP_AVAIL;
    }
  }
  vt->vtSplit->getCmdUnderCursorNum(paramcount);

  if(paramcount<depthH) {
//    vt->twrite(help[paramcount]);
    return CMD_PARAM_HELP_AVAIL;
  }
  return CMD_NO_HELP_AVAIL;
}

commandStatus
Command::completeCommand() {
  int param_next_pos = 0, len = 0;
  int paramcount = 0;
//  const char *vt_line = vt->vty_get_line().c_str();
  char vt_line[300];
  snprintf(vt_line,sizeof(vt_line)-1,"%s",vt->vty_get_line().c_str());

  int paramUnderCursor;
  vt->vtSplit->getCmdUnderCursorNum(paramUnderCursor);

  if(vt->vtSplit->getCmdPartCount()>depthC) {
    completeStat.couldComplete=false;
    completeStat.paramNum=paramcount;
    return CMD_NOT_COMPLETED;	
  }
  if(paramUnderCursor >= depthC) return CMD_NOT_COMPLETED;

  if(vt->vtSplit->getCmdPartCount() == 0) {
    completeStat.couldComplete = true;
    completeStat.paramNum = 0;
    completeStat.paramVtyLen = 0;
    completeStat.cmd = command[0];
    return CMD_COMPLETED;
  }

  for(paramcount=0;paramcount<vt->vtSplit->getCmdPartCount();paramcount++) {
    vt->vtSplit->getCmdPartPos(paramcount, param_next_pos, len);
    completeStat.paramVtyLen = 0;
    //special sign '<{ in command parametr
    //mask {config},{line},{int:1-10}
    if(command[paramcount][0] == '{') continue;

    if(!utl::matchNstrict(vt_line+param_next_pos,command[paramcount])) {
      if(paramcount == paramUnderCursor) {
         if(strncasecmp(vt_line+param_next_pos, command[paramcount], strlen(vt_line+param_next_pos)) == 0) {
          completeStat.couldComplete = true;
          completeStat.paramNum = paramcount;
          completeStat.paramVtyLen = len;
          completeStat.cmd = command[paramcount];
          return  CMD_COMPLETED_BY_CMD_PART;
        }
      }
      completeStat.couldComplete = false;
      completeStat.paramNum = paramcount;
      return CMD_NOT_COMPLETED;
    }
  }

  if(vt->vtSplit->getCmdPartPos(paramUnderCursor, param_next_pos, len)) { // find  is paramUnderCursor belong to command
    if(command[paramUnderCursor][0] == '{') {//this is when we try to complete the parameter but we don't know how
      completeStat.paramVtyLen = len;
      completeStat.couldComplete = false;
      completeStat.paramNum = paramUnderCursor;
      completeStat.cmd = command[paramUnderCursor];
      return CMD_NOT_COMPLETED;
    }
    completeStat.paramVtyLen = len;
    completeStat.couldComplete = true;
    completeStat.paramNum = paramUnderCursor;
    completeStat.cmd = command[paramUnderCursor];
    return CMD_COMPLETED;
  } else {
    completeStat.paramVtyLen = 0;
    if(vt->vtSplit->getCmdPartCount() < depthC) { //this play when next command part to complete is parameter
      vt->vtSplit->getCmdPartPos(vt->vtSplit->getCmdPartCount(), param_next_pos, len);
      if(command[vt->vtSplit->getCmdPartCount()][0] == '{') {
        completeStat.couldComplete = false;
        completeStat.paramNum = vt->vtSplit->getCmdPartCount();
        completeStat.cmd = command[vt->vtSplit->getCmdPartCount()];
        return CMD_NOT_COMPLETED_PARAM_ENCOUNTERED;
      }
    }
    completeStat.couldComplete = true;
    completeStat.paramNum = paramUnderCursor;
    completeStat.cmd = command[paramUnderCursor];
    return CMD_COMPLETED;
  }
}

//////////////////////////////NonInteractiveCommandNode///////////////
NonInteractiveCommandNode::NonInteractiveCommandNode(char **cmd,char **hlp,char*cmdi,char **cmdExit,char **hlpExit,char*ecmdi,Command *up):Command(cmd,hlp,cmdi,up) {
  insertCommand(new Command(cmdExit,hlpExit,ecmdi));
}

NonInteractiveCommandNode::~NonInteractiveCommandNode() {
  for(list <Command*>::iterator ii=cmdList.begin();ii!=cmdList.end();++ii) {
    delete (*ii);
  }
}

NonInteractiveCommandNode *
NonInteractiveCommandNode::insertCommand(Command* cmd) {
  cmdList.insert(cmdList.end(),cmd);
  return this;
}

void
NonInteractiveCommandNode::setCFVty(Vty *vtt,commandFacility *cff) {
  vt = vtt;
  Command::vt = vtt;
  cf = cff;
  for(list <Command*>::iterator ii=cmdList.begin();ii != cmdList.end();++ii) {
    (*ii)->setCFVty(vtt,cff);
  }
}

commandStatus
NonInteractiveCommandNode::makeAction() throw(CmdErr,VtyErr) {
  commandStatus ret = CMD_UNKNOWN;
  list<Command*>::iterator ii;

  //const char *vt_line = vt->vty_get_line().c_str();
  ret = this->Command::makeAction();

  if(ret!=CMD_EXECUTED && ret!=CMD_NO_ACTION && upNode!=NULL) return ret;
  vt->vty_cleanup();

  int cStatus;
  string s;

  while(ret != CMD_EXIT) {
    try {
      cStatus = vt->treadstr();
    }
    catch(VtyErr t) {
      if(upNode != NULL) throw CmdErr(0,t.execStat,t.addComment + string(" May be no exit section in config stream/file or incomlete configuration."));
      errReport(CMD_EXECUTED);
      //OK I know, this section is bad and tricky, I know it, but we start
      //process with some RootNonInteractiveNode and want to exit from it without exception
      //and get by this, that we need one more unballanced (from the view of config file)
      // "exit" command, so by this code we FIX this.
      return CMD_EXECUTED;
    }

    switch(cStatus) {
      case T_KEY_EXECUTE_COMMAND:
      for(ii = cmdList.begin();ii != cmdList.end();++ii) {	
        ret = (*ii)->makeAction();
        if(ret == CMD_EXIT) {
	  errReport(CMD_EXECUTED);
	  return CMD_EXECUTED;
	}
        if(ret == CMD_EXECUTED || ret == CMD_NO_ACTION) break;
        else execErrors.insert(execErrors.end(),(*ii)->getCmdErr());
      }
      errReport(ret);
      vt->vty_cleanup();
      break;
     default:
      throw CmdErr(0,ret,"Unknown command status! No reaction for this event.");
      break;
    }
  }
  return CMD_EXECUTED;
}

void
NonInteractiveCommandNode::errReport(commandStatus ret) throw(CmdErr,VtyErr) {
  list<CmdErr>::iterator err;

  if(ret == CMD_EXECUTED || ret == CMD_NO_ACTION) {
    execErrors.erase(execErrors.begin(),execErrors.end());
    return;
  }

  err = max_element(execErrors.begin(),execErrors.end(),compCmdErr);
  CmdErr cme;
  cme.execStat = err->execStat;
  cme.errPos = err->errPos;

  char pos[10];
  snprintf(pos,9,"%d",err->errPos);
  switch(err->execStat) {
   case CMD_UNKNOWN:
    cme.addComment.append("% Command at row ");
    cme.addComment.append(pos);
    cme.addComment.append(" is unknown:");
    break;
   case CMD_PARAM_ERROR:
    cme.addComment.append("\n% Command parametr at row ");
    cme.addComment.append(pos);
    cme.addComment.append(" is unknown or incorrect:");
    break;
   case CMD_PARAM_COUNT_ERROR:
    cme.addComment.append("\n% Parameter count error at row ");
    cme.addComment.append(pos);
    cme.addComment.append(" is unknown:");
    break;
   default:
    cme.addComment.append("\n% Unknown error at row ");
    cme.addComment.append(pos);
    cme.addComment.append(" is unknown:");
    break;
  }
  cme.addComment += err->addComment;
  cme.addComment.append("cmd was<");
  cme.addComment.append(vt->vty_get_line().c_str());
  cme.addComment.append(">");

  execErrors.erase(execErrors.begin(),execErrors.end());
  throw(cme);
  return;
}

////////////////////////////// Command Node ///////////////////////////

CommandNode::CommandNode(char **cmd,char **hlp,char *cmdi,char **cmdExit,char **hlpExit,char *ecmdi,char *invStr,char *invStrM,Command *up):Command(cmd,hlp,cmdi,up) {
  insertCommand(new Command(cmdExit,hlpExit,ecmdi));
  strncpy(inviteStr,invStr,maxInvStr);
  strncpy(inviteStrM,invStrM,maxInvStr);
  histIt = --histList.end();
}

CommandNode::~CommandNode() {
  for(list <Command*>::iterator ii=cmdList.begin();ii!=cmdList.end();++ii) {
    delete (*ii);
  }
}

void
CommandNode::setCFVty(Vty *vt_t,commandFacility *cff) {
  vtt = (VtyTerm *)vt_t;
  vt = vt_t;
  cf = cff;
  for(list <Command*>::iterator ii=cmdList.begin();ii!=cmdList.end();++ii) {
    (*ii)->setCFVty(vt_t,cff);
  }
}

char*
CommandNode::invStr() {
  static char inv[2*maxInvStr+1]={">"};
  static int dep=0;
  if(upNode != NULL) {
    inv[0]='\0';
    dep++;
    ((CommandNode*)upNode)->invStr();
    strncat(inv,inviteStr,maxInvStr);
    dep--;
    if(dep==0)strncat(inv,inviteStrM,maxInvStr);
  } else {
    if(inv[0]!='\0')snprintf(inv,maxInvStr,"\n%s%s",inviteStr,inviteStrM);
    else snprintf(inv,maxInvStr,"\n%s",inviteStr);
  }
  return inv;
}

CommandNode*
CommandNode::insertCommand(Command *cmd) {
  cmdList.insert(cmdList.end(),cmd);
  return this;
}

//Most simple cases of CommandNode own command without patametr's
//such as #config terminal
//for entering in (config)# mode
//and not #interface fastEtherne 0/1
commandStatus
CommandNode::makeAction() {
  commandStatus ret = CMD_UNKNOWN;
  list<Command*>::iterator ii;
  //const char *vt_line = vt->vty_get_line().c_str();

  ret = this->Command::makeAction();

  if(ret!=CMD_EXECUTED && ret!=CMD_NO_ACTION && upNode !=NULL) return ret;

  vtt->twrite(invStr());
  vtt->vty_cleanup();

  int cStatus;
  string s;

  while(ret != CMD_EXIT) {
    cStatus = vt->treadstr();

    switch(cStatus) {
     case T_KEY_GET_HELP:
      vtt->twrite("\n");
      makeHelpNode();
      vtt->twrite(invStr());
      vtt->vty_redraw_line();
      break;
     case T_KEY_COMPLETE_COMMAND:
      completeNodeCommand();
      break;
     case T_KEY_EXECUTE_COMMAND:
      insertHistory();
      for(ii=cmdList.begin();ii != cmdList.end();++ii) {	
        ret=(*ii)->makeAction();
        if(ret == CMD_EXIT) {
          errReport(CMD_EXECUTED);
          return CMD_EXECUTED;
        }
        if(ret==CMD_EXECUTED || ret==CMD_NO_ACTION) break;
        else execErrors.insert(execErrors.end(),(*ii)->getCmdErr());
      }
      errReport(ret);
      vtt->twrite(invStr());
      vtt->vty_cleanup();
      break;

     case T_KEY_NEXT_COMMAND:
      insertHistory();
      getHistory(true);
      break;
     case T_KEY_PREVIOUS_COMMAND:
      insertHistory();
      getHistory(false);
      break;
     default:
      vtt->twriteErr("Unknown command status!\n No reaction for this event.\n");
      break;
    }
  }
  return CMD_EXECUTED;
}

commandStatus
CommandNode::makeHelp() {
  return Command::makeHelp();
}

typedef set<string,less<string> > settype;
typedef map<string,settype,less<string> > maptype;

bool 
compCmdLen(pair <string,settype> f,pair<string,settype> s) {
  return f.first.size() < s.first.size();
}

commandStatus
CommandNode::makeHelpNode() {
  list<Command*>::iterator ii;
  int hlp4param = 0;
  maptype M;
  maptype::iterator im;
  settype::iterator is,isbegin,isend;

  //bool belong2=
  vtt->vtSplit->getCmdUnderCursorNum(hlp4param);

  for(ii=cmdList.begin();ii != cmdList.end();++ii) {
    if((*ii)->makeHelp() != CMD_PARAM_HELP_AVAIL) continue;

    if((*ii)->getCommandDepth() == hlp4param) {
      im = M.find("<CR>");
      if(im == M.end()) im = M.insert(maptype::value_type("<CR>",settype())).first;
      (*im).second.insert((*ii)->getCommandHlp(hlp4param));
      continue;
    }
    im = M.find((*ii)->getCommand(hlp4param));
    if(im == M.end()) im = M.insert(maptype::value_type((*ii)->getCommand(hlp4param),settype())).first;
    (*im).second.insert((*ii)->getCommandHlp(hlp4param));
  }
  //FIXME For the furute, this place I should allocat buffer for help and use vty->pager but I do this when
  //we had have a more large CLI
  int maxCmdLen = (*max_element(M.begin(),M.end(),compCmdLen)).first.size();
  char *cmdStr = new char[maxCmdLen+2];
  for(im=M.begin();im != M.end();++im) {
    isbegin = (*im).second.begin();
    isend = (*im).second.end();
    strncpy(cmdStr,(*im).first.c_str(),(*im).first.size());
    memset(cmdStr+(*im).first.size(),' ',maxCmdLen+1-(*im).first.size());
    cmdStr[maxCmdLen+1] = '\0';
    vtt->twrite(utl::lower(cmdStr).c_str());
    vtt->twrite((*isbegin).c_str());
    vtt->twrite("\n");
    isbegin++;
    for(is=isbegin;is != isend;++is) {
      memset(cmdStr,' ',maxCmdLen+2);
      cmdStr[maxCmdLen+1] = '\0';
      vtt->twrite(cmdStr);
      vtt->twrite((*is).c_str());
      vtt->twrite("\n");
    }
  }
  delete [] cmdStr;
  return CMD_NO_HELP_AVAIL;
}

void
CommandNode::insertHistory() {
  string s;
  if((int)histList.size() == maxHistSize) {
    histList.erase(histList.begin());
    s = vtt->vty_get_line();
    histList.remove(s);
  }
  s = vtt->vty_get_line();

  //this is not perfect but I hope that maxHistSize is contain not more than 20 str lines
  for(list<string>::iterator ii=histList.begin();ii != histList.end();++ii) if((*ii) == s) return;

  histList.insert(histList.end(),s);
  histIt = --histList.end();
  if(histIt != histList.begin()) --histIt;
  return;
}

void
CommandNode::getHistory(bool next) {
  if(histList.size() == 0) return;
  vtt->twrite(invStr());

  if(next) {
    if(histIt != histList.end()) {
      vtt->vty_set_line(histIt->c_str());
      vtt->vty_redraw_line();
      histIt++;
    } else {
      vtt->twrite("\a");
      histIt = histList.begin();
      vtt->vty_set_line(histIt->c_str());
      vtt->vty_redraw_line();
      histIt++;
    }
  } else {
    if(histIt!=histList.begin() && histIt!=histList.end()) {
      vtt->vty_set_line(histIt->c_str());
      vtt->vty_redraw_line();
      histIt--;
    } else if(histIt == histList.end()) {
      --histIt;
      vtt->twrite("\a");
      vtt->vty_set_line(histIt->c_str());
      vtt->vty_redraw_line();
      histIt--;
    } else {
      vtt->twrite("\a");
      vtt->vty_set_line(histIt->c_str());
      vtt->vty_redraw_line();
      histIt = --histList.end();
    }
  }
  return;
}

void
CommandNode::errReport(commandStatus ret) {
  list<CmdErr>::iterator err;

  if(ret==CMD_EXECUTED || ret==CMD_NO_ACTION) {
    execErrors.erase(execErrors.begin(),execErrors.end());
    return;
  }

  err=max_element(execErrors.begin(),execErrors.end(),compCmdErr);
  vtt->twrite_marker_at(err->errPos+strlen(invStr())-1);
  char pos[10];
  snprintf(pos,9,"%d",err->errPos);

  switch(err->execStat) {
   case CMD_UNKNOWN:
    vtt->twriteErr("\n% Command at marker \'^\'  (col ");
    vtt->twriteErr(pos);
    vtt->twriteErr(") is unknown:");
    break;
   case CMD_PARAM_ERROR:
    vtt->twriteErr("\n% Command parametr at marker \'^\'  (col ");
    vtt->twriteErr(pos);
    vtt->twriteErr(") is unknown or incorrect:");
    break;
   case CMD_PARAM_COUNT_ERROR:
    vtt->twriteErr("\n% Parameter count error at marker \'^\'  (col ");
    vtt->twriteErr(pos);
    vtt->twriteErr(") is unknown:");
    break;
   default:
    vtt->twriteErr("\n% Unknown error at marker \'^\' (col ");
    vtt->twriteErr(pos);
    vtt->twriteErr(") is unknown:");

    break;
  }
  vtt->twriteErr(err->addComment.c_str());

  execErrors.erase(execErrors.begin(),execErrors.end());
  return;
}

commandStatus
CommandNode::completeNodeCommand() {
  list<cmdComplete>::iterator cmp;
  list<Command*>::iterator ii;
  map<string,bool,less<string> > M;
  map<string,bool,less<string> >::iterator im;

  commandStatus cs;
  bool isCompleteByPart = false;
  bool isNotCompletedParam = false;

  for(ii=cmdList.begin();ii!=cmdList.end();++ii) {
    cs=(*ii)->completeCommand();

    if(cs == CMD_NOT_COMPLETED_PARAM_ENCOUNTERED) isNotCompletedParam = true;
    if(cs == CMD_COMPLETED_BY_CMD_PART) isCompleteByPart = true;

    if(cs!=CMD_COMPLETED && cs!=CMD_COMPLETED_BY_CMD_PART) continue;
    im = M.find((*ii)->getCmdComplete().cmd);
    if(im == M.end()) im=M.insert(map<string,bool,less<string> >::value_type((*ii)->getCmdComplete().cmd,bool(false)) ).first;
    if((*im).second == false) {
      (*im).second = bool(true);
      completeStatus.insert(completeStatus.end(),(*ii)->getCmdComplete());
    }
  }
  if(isNotCompletedParam==true && isCompleteByPart==false) {
    completeStatus.erase(completeStatus.begin(),completeStatus.end());
    return CMD_NOT_COMPLETED_PARAM_ENCOUNTERED;
  }

  if(completeStatus.size() > 1) vtt->twrite("\n");
  else if(completeStatus.size() < 1) {
    vtt->twrite("\a");
    return  CMD_NOT_COMPLETED;
  }
  char current;
  int pos = 0;
  int isOneMore = 1;
  //move cursor of vty to the end of the comleted command
  int cmdPartnum = 0;
  vtt->vtSplit->getCmdUnderCursorNum(cmdPartnum);
  int start=0,len=0;
  if(vtt->vtSplit->getCmdPartPos(cmdPartnum, start, len)) {
    vtt->vty_move_to(start+len);
  }
  while((current=*(utl::lower(completeStatus.begin()->cmd).c_str() + completeStatus.begin()->paramVtyLen+pos ))!=0  &&  isOneMore==1) {
    for(cmp=completeStatus.begin();cmp != completeStatus.end();++cmp) {
      if(cmp->paramVtyLen+pos!=(int)strlen(cmp->cmd)) {
        if(current != *(utl::lower(cmp->cmd).c_str()+cmp->paramVtyLen+pos)) {
          isOneMore = 0;
          break;
        }
      } else {
        isOneMore = 0;
        break;
      }
    }
    if(isOneMore == 1) {
      vtt->vty_insert_char(current);
      pos++;
    }
  }
    
  for(cmp=completeStatus.begin();cmp != completeStatus.end();++cmp) {
    if(cmp->paramVtyLen+pos != (int)strlen(cmp->cmd)) {
      vtt->twrite(utl::lower(cmp->cmd).c_str());
      vtt->twrite(" ");
    }
  }
  if(completeStatus.size() > 1) {
    vtt->twrite(invStr());	
    vtt->vty_redraw_line();
  } else if(completeStatus.size() == 1) {
    completeStatus.erase(completeStatus.begin(),completeStatus.end());
    vtt->vty_insert_char(' ');
    this->completeNodeCommand();
    vtt->vty_del_char();
  }
  completeStatus.erase(completeStatus.begin(),completeStatus.end());
  return CMD_COMPLETED;
}

//-----------------------------------------------------=====VtyLineSplitter
VtyLineSplitter::VtyLineSplitter(Vty *vtt) {
  vt = vtt;
}

void
VtyLineSplitter::split() {
  int param_next_pos = 0;
  splitLine.erase(splitLine.begin(),splitLine.end());
  char vt_line[300];
  snprintf(vt_line,sizeof(vt_line)-1,"%s",vt->vty_get_line().c_str());
  //const char * vt_line=vt->vty_get_line().c_str();

  bool in = false;
  char delim = ' ';
  vector< mypair >::iterator paramcount;
  paramcount = splitLine.begin();

  while(vt_line[param_next_pos] != '\0') {
    if(vt_line[param_next_pos] == '\\') {
      if(in) {
      } else {
        splitLine.insert(splitLine.end(),mypair(param_next_pos,0));
        --(paramcount=splitLine.end());
        in = true;
      }
      if(vt_line[param_next_pos+1] != '\0') param_next_pos+=2;
      else param_next_pos++;
      continue;
    }
    if(!in && (vt_line[param_next_pos]=='!' || vt_line[param_next_pos]=='#' || vt_line[param_next_pos]==';') ) { //comment's
      comment.first = 1;
      comment.second = param_next_pos;
      param_next_pos = strlen(vt_line);
      continue;
    }
    if(vt_line[param_next_pos] == delim) {
      if(in) {
        paramcount->second = param_next_pos - paramcount->first;
        in = false;
        delim = ' ';
      }
      param_next_pos++;
      continue;
    }
    if(vt_line[param_next_pos] == '\'' || vt_line[param_next_pos] == '\"' || vt_line[param_next_pos] == ' ') {
      if(in) { //if we inside parametr
        param_next_pos++;
      } else {
        splitLine.insert(splitLine.end(),mypair(param_next_pos+1,0));
        --(paramcount=splitLine.end());
        delim = vt_line[param_next_pos];
        param_next_pos++;
        in = true;
      }
      continue;
    }
    if(in) {
      param_next_pos++;
      continue;
    } else {
      splitLine.insert(splitLine.end(),mypair(param_next_pos,0));
      --(paramcount=splitLine.end());
      param_next_pos++;
      in = true;
      continue;
    }
  }
  if(in) {//error if \' or simply end of line
    paramcount->second = param_next_pos - paramcount->first;
    in = false;
  }
  int ii = 0;
  int l_pos = vt->vty_get_linepos();

  paramcount = splitLine.begin();

  for(paramcount=splitLine.begin();paramcount != splitLine.end(); ++paramcount) {
//    cerr<<"Param="<<vt_line+paramcount->first<<" len="<<paramcount->second<<endl;
    if((paramcount->first <= l_pos) && (paramcount->first + paramcount->second >= l_pos)) {
      cmdUnderCursor = mypair(1,ii);
      return;
    }
    if(paramcount->first > 0) {
      if((vt_line[paramcount->first-1]=='\''||vt_line[paramcount->first-1]=='\"')&&(paramcount->first-1==l_pos)) {
        cmdUnderCursor = mypair(1,ii);
        return;
      }
    }
    if(paramcount->first+paramcount->second+1 < (int)strlen(vt_line)) {
      if((vt_line[paramcount->first+paramcount->second+1]=='\''||vt_line[paramcount->first-1]=='\"')
        &&(paramcount->first+paramcount->second+1==l_pos)) {
        cmdUnderCursor = mypair(1,ii);
        return;
      }
    }
    if(paramcount->first > l_pos) { //if cursor is between the command's part  #show  _ config //where _ is cursor pos
      if(ii != 0) cmdUnderCursor = mypair(0,ii-1);
      else cmdUnderCursor = mypair(0,0);
      return;
    }
    ii++;
  }
  cmdUnderCursor = mypair(0,ii); //be carefull if ii > depthCommand
  return;
}

bool
VtyLineSplitter::getCmdPartPos(int cmdPartNum, int &start,int  &len) {
  if(vt->isChanged) {
    split();
    vt->isChanged = false;
  }
  if((int)splitLine.size() <= cmdPartNum) return false;
  start = splitLine[cmdPartNum].first;
  len = splitLine[cmdPartNum].second;
  return true;
}

int
VtyLineSplitter::getCmdPartCount() {
  if(vt->isChanged) {
    split();
    vt->isChanged = false;
  }
  return splitLine.size();
}

bool
VtyLineSplitter::getCmdUnderCursorNum(int &cmdPartNum) {
  if(vt->isChanged) {
    split();
    vt->isChanged = false;
  }
  cmdPartNum = cmdUnderCursor.second;
  return cmdUnderCursor.first;
}

//
bool //return false if no comment's exists
VtyLineSplitter::getCommentPos(int &start,int &len) {
  if(vt->isChanged) {
    split();
    vt->isChanged = false;
  }
  if(!comment.first) return false;

  start = comment.first;
  len = vt->vty_get_line().size() - comment.second;
  return true;
}

/////////////// that is all ////////////////
