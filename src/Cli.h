// tacppd Application objects
// (c) Copyright 2000-2007 by tacppd team and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:Igor Lyapin aka lsk

// all comments which start from /** - this is comments for KDoc
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

#ifndef __CLI_H__
#define __CLI_H__

#include "global.h"

namespace tacpp {

struct common {
  Command *currentNode;
  Command *currentNiNode;
  static XML_Parser *p;
};

/**
@short CLI - simple class for creating Command Line Interface from XML description.
@version 1
*/
class Cli {
 private:
  /**
   */
  CommandNode *root;
  /**
   */
  NonInteractiveCommandNode *niroot;
 public:
  /**
   */
  Cli(char *xmlBuf);
  /**
   */
  CommandNode * getRoot();
  /**
   */
  NonInteractiveCommandNode * getNiRoot();
  /**
   */
  ~Cli();
};

};
#endif  //__CLI_H__
