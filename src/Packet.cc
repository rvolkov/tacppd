// tacppd basic tcp/udp packet functions
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

#include "global.h"

/////////// Basic packet functions ////////////

Packet::Packet(int fd) {
  pthread_mutex_init(&mutex_,0);
  lock();
  signal(13,SIG_IGN); // ignore SIGPIPE
  // we do it because write() call generate SIGPIPE signal
  // if TCP socket (not only TCP) will close from other side
  // during data transmitting, do not know how it works in normal programs,
  // but here it drop whole server
  fid_ = fd;
  unlock();
}

Packet::~Packet() {
  pthread_mutex_destroy(&mutex_);
}

void
Packet::lock() {
  pthread_mutex_lock(&mutex_);
}

void
Packet::unlock() {
  pthread_mutex_unlock(&mutex_);
}

void
Packet::setFid(int d) {
  lock();
  fid_ = d;
  unlock();
}

int
Packet::getFid() {
  int ret;
  lock();
  ret = fid_;
  unlock();
  return ret;
}

// read from socket
int
Packet::sockread(unsigned char* ptr,int nbytes,int timeout) {
  int fd;
  fd = getFid();
  int nleft, nread;
  fd_set readfds, exceptfds;
  struct timeval tout;

  if(fd == -1) return -1;

  tout.tv_sec = timeout;
  tout.tv_usec = 0;
  FD_ZERO(&readfds);
  FD_SET(fd, &readfds);
  FD_ZERO(&exceptfds);
  FD_SET(fd, &exceptfds);
  nleft = nbytes;
  int status;
  while(nleft > 0) {
    status = select(fd + 1, &readfds, (fd_set *) NULL,
      &exceptfds, &tout);
    if(status==0 || status<0) {
      return -1;
    }
    if(FD_ISSET(fd, &exceptfds)) {
      return -1;
    }
    if(!FD_ISSET(fd, &readfds)) continue;
    nread = read(fd, ptr, nleft);
    if(nread < 0) {
      return -1;
    } else if(nread == 0) {
      return -1;
    }
    if(nread > nleft) {
      return -1;
    }
    nleft -= nread;
    if(nleft) ptr += nread;
  }
  int ret;
  ret = nbytes - nleft;
  return ret;
}

// write to socket
int
Packet::sockwrite(unsigned char *ptr,int bytes,int timeout) {
  int fd;
  fd = getFid();
  int remaining, sent;
  fd_set writefds, exceptfds;
  struct timeval tout;
  if(fd == -1) {
    return -1;
  }
  sent = 0;
  tout.tv_sec = timeout;
  tout.tv_usec = 0;
  FD_ZERO(&writefds);
  FD_SET(fd, &writefds);
  FD_ZERO(&exceptfds);
  FD_SET(fd, &exceptfds);
  remaining = bytes;
  int status = 0;
  while(remaining > 0) {
    status = select(fd + 1, (fd_set *) NULL,
        &writefds, &exceptfds, &tout);
    if(status==0 || status<0) {
      return -1;
    }
    if(FD_ISSET(fd, &exceptfds)) {
      return sent;
    }
    if(!FD_ISSET(fd, &writefds)) continue;
    sent = write(fd, ptr, remaining);
    if(sent <= 0) {
      return sent;
    }
    if(sent > remaining) {
      return -1;
    }
    remaining -= sent;
    ptr += sent;
  }
  int ret;
  ret = bytes - remaining;
  return ret;
}

//////////////// that's all //////////////////
