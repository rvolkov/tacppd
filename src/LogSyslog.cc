// output logs to syslog servers
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"

//// class LogSyslogAtom

// constructor
LogSyslogAtom::LogSyslogAtom(Logging *l,char *ip) {
  logging = l;
  syslogserv.fill(ip);
  output_id = logging->add(); // create output id
}

// destructor
LogSyslogAtom::~LogSyslogAtom() {
  for(list<int>::iterator i=fc.begin();i!=fc.end() && fc.size()>0;) {
    i = fc.erase(i);
  }
  fc.clear();
  logging->del(output_id); // del debug output
}

// add debug facility
void
LogSyslogAtom::add_debug_fc(int n) {
  int tmp;
  for(list<int>::const_iterator i=fc.begin();i!=fc.end();i++) {
    tmp = *i;
    if(tmp == n) return; // no double debug
  }
  fc.push_back(n);
}

// del debug facility
void
LogSyslogAtom::del_debug_fc(int n) {
  int tmp;
  for(list<int>::iterator i=fc.begin();i!=fc.end();) {
    tmp = *i;
    if(tmp == n) {
      i = fc.erase(i);
      continue;
    }
    i++;
  }
}

// write buffers to syslog server without delete log strings
void
LogSyslogAtom::logwrite() {
  // log file name
  //time_t t = time(0);
  //struct tm res;
  //struct tm *tt=localtime_r(&t,&res);

/*
  LogList *tmpl;
  LogData *tmpd;
  tmpl = logging->getl(output_id);

  ///// any case output //////
  // write errors - level LOG_ERROR - 0
  for(list<LogData*>::iterator i=tmpl->logs.begin();i!=tmpl->logs.end();i++) {
    tmpd = *i;
    if(tmpd->get_level() == LOG_ERROR) {
      if(syslogserv == "127.0.0.1")
        syslog(LOG_ERR,"TACPPD: %s",tmpd->get_string().c_str());
    }
  }
  // write events - level LOG_EVENT - 253
  for(list<LogData*>::iterator i=tmpl->logs.begin();i!=tmpl->logs.end();i++) {
    tmpd = *i;
    if(tmpd->get_level() == LOG_EVENT) {
      if(syslogserv == "127.0.0.1")
        syslog(LOG_NOTICE,"TACPPD: %s",tmpd->get_string().c_str());
    }
  }
  // write informational - level LOG_INFORM - 254
  for(list<LogData*>::iterator i=tmpl->logs.begin();i!=tmpl->logs.end();i++) {
    tmpd = *i;
    if(tmpd->get_level() == LOG_INFORM) {
      if(syslogserv == "127.0.0.1")
        syslog(LOG_INFO,"TACPPD: %s",tmpd->get_string().c_str());
    }
  }

// no log logging information - this is too many for syslog
//	// write to log file - level LOG_LOGGING - 255
//	int logfd = 0;
//	for(int i=0; i < tmpl->logs.size(); i++) {
//		if(tmpl->logs.get(i)->get_level() == LOG_LOGGING) {
//			tmpd = tmpl->logs.remove(i);
//			if(!logfd)
//				logfd = open(logf, O_CREAT | O_WRONLY | O_APPEND, 0600);
//			write(logfd, tmpd->get_string(), strlen(tmpd->get_string()));
//			write(logfd, "\n", 1);
//			delete tmpd;
//		}
//	}
//	if(logfd) close(logfd);

// no log netflow - too many for syslog
//	// write to netflow log file - level LOG_NETFLOWLOG - 252
//	int lognfd = 0;
//	for(int i=0; i < tmpl->logs.size(); i++) {
//		if(tmpl->logs.get(i)->get_level() == LOG_NETFLOWLOG) {
//			tmpd = tmpl->logs.remove(i);
//			if(!lognfd)
//				lognfd = open(logn, O_CREAT | O_WRONLY | O_APPEND, 0600);
//			write(lognfd, tmpd->get_string(), strlen(tmpd->get_string()));
//			write(lognfd, "\n", 1);
//			delete tmpd;
//		}
//	}
//	if(lognfd) close(lognfd);

  ///// selectable output
  // write to debug file - levels 1-252 from list
  //char nnn[5];// number of debug faci
  int tmpi;
  for(list<LogData*>::iterator i=tmpl->logs.begin();i!=tmpl->logs.end();i++) {
    tmpd = *i;
    for(list<int>::iterator j=fc.begin();j!=fc.end();j++) {
      tmpi = *j;
      if(tmpd->get_level() == tmpi) {
        if(syslogserv == "127.0.0.1")
          syslog(LOG_DEBUG,"TACPPD: %s",tmpd->get_string().c_str());
        break;
      }
    }
  }

*/

}


//// class LogSyslog

LogSyslog::LogSyslog(Logging *l) {
  logging = l;
  pthread_mutex_init(&Mutex,0);
}

LogSyslog::~LogSyslog() {
  LogSyslogAtom *tmp;
  for(list<LogSyslogAtom*>::iterator i=la.begin();i!=la.end() && !la.empty();) {
    tmp = *i;
    i = la.erase(i);
    delete tmp;
  }
  la.clear();
  pthread_mutex_destroy(&Mutex);
}

void
LogSyslog::lock() {
  pthread_mutex_lock(&Mutex);
}

void
LogSyslog::unlock() {
  pthread_mutex_unlock(&Mutex);
}

// add syslog server to list
void
LogSyslog::add(char *ip) {
  LogSyslogAtom *tmp;
  // search for previously added servers (prevent duplications)
  for(list<LogSyslogAtom*>::const_iterator i=la.begin();i!=la.end();i++) {
    tmp = *i;
    if(tmp->syslogserv == ip) return; // already here
  }
  tmp = new LogSyslogAtom(logging,ip);
  la.push_back(tmp);
}

// delete syslog server from list
void
LogSyslog::del(char *ip) {
  LogSyslogAtom *tmp;
  // search for all servers
  for(list<LogSyslogAtom*>::iterator i=la.begin();i!=la.end();i++) {
    tmp = *i;
    if(tmp->syslogserv == ip) {
      i = la.erase(i);
      delete tmp;
      return;
    }
  }
}

// add debug facility to syslog server
void
LogSyslog::add_debug_fc(char *ip,int f) {
  LogSyslogAtom *tmp;
  // search for previously added servers
  for(list<LogSyslogAtom*>::const_iterator i=la.begin();i!=la.end();i++) {
    tmp = *i;
    if(tmp->syslogserv == ip) {
      tmp->add_debug_fc(f);
      return;
    }
  }
}

// delete debug facility from syslog server
void
LogSyslog::del_debug_fc(char *ip,int f) {
  LogSyslogAtom *tmp;
  // search for previously added servers
  for(list<LogSyslogAtom*>::const_iterator i=la.begin();i!=la.end();i++) {
    tmp = *i;
    if(tmp->syslogserv == ip) {
      tmp->del_debug_fc(f);
      return;
    }
  }
}

// write from buffer to all syslog servers
void
LogSyslog::logwrite() {
  LogSyslogAtom *tmp;
  // search for previously added servers
  for(list<LogSyslogAtom*>::const_iterator i=la.begin();i!=la.end();i++) {
    tmp = *i;
    lock();
    tmp->logwrite();
    unlock();
  }
}

///////////////// that's all /////////////////////
