// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2007 by tacppd team and contributors
// see http://tacppd.org for more information
//
// file: XmlServer.cc
// description: xml-rpc interface

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// Brett Maxfield, maxfieb@spacenow.net

// modifications:
//

#ifdef WITH_XMLRPC

#include "global.h"

XmlRpcServer s;

/**
@short simple version method
*/
class XML_Version : public XmlRpcServerMethod
{
public:
  XML_Version(XmlRpcServer* s) : XmlRpcServerMethod("version", s) {}
  void execute(XmlRpcValue& params, XmlRpcValue& result)
  {
      result = string(T_VERSION);
  }
} version(&s);

/////////////////// XmlRpcServer Process //////////////////

XmlServer::XmlServer(int fh,struct in_addr addr,int port) {
    // create server
    s.bindAndListen(port);
}

// process xml-rpc connections
void
XmlServer::process() {
    s.work(-1.0);
}

#endif //WITH_XMLRPC

///////////////// that's all, folks /////////////////////
