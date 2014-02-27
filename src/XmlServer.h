// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2007 by tacppd team and contributors
// see http://tacppd.org for more information
//
// file: XmlRpc.h
// description: http support

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

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

#ifndef __XMLRPC_H__
#define __XMLRPC_H__

#ifdef WITH_XMLRPC

#include "global.h"

namespace tacpp {

/**
@short xml-rpc server
*/
class XmlServer {
  char address[30];
 public:
  XmlServer(int fh,struct in_addr addr,int port);
  void process();
};

}

#endif //WITH_XMLRPC

#endif //__XMLRPC_H__
