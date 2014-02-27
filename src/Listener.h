// Low-level listener methods
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
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

#ifndef __LISTENER_H__
#define __LISTENER_H__

#include "global.h"

namespace tacpp {

#define TCP             1
#define UDP             2
#define UDP_MULTICAST   3
#define XMLRPC          4

// max UDP datagram size
#define MAX_UDP       2048

/**
@short object for send data to created thread
@version 1
@author rv
*/
class Connect {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  socket handler */
  socklen_t fh_;
  /**
  addr */
  struct in_addr addr_;
  /**
  connection port number */
  int port_;
  /**
  buffer for UDP data */
  char *bufx_;
  /**
  type of server */
  int servertype_;
 public:
  pthread_t HandlerThread;
  int number;
  /**
  set thread lock */
  void lock();
  /**
  remove thread lock */
  void unlock();
  /**
  init */
  Connect(const socklen_t,const int,const char*,const int,const struct in_addr);
  /**
  free */
  ~Connect();
  /**
  get socket
  @return socket */
  socklen_t getSock();
  /**
  get addr
  @return addr */
  struct in_addr getAddr();
  /**
  get port
  @return port */
  int getPort();
  /**
  get buf
  @return udp packet buf */
  char *getBuf();
  /**
  get servertype
  @return server type */
  int getServertype();
};

//class ConnectManager {
//  list<Connect*> con_;
//public:
//};

class Listener_data;

void AcceptConnection(Listener_data*);
int HandleConnection(void**);

class ListenerHandlerEnd {};

};

#endif //__LISTENER_H__
