// The main() procedure place
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"
#include "../version.h"
#include "../build/vardata.h"

// set platform name during compilation
#ifdef LINUX
#define T_PLATFORM	"Linux"
#endif
#ifdef SOLARIS
#define T_PLATFORM	"SUN Solaris"
#endif
#ifdef FREEBSD
#define T_PLATFORM	"FreeBSD"
#endif

//////////////// core data structures //////

namespace tacpp {
  /**
  core data and processes */
  CoreData *coreData;
  /**
  current working users */
  UserTrack userTrack;
  /**
  debug/log system */
  Logging logSystem;
  /**
  debug to terminals */
  LogTerm *logTermSystem;
  /**
  debug/log to file */
  LogFile *logFileSystem;
  /**
  debug to syslog */
  LogSyslog *logSyslogSystem;
  /**
  snmp polling system */
  Snmp snmpSystem;
  /**
  directory where tacppd placed - it's root dir */
  char MAINDIR[MAINDIR_LEN];
  /**
  server tracking system */
  ServerTrack serverTrack;
  /**
  netflow aggregator */
  NfAggregator nfAggregator;
};

///////////////////////////////////////

void start_error(char *sss) {
  std::cerr<<T_PACKAGE<<"an error: "<<sss<<std::endl;
}

// periodical tasks
void
low_priority_tasks() {
  while(true) {
    ::userTrack.periodic();         // periodic user tracking
    ::logTermSystem->logwrite();    // debug send to all terminals
    ::logSyslogSystem->logwrite();  // debug send to syslog
//std::cerr<<"nfd_.size="<<::nfAggregator.size()<<" expire="<<::nfAggregator.expireTime()<<" nodes="<<::nfAggregator.nodesSize()<<" sources="<<::nfAggregator.sourceSize()<<std::endl;
    ::logFileSystem->logwrite();    // log flush to file - must be the last!!!
    ::coreData->getBundle()->periodic(); // send keepalive packets every 10 seconds
    ::coreData->getDevice()->startPeriodic(); // device snmp/icmp polling process
    sleep(1);                       // delay for any task
  }
}

class mainErrorEx {};

//// main
int
main(int argc,char **argv, char **val) {
  ::logFileSystem = 0;
  ::logSyslogSystem = 0;
  ::logTermSystem = 0;
  ::coreData = 0;

  // set LC_ALL to "" (the LANG and direct values should work)
  setlocale(LC_ALL,""); // as i know, using LC_ALL is bad practice, so clear it
  // out start message
  std::cerr<<T_PACKAGE<<"Version: "<<T_VERSION<<" for "<<T_PLATFORM<<std::endl;
  std::cerr<<T_PACKAGE<<"Copyright (c) "<<T_COPYRIGHT<<std::endl;
  std::cerr<<T_PACKAGE<<"compiled "<<T_BUILDED<<" by user "<<T_PERSON<<std::endl;
  std::cerr<<T_PACKAGE<<T_WARRANTY<<std::endl;
  std::cerr<<T_PACKAGE<<T_SUPPORT<<std::endl;
  int ch;
  bool foreground = false;
  bool maindir = false;
  // understand keys:  -f  - foreground work
  // -d /path/to/tacppd/dir  - path to maindir

  while((ch=getopt(argc,argv,"d:f")) != EOF) {
    switch(ch) {
      case 'f':
        foreground = true;
        break;
      case 'd':
        maindir = true;
        strncpy(::MAINDIR,optarg,sizeof(::MAINDIR));
        break;
      case '?':
      default:
        start_error("Only -d [directory] and -f keys permit");
        std::cerr<<T_PACKAGE<<"   -d [directory]   - the program main path"<<std::endl;
        std::cerr<<T_PACKAGE<<"   -f  - working foreground"<<std::endl;
        exit(1);
        break;
    }
  }
  if(!maindir) {
    char *maindir_tmp = getenv("TACPPD_DIR");
    if(!maindir_tmp)
      strncpy(::MAINDIR,MAINDIR_DEF,sizeof(::MAINDIR));
    else
      strncpy(::MAINDIR,maindir_tmp,sizeof(::MAINDIR));
  }
  char spath[150];

 try {

  // create log/debug system
  snprintf(spath,sizeof(spath)-1,"%s/log",::MAINDIR);
  ::logFileSystem = new LogFile(&::logSystem,spath);  // create log system
  ::logSyslogSystem = new LogSyslog(&::logSystem);    // create syslog system
  ::coreData = new CoreData;  // create Core objects (require access to logSystem and other)
  ::logTermSystem = new LogTerm(&::logSystem);        // create term log system
  int fh = 0;
  struct in_addr addr;
  Terminal *confr;
  confr = new Terminal(fh,addr);
//  snprintf(spath,sizeof(spath),"%s/etc/tacppd.conf",::MAINDIR);
  snprintf(spath,sizeof(spath),"etc/tacppd.conf");
  std::cerr<<T_PACKAGE<<"Read config from "<<spath<<std::endl;
  confr->process((const char*)spath);
  delete confr;

  if(foreground) {
    std::cerr<<T_PACKAGE<<"config have loaded, working foreground."<<std::endl;
  } else {
    std::cerr<<T_PACKAGE<<"config have loaded, become daemon process."<<std::endl;
    Daemon d; // daemon object
    if(d.daemon(0,0) < 0) {
      start_error("Daemonization error, exiting");
      throw mainErrorEx();
    }
  }
  // write pidfile
  char str[200];
  snprintf(str,sizeof(str)-1,"%s/run/tacppd.pid",::MAINDIR);
  unlink(str);
  int fp = open(str, O_CREAT | O_WRONLY, 0600);
  if(fp >= 0) {
    snprintf(str,sizeof(str),"%d\n",getpid());
    write(fp,str,strlen(str));
    close(fp);
  } else {
    start_error("Can't create pidfile, exiting");
    throw mainErrorEx();
  }
  // start servers
  startlog(LOG_EVENT)<<"starting listeners"<<endlog;
  ::coreData->getListener()->init();
  startlog(LOG_EVENT)<<"connect to bundle"<<endlog;
  ::serverTrack.sendStart(); // start entering to bundle
  startlog(LOG_EVENT)<<"starting NetFlow aggregator"<<endlog;  
  nfAggregator.runProcess(); // run nfAggregator expiring process
  startlog(LOG_EVENT)<<"starting low priority tasks"<<endlog;  
  low_priority_tasks();   // circle with low priv tasks
  // never here, as prev method - unconditional loop
  throw mainErrorEx();
 }
 catch(mainErrorEx) {
  if(::logFileSystem) delete ::logFileSystem;
  if(::logSyslogSystem) delete ::logSyslogSystem;
  if(::logTermSystem) delete ::logTermSystem;
  if(::coreData) delete ::coreData;
  _exit(1);
 }
 catch(...) {
  if(::logFileSystem) delete ::logFileSystem;
  if(::logSyslogSystem) delete ::logSyslogSystem;
  if(::logTermSystem) delete ::logTermSystem;
  if(::coreData) delete ::coreData;
  cerr<<"Unknown exception received"<<endl;
  _exit(1);
 }
}

/////////////////// that's all /////////////////////
