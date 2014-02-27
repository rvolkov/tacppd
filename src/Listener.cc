// Multithreaded tcp/udp-server
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"

/////////////// class Connect ////////////////

// Constructor
Connect::Connect(const socklen_t filehandle,const int p,const char *buf,
        const int st,const struct in_addr a) {
  pthread_mutex_init(&mutex_,0);
//  pthread_mutex_lock(&mutex_);
  fh_ = filehandle;
  port_ = p;
  bufx_ = (char*)buf;
  addr_ = a;
  servertype_ = st;
}

Connect::~Connect() {
//  pthread_mutex_unlock(&mutex_);
  pthread_mutex_destroy(&mutex_);
}

void
Connect::lock() {
  pthread_mutex_lock(&mutex_);
}

void
Connect::unlock() {
  pthread_mutex_unlock(&mutex_);
}

socklen_t
Connect::getSock() {
  socklen_t ret = fh_;
  return ret;
}

struct in_addr
Connect::getAddr() {
  struct in_addr ret = addr_;
  return ret;
}

int
Connect::getPort() {
  int ret = port_;
  return ret;
}

char*
Connect::getBuf() {
  char *ret;
  ret = bufx_;
  return ret;
}

int
Connect::getServertype() {
  int ret = servertype_;
  return ret;
}


/////////////// functions ////////////////

