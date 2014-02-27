// tacppd application functions
// (c) Copyright 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

#include "global.h"

// buf variable here  - for udp packets, tcp uses sockets

// This is incoming TACACS+ connection handler
void
tacpp::tacacs_server(int fh,struct in_addr addr,int port,char *buf) {
  char address[30];
  snprintf(address,sizeof(address),"%s",inet_ntoa(addr));
  TacacsPacketReader *tpr = new TacacsPacketReader(address,fh);
  // i know that use constructor for main operation is wrong style, but...
  delete tpr;
  return;
}

// gtp_prime_server
void
tacpp::gtpp_server(int fh,struct in_addr addr,int port,char *buf) {
  char address[30];
  snprintf(address,sizeof(address),"%s",inet_ntoa(addr));
  ipaddr ip = address;
#ifdef COMMERCIAL_TACPPD
  GtppServerProcessor *gsp = new GtppServerProcessor(ip);
  delete gsp;
#endif
}

// diameter_server
void
tacpp::diameter_server(int fh,struct in_addr addr,int port,char *buf) {
  char address[30];
  snprintf(address,sizeof(address),"%s",inet_ntoa(addr));
  ipaddr ip = address;
//  DiameterServerProcessor *dsp = new DiameterServerProcessor(ip);
//  delete dsp;
}

// radius_server
void
tacpp::radius_server(int fh,struct in_addr addr,int port,char *buf) {
  char address[30];
  snprintf(address,sizeof(address),"%s",inet_ntoa(addr));
  ipaddr ip = address;
//  RadiusServerProcessor *rsp = new RadiusServerProcessor(ip);
//  delete rsp;
}

// radius accounting server
void
tacpp::radacct_server(int fh,struct in_addr addr,int port,char *buf) {
  char address[30];
  snprintf(address,sizeof(address),"%s",inet_ntoa(addr));
  ipaddr ip = address;
//  RadiusAccProcessor *rap = new RadiusAccProcessor(ip);
//  delete rap;
}

// HTTP connection handler
void
tacpp::http_server(int fh,struct in_addr addr,int port,char *buf) {
  char address[30];
  snprintf(address,sizeof(address),"%s",inet_ntoa(addr));
//  startlog(LOG_SERVER)<<"process HTTP connect from "<<address<<" to port "<<port<<endlog;
  HttpServer *http = new HttpServer(fh, addr);  // create http server
  http->process();
  delete http; // delete http server
  return;
}

#ifdef WITH_XMLRPC
// XML-RPC connection handler
void
tacpp::xmlrpc_server(int fh,struct in_addr addr,int port,char *buf) {
  char address[30];
  snprintf(address,sizeof(address),"%s",inet_ntoa(addr));
//  startlog(LOG_SERVER)<<"process XML-RPC connect from "<<address<<" to port "<<port<<endlog;
  XmlServer *xml = new XmlServer(fh, addr, port);  // create xml-rpc server
  xml->process();
  delete xml; // delete xmlrpc server
  return;
}
#endif

// TELNET connection handler
void
tacpp::terminal_server(int fh,struct in_addr addr,int port,char *buf) {
  char address[30];
  snprintf(address,sizeof(address),"%s",inet_ntoa(addr));
//  startlog(LOG_SERVER)<<"process TELNET connect from "<<address<<" to port "<<port<<endlog;
  Terminal *t = new Terminal(fh,addr);  // create terminal
  t->process();
  delete t; // delete terminal
  return;
}

// roaming PEER connection handler
void
tacpp::peer_server(int fh,struct in_addr addr,int port,char *buf) {
  char address[30];
  snprintf(address,sizeof(address),"%s",inet_ntoa(addr));
  startlog(LOG_SERVER)<<"process PEER connect from "<<address<<" to port "<<port<<endlog;
//  Peer p(fh,addr);
//  p.process();
  return;
}

// neighbor connection handler
void
tacpp::bundle_server(int fh,struct in_addr addr,int port,char *buf) {
  char address[30];
  snprintf(address,sizeof(address),"%s",inet_ntoa(addr));
  ipaddr ip = address;
  BundleProcessor *bp = new BundleProcessor(ip,port,buf);
  delete bp;
}

// NetFlow connection handler
void
tacpp::netflow_server(int fh, struct in_addr addr, int port,char *buf) {
  //startlog(LOG_SERVER)<<"process NetFlow connect from "<<address<<" to port "<<port<<endlog;
  ::nfAggregator.process(addr.s_addr, buf);
  return;
}

// snmp traps connection handler
void
tacpp::traps_server(int fh,struct in_addr addr,int port,char *buf) {
  char address[30];
  snprintf(address,sizeof(address),"%s",inet_ntoa(addr));
  ipaddr ip = address;
//  startlog(LOG_SERVER)<<"process SNMP TRAP connect from "<<address<<" to port "<<port<<endlog;
  return;
}

// snmp agent code
void
tacpp::snmp_server(int fh,struct in_addr addr,int port,char *buf) {
  char address[30];
  snprintf(address,sizeof(address),"%s",inet_ntoa(addr));
  ipaddr ip = address;
//  startlog(LOG_SERVER)<<"process SNMP connect from "<<address<<" to port "<<port<<endlog;
  return;
}

// Tacppd Control Interface (TCI)
void
tacpp::tci_server(int fh,struct in_addr addr,int port,char *buf) {
  char address[30];
  snprintf(address,sizeof(address),"%s",inet_ntoa(addr));
//  startlog(LOG_SERVER)<<"process TCI connect from "<<address<<" to port "<<port<<endlog;
  TciServer *xs = new TciServer(fh, addr); // create tci server process
  xs->process(); // process connect
  delete xs; // delete tci server process
  return;
}

// Syslog server
void
tacpp::syslog_server(int fh,struct in_addr addr,int port,char *buf) {
  char address[30];
  snprintf(address,sizeof(address),"%s",inet_ntoa(addr));
//  SyslogServer *ss = new SyslogServer(fh, addr); // create server process
//  ss->process(); // process connect
//  delete ss; // delete server process
  return;
}

// SMPP server
void
tacpp::smpp_server(int fh,struct in_addr addr,int port,char *buf) {
  char address[30];
  snprintf(address,sizeof(address),"%s",inet_ntoa(addr));
//  SyslogServer *ss = new SyslogServer(fh, addr); // create server process
//  ss->process(); // process connect
//  delete ss; // delete server process
  return;
}

// HTTPS server
void
tacpp::https_server(int fh,struct in_addr addr,int port,char *buf) {
  char address[30];
  snprintf(address,sizeof(address),"%s",inet_ntoa(addr));
//  SyslogServer *ss = new SyslogServer(fh, addr); // create server process
//  ss->process(); // process connect
//  delete ss; // delete server process
  return;
}


///////////////////// that is all ///////////////////
