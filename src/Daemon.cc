// low-level daemonization
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: Daemon.cc
// description: universal daemonization

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

// Hope, this will work with all POSIX systems

#include "global.h"

//
// close all FDs >= a specified value
//
void
Daemon::closeall(int fd) {
#ifndef SOLARIS
  int fdlimit = sysconf(_SC_OPEN_MAX);
  while(fd < fdlimit) close(fd++);
#endif //SOLARIS
}

// detach process from user and disappear into the background
// returns -1 on failure, but you can't do much except exit in that case
// since we may already have forked. This is based on the BSD version,
// so the caller is responsible for things like the umask, etc.
int
Daemon::daemon(int nochdir, int noclose) {
  switch(fork()) {
  case 0:
    break;
  case -1:
    return -1;
  default:
    _exit(0); // exit the original process
//  throw ErrExit();
  }
  if(setsid() < 0)  // shoudn't fail
    return -1;
  // dyke out this switch if you want to acquire a control tty in
  // the future -- not normally advisable for daemons
  switch(fork()) {
  case 0:
    break;
  case -1:
    return -1;
  default:
    _exit(0);
//  throw ErrExit();
  }
  if(!nochdir) chdir("/");
  if(!noclose) {
    closeall(0);
    open("/dev/null",O_RDWR);
    dup(0); dup(0);
  }
  return 0;
}

// fork2() -- like fork, but the new process is immediately orphaned
//            (won't leave a zombie when it exits)                 
// Returns 1 to the parent, not any meaningful pid.               
// The parent cannot wait() for the new process (it's unrelated).

// This version assumes that you *haven't* caught or ignored SIGCHLD.
// If you have, then you should just be using fork() instead anyway. 

int
Daemon::fork2() {
  pid_t pid;
  //int rc;
  int status;

  if(!(pid = fork())) {
    switch (fork()) {
    case 0:
      return 0;
    case -1:
      _exit(errno);    // assumes all errnos are <256
      //throw ErrExit();
    default:
      _exit(0);
      //throw ErrExit();
    }
  }
  if(pid < 0 || waitpid(pid,&status,0) < 0) return -1;
  if(WIFEXITED(status)) {
    if(WEXITSTATUS(status) == 0) return 1;
    else errno = WEXITSTATUS(status);
  } else {
    errno = EINTR; // well, sort of :-)
  }
  return -1;
}

///////////////////// that's all //////////////////////////