// thread which listening on port, started as thread
void
tacpp::AcceptConnection(Listener_data *us) {
  // create variable for send parameters to thread
  startlog(LOG_SERVER)<<"start AcceptConnection"<<endlog;
  if(us->UsersFunction == 0) {
    startlog(LOG_SERVER)<<"Cant start AcceptConnection due to no userfunction"<<endlog;
    pthread_exit(0);
    return;
  }
  socklen_t socketfd = 0;
  switch(us->getServertype()) {
    case TCP:
      socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      break;
    case UDP:
      socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      break;
    case UDP_MULTICAST:
      socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      break;
    default:
      startlog(LOG_ERROR)<<"Server: unknown server type"<<endlog;
      pthread_exit(0);
      return;
  }
  if(socketfd < 0) {
    startlog(LOG_ERROR)<<"socket functions returned value < 0"<<endlog;
    pthread_exit(0);
    return;
  }
  socklen_t size_sin = sizeof(struct sockaddr);
  struct sockaddr_in MyAddress, TheirAddress;
  bzero((char*)&MyAddress,sizeof(MyAddress));
  bzero((char*)&TheirAddress,sizeof(TheirAddress));

  MyAddress.sin_family = AF_INET; // Denotes an internet address
  MyAddress.sin_port = htons(us->getPort());
  MyAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  bzero(&(MyAddress.sin_zero), 8);

  if(us->getServertype() == TCP) { // this is TCP
    int on = 1;
    if(setsockopt(socketfd,SOL_SOCKET,SO_REUSEADDR,(char *)&on,
        sizeof(on)) < 0) {
      startlog(LOG_ERROR)<<"Error with SO_REUSEADDR"<<endlog;
      pthread_exit(0);
      return;
    }
  }
  if(us->getServertype() == UDP_MULTICAST) {   // UDP multicast
    if(!::coreData->getBundle()->isBundle()) {  // no bundle created
      startlog(LOG_ERROR)<<"cant init bundle listener - no bundles created"<<endlog;
      pthread_exit(0);
      return;
    }
    if(us->getPort() != ::coreData->getBundle()->getPort()) {
      startlog(LOG_ERROR)<<"cant init bundle listener - port number mismatch"<<endlog;
      pthread_exit(0);
      return;
    }
    ipaddr mip = ::coreData->getBundle()->getMgroup();
    int ttlval = ::coreData->getBundle()->getTtl();
    unsigned char loop = 0;        // disable return to loopback
    setsockopt(socketfd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    unsigned char ttl = (unsigned char)ttlval;  // multicast ttl value
    setsockopt(socketfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(mip.get().c_str()); // add to mcast group
    mreq.imr_interface.s_addr = INADDR_ANY;
    if(setsockopt(socketfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
      startlog(LOG_ERROR)<<"Failed to join IP multicast group "<<mip<<endlog;
      // failed to join IP multicast group
      pthread_exit(0);
      return;
    }
  }
  if(bind(socketfd, (struct sockaddr *)&MyAddress,sizeof(struct sockaddr))==-1) {
    startlog(LOG_ERROR)<<"Bind error; can't bind to port "<<us->getPort()<<endlog;
    pthread_exit(0);
    return;
  }
  if(us->getServertype() == TCP) { // listen only for SOCK_STREAM
    if(listen(socketfd, 5) < 0) {
      startlog(LOG_ERROR)<<"Listen error"<<endlog;
      pthread_exit(0);
      return;
    }
  }
  pthread_mutex_t Mutex;
  pthread_mutex_init(&Mutex,0);
  
  Connect *NewConnect;
  pthread_attr_t tattr; // thread attribute
  pthread_attr_init(&tattr); // init thread attribute
  pthread_attr_setdetachstate(&tattr,PTHREAD_CREATE_DETACHED);

  int *ses;
  void **args;
  
  while(us->isCreated()) {		// if listener must work
    socklen_t AcceptFD = 0;
    char *bufudp = 0;
    if(us->getServertype() == TCP) { // TCP
#ifndef LINUX
      us->llock();
#endif
      int socktmp = accept(socketfd, (struct sockaddr *)&TheirAddress,&size_sin);
#ifndef LINUX
      us->lunlock();
#endif
      if(socktmp == -1) {
        startlog(LOG_SERVER)<<"TCP accept function returned value -1"<<endlog;
        continue;
      }
      AcceptFD = (socklen_t)socktmp;
    } else { // UDP or UDP multicast
      bufudp = new char[MAX_UDP];
      AcceptFD = socketfd;
      bzero(bufudp, MAX_UDP);
      int sz = recvfrom(AcceptFD,bufudp,MAX_UDP,0,
        (struct sockaddr *)&TheirAddress,&size_sin);
      if(sz < 0) {
        if(bufudp) delete [] bufudp;
        startlog(LOG_SERVER)<<"UDP recvfrom returned value < 0"<<endlog;
        continue;
      }
    }
    // check by acl
    char address[30];
    snprintf(address,sizeof(address)-1,"%s",inet_ntoa(TheirAddress.sin_addr));
    if(!::coreData->getListener()->check(us->getPort(),address)) {
      startlog(LOG_INFORM)<<"Unauthorized access "<<address<<" don't match acl "<<us->getAcl()<<endlog;
      if(us->getServertype() == TCP) {
        close(AcceptFD);
      }
      if(bufudp) delete [] bufudp;
      continue;
    }
    // ok, working
    //startlog(LOG_SERVER)<<"open socket "<<AcceptFD<<" from "<<address<<" to port "<<us->getPort()<<endlog;
    if(us->con_session()>=us->getMaxconnect() && us->getServertype()==TCP) {
      startlog(LOG_INFORM)<<
        "Too many open connections ("<<us->con_session()<<
          "vs "<<us->getMaxconnect()<<") for server "<<us->getServname()<<" on port "<<
            us->getPort()<<endlog;
      close(AcceptFD);
      continue;
    }
    int port = us->getPort();
    int servertype = us->getServertype();

    NewConnect = new Connect(AcceptFD, port, bufudp, servertype, TheirAddress.sin_addr); // new connect structure
    ses = new int;
    *ses = us->add_session(NewConnect); // counter for limit max open sessions
    args = new void* [3];
    args[0] = us;
    args[1] = NewConnect;
    args[2] = ses;
    
    if(us->getServertype() == TCP) {
      if(pthread_create(&(NewConnect->HandlerThread),&tattr,
          (void*(*)(void*))&HandleConnection,args) != 0) { // error
        startlog(LOG_ERROR)<<"Error thread create for connection handler"<<endlog;
        close(AcceptFD);
        us->del_session(0);
        delete NewConnect;
        delete ses;
        delete [] args;
        continue;
      }
      // very strange and ugly code for prevent locking if something wrong,
      // also for prevent variables from destruction during handler
      // copy this variables
//      bool waiter = true;
//      int tmpint = 0;
//      while(waiter) {
//        pthread_mutex_lock(&Mutex);
//        tmpint = stopper;
//        pthread_mutex_unlock(&Mutex);
//        if(tmpint == 0) waiter = false;
//      }
    } else {  // UDP
      us->UsersFunction(AcceptFD,TheirAddress.sin_addr,port,bufudp);
      if(bufudp) delete [] bufudp;
      us->del_session(0);
      delete NewConnect;
      delete ses;
      delete [] args;
    }
  }
  pthread_mutex_destroy(&Mutex);
  pthread_exit(0); // end thread
}

// run user function and exit
// start in separate thread
int
tacpp::HandleConnection(void **args) {
  Listener_data *tus = (Listener_data *)(args[0]);
  Connect *Cxn = (Connect *)(args[1]);
  int *ses = (int*)(args[2]);
  socklen_t fh = Cxn->getSock();
  int port = Cxn->getPort();
  struct in_addr addr = Cxn->getAddr();
  char *bufx = 0;
  bufx = Cxn->getBuf();
  pthread_t HandlerThread = Cxn->HandlerThread;

  tus->UsersFunction(fh,addr,port,bufx);

  delete Cxn;
  delete ses;
  delete [] args;
  
  shutdown(fh,2); // shutdown handler
  close(fh); // close handler
  tus->del_session(0); // delete current thread from qDescr

  return 0;
}

//////////////////// That's all, folks ////////////////////
