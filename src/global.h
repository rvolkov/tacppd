// global defines, includes etc. In every file
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#ifndef __global_H__
#define __global_H__

//#include "../version.h"

#ifndef _PTHREADS
  #define   _PTHREADS   // enable STL pthread-safe code
#endif //_PTHREADS

// set pthread-safe stdlibs
#ifndef _REENTRANT
  #define	_REENTRANT    
#endif //_REENTRANT

// set posix threads safe code in glibc
//#ifndef _POSIX_THREAD_SAFE_FUNCTIONS
//#define _POSIX_THREAD_SAFE_FUNCTIONS
//#endif // _POSIX_THREAD_SAFE_FUNCTIONS
#ifdef __APPLE__
#define FREEBSD   1
#endif

//#ifdef __linux__
//  #define _GNU_SOURCE
//  #include <features.h>
//#endif __linux__
#ifdef FREEBSD
  typedef int socken_t;    // FreeBSD use int for socket
  #include <sys/param.h>
#endif //FREEBSD

#define MAINDIR_LEN	200

#ifndef MAINDIR_DEF
  #define MAINDIR_DEF      "."
#endif //MAINDIR_DEF

#define DEF_CONFIG_FILE     "etc/tacpp.conf"
#define DEF_ERROR_FILE      "log/debug.log"

#define PATH_TO_DB_MODULES  "db"
#define PATH_TO_TMP         "tmp"

typedef unsigned long Bit32;

const int MAX_AVP = 100;   // max number of avpairs

const int USERNAME_LEN = 32;    // max username length
const int AUTHTYPE_LEN = 5;     // authen type str len
const int PASSWORD_LEN = 50;    // max password length
const int PORT_LEN = 32;    	// max portname length
const int REM_ADDR_LEN = 32;    // max rem_addr length (AS5300 store user phone)
const int DATA_LEN = 254;   // data field in tacacs+ length
const int IP_LEN = 16;    // ip addr len
const int SERVICE_LEN = 15;    // service len
const int PROTOCOL_LEN = 15;    // protocol len
const int TIMECHECK_LEN = 50;    // timecheck (cron style)
const int MAX_FROM = 32;    // rules number in from check
const int RESATTR_LEN = 32;    // ����� ��������� �������
const int RESVAL_LEN = 32;    // ����� char* �������� �������
const int CRYPTOKEY_LEN = 32;    // crypto key length

//#define UNKNOWN_IP      "0.0.0.0"

#include <cstdio>
#include <locale.h>
#include <pthread.h>
#include <iostream>
//using std::cout;
//using std::endl;
#include <string>

#include <fstream>
//#include <strstream>

#ifdef HAVE_SSTREAM
#include <sstream>
#else
#include <strstream>
#endif

#ifdef SOLARIS
  #include <strings.h>
  #include <netinet/in.h>
  #include <netinet/ip.h>
  #include <netinet/in_systm.h>
#endif

#ifdef WITH_XMLRPC
  #include <XmlRpc.h>
  using namespace XmlRpc;
#endif //WITH_XMLRPC

#include <vector>
#include <stack>
#include <list>
#include <map>

namespace tacpp {};       // do next string happy
using namespace tacpp;
//using namespace std;
using std::list;
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::less;
using std::map;
using std::ifstream;
using std::istringstream;

#include "ipaddr.h"

#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/time.h>
#include <cstdarg>
#include <netinet/in.h>

#ifdef FREEBSD
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#endif //FREEBSD

#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <regex.h>
#include <syslog.h>

#ifdef FREEBSD
#include <sys/types.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#endif //FREEBSD

//#ifndef USE_OPENSSL
//#include "md5.h"
//#else
#define MD5_LEN	16
#include <openssl/md5.h>
//#endif // USE_OPENSSL

#ifdef WITH_CCARD
#ifdef HAVE_DES_H
#include <des.h>
#else
#include <unistd.h>
#endif //HAVE_DES_H
#endif //WITH_CCARD

#include <expat.h>

#include "Errors.h"
#include "Utils.h"
#include "myQueue.h"
#include "Debug.h"
#include "Packet.h"
#include "ServerTrack.h"
#include "Logging.h"
#include "LogTerm.h"
#include "Vty.h"
#include "Command.h"
#include "Terminal.h"
#include "Cli.h"
#include "LogFile.h"
#include "LogSyslog.h"
#include "DLoad.h"
#include "db_module.h"
#include "NetFlow.h"
#include "TacDb.h"
#include "AAA.h"
#include "Tacacs.h"
#include "Tci.h"
#include "App.h"
#include "Listener.h"
#include "CoreData.h"
#include "CoreAccess.h"
#include "CorePool.h"
#include "CoreDb.h"
#include "CoreDebug.h"
#include "CoreManager.h"
#include "CoreDevice.h"
#include "CoreListener.h"
#include "CorePeer.h"
#include "CoreBundle.h"
#include "CoreBilling.h"
#include "Daemon.h"
#include "Icmp.h"
#include "Snmp.h"
#include "Http.h"
#include "XmlServer.h"
#include "UserTrack.h"
#include "AModify.h"
#include "Tiep.h"
#include "Radius.h"
#include "Diameter.h"
#include "GtpPrime.h"

typedef char* char_ptr;

namespace tacpp {
  extern CoreData *coreData;
  extern char MAINDIR[MAINDIR_LEN];
  extern Logging logSystem;
  extern LogFile *logFileSystem;
  extern LogSyslog *logSyslogSystem;
  extern LogTerm *logTermSystem;
  extern Snmp snmpSystem;
  extern UserTrack userTrack;
//extern DeviceTrack deviceTrack;
//extern pthread_mutex_t AbsMutex;
  extern ServerTrack serverTrack;
  extern NfAggregator nfAggregator;
};

#define TLOG      ::logSystem.log
#define TLOG2     ::logSystem
#define endlog    ::logSystem.Logging_end_endlog()
#define setlog(a) ::logSystem.Logging_set_setlog(a)
#define startlog(a) TLOG2<<setlog(a)

#endif //__global_H__
