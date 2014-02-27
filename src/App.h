// tacppd Application methods for call from tcp/udp server
// (c) Copyright 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

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

#ifndef __APP_H__
#define __APP_H__

#include "global.h"

namespace tacpp {

extern void tacacs_server(int,struct in_addr,int,char*);
extern void gtpp_server(int,struct in_addr,int,char*);
extern void diameter_server(int,struct in_addr,int,char*);
extern void radius_server(int,struct in_addr,int,char*);
extern void radacct_server(int,struct in_addr,int,char*);
extern void http_server(int,struct in_addr,int,char*);
extern void https_server(int,struct in_addr,int,char*);
#ifdef WITH_XMLRPC
extern void xmlrpc_server(int,struct in_addr,int,char*);
#endif
extern void terminal_server(int,struct in_addr,int,char*);
//extern void ssh_server(int,struct in_addr,int,char*);
//extern void ssl_server(int,struct in_addr,int,char*);
extern void netflow_server(int,struct in_addr,int,char*);
extern void traps_server(int,struct in_addr,int,char*);
extern void peer_server(int,struct in_addr,int,char*);
extern void bundle_server(int,struct in_addr,int,char*);
extern void snmp_server(int,struct in_addr,int,char*);
extern void tci_server(int,struct in_addr,int,char*);
extern void syslog_server(int,struct in_addr,int,char*);
extern void smpp_server(int,struct in_addr,int,char*);

};
#endif //__APP_H__
