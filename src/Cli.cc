// tacppd command-line interface
// (c) Copyright 2000-2007 by tacppd team and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications: Igor Lyapin aka lsk

#include "global.h"

struct tempCommandData {
  char *cmdID;
  char *inviteStr;
  char *inviteStrMark;
  char *ecmdID;
  char *crhelp;

  char **emask;
  char **ehelp;
  char **mask;
  char **help;

  char **attr;
};

static void
startFun(void *data, const char *el, const char **attr) {
  int i;
  int depth = 1;
  tempCommandData t;
  t.cmdID = 0;
  t.inviteStr = 0;
  t.inviteStrMark = 0;
  t.ecmdID = 0;
  t.crhelp = 0;
  common *cm = (common*)data;

  //  commandnode cmdID='database' inviteStr='(database)' ecmdID='edatabase' emask='Exit' ehelp='Exit from database section' mask='DAtabase' help='Database control section.\n' inviteStrMark='#' crhelp='Press enter to execute command.'
  //  command cmdID='dcreate' mask='CReate' help='Create all database tables(only for first time).crhelp='Press enter to execute command.'
  for(i = 0; attr[i]; i += 2) depth++;
  t.attr = new char_ptr[depth*2+1];
  for (i = 0; attr[i]; i += 1) {
    t.attr[i] = new char[strlen(attr[i])+1];
    strcpy(t.attr[i],attr[i]);
  }
  t.attr[i] = 0; // set zero in last position
  t.mask = new char_ptr[depth+2];
  t.help = new char_ptr[depth+2]; //for null and for <CR>
  char **curMask = t.mask;
  char **curHelp = t.help;

  t.emask = new char_ptr[depth+2];
  t.ehelp = new char_ptr[depth+2]; //for null and for <CR>
  char **curEMask = t.emask;
  char **curEHelp = t.ehelp;

  for (i = 0; t.attr[i]; i += 2) {
    if(strcmp(t.attr[i],"cmdID")==0) {
       t.cmdID = t.attr[i + 1];
    } else if(strcmp(t.attr[i],"inviteStr")==0) {
      t.inviteStr = t.attr[i + 1];
    } else if(strcmp(t.attr[i],"inviteStrMark")==0) {
      t.inviteStrMark = t.attr[i + 1];
    } else if(strncmp(t.attr[i],"emask",5)==0) {
      *curEMask = t.attr[i + 1];
      *curEMask++;
    } else if(strncmp(t.attr[i],"ehelp",5)==0) {
      *curEHelp = t.attr[i + 1];
      *curEHelp++;
    } else if(strcmp(t.attr[i],"ecmdID")==0) {
        t.ecmdID = t.attr[i + 1];
    } else if(strcmp(t.attr[i],"crhelp")==0) {
      t.crhelp = t.attr[i + 1];
    } else if(strncmp(t.attr[i],"help",4)==0) {
      *curHelp = t.attr[i + 1];
      *curHelp++;
    } else if(strncmp(t.attr[i],"mask",4)==0) {
      *curMask = t.attr[i + 1];
      *curMask++;
    }
  }
  *curHelp = t.crhelp; *curHelp++;
  *curEHelp = t.crhelp;    *curEHelp++;
  *curHelp = 0;
  *curMask = 0;
  *curEHelp = 0;
  *curEMask = 0;
  if(strcmp(el,"commandnode")==0) {
    if(cm->currentNode) {
      Command *temp = cm->currentNode;
      Command *nitemp = cm->currentNiNode;
      cm->currentNode = new CommandNode(t.mask,t.help,t.cmdID,t.emask,t.ehelp,t.ecmdID,t.inviteStr,t.inviteStrMark,temp);
      cm->currentNiNode = new NonInteractiveCommandNode(t.mask,t.help,t.cmdID,t.emask,t.ehelp,t.ecmdID,nitemp);
      ((CommandNode*)temp)->insertCommand(cm->currentNode);
      ((NonInteractiveCommandNode*)nitemp)->insertCommand(cm->currentNiNode);
    } else {
      cm->currentNode = new CommandNode(t.mask,t.help,t.cmdID,t.emask,t.ehelp,t.ecmdID,t.inviteStr,t.inviteStrMark,cm->currentNode);
      cm->currentNiNode = new NonInteractiveCommandNode(t.mask,t.help,t.cmdID,t.emask,t.ehelp,t.ecmdID,cm->currentNiNode);
    }
  } else {
    ((CommandNode*)cm->currentNode)->insertCommand(new Command(t.mask,t.help,t.cmdID,cm->currentNode));
    ((NonInteractiveCommandNode*)cm->currentNiNode)->insertCommand(new Command(t.mask,t.help,t.cmdID,cm->currentNiNode));
  }

  delete [] t.mask;
  delete [] t.help;
  delete [] t.emask;
  delete [] t.ehelp;

  for(int ii=0;t.attr[ii];ii++) {
    delete [] t.attr[ii];
  }
  delete [] t.attr;
}

static void
endFun(void *data, const char *el) {
  common *cm = (common*)data;
  if(strcmp(el,"commandnode")==0) {
    if(cm->currentNode->upNode) {
      cm->currentNode=cm->currentNode->upNode;
      cm->currentNiNode=cm->currentNiNode->upNode;
    }
  }
}

Cli::Cli(char *xmlBuf) {
  root = NULL;
  common cm;
  cm.currentNode = NULL;
  cm.currentNiNode = NULL;

  XML_Parser p = XML_ParserCreate(NULL);
  if(!p) {
    //Could not allocate memory FIXME
  }

  XML_SetUserData(p,(void*) &cm);
  XML_SetElementHandler(p, startFun, endFun);
  int done = 0;
  int xmlBufl = strlen(xmlBuf);
  if(! XML_Parse(p, xmlBuf, xmlBufl, done)) {
   //  "Error while parsing line %d:\n%s\n",  XML_GetCurrentLineNumber(p),  XML_ErrorString(XML_GetErrorCode(p)))
   // What should I do with erroneous XML CLI description's ?? may be error throw?
  }
  root = (CommandNode*)cm.currentNode;
  niroot = (NonInteractiveCommandNode*)cm.currentNiNode;
}

CommandNode*
Cli::getRoot() {
  return root;
}

NonInteractiveCommandNode *
Cli::getNiRoot() {
  return niroot;
}

Cli::~Cli() {
  delete root;
  delete niroot;
}

////////// that is all //////////////
