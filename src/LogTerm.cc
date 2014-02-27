// logging to terminal
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"

//// class LogTermAtom

// constructor
LogTermAtom::LogTermAtom(Logging *l,int f) : Packet(f) {
  logging = l;
  output_id = logging->add();       // create output id
  fh = f;                           // store socket
}

// destructor
LogTermAtom::~LogTermAtom() {
  for(list<int>::iterator i=fc.begin();i!=fc.end() && !fc.empty();) {
    i = fc.erase(i);
  }
  fc.clear();
//  while(fc.size() > 0) fc.remove(); // delete whole list
  logging->del(output_id);          // del debug output
}

// write string
bool
LogTermAtom::twrite(char *str) {
  char s[300];
  bzero(s,sizeof(s));
  int j=0;
  for(int i=0; str[i] && j<300 && i<300; i++) {
    s[j++] = str[i];
    if(str[i] == '\n') s[j++] = '\r';
  }
  string sss;
  sss = sss + "% " + s + "\n\r";
  if(sockwrite((unsigned char *)sss.c_str(),sss.size(),TERMINAL_TIMEOUT) > 0)
    return true;
  return false;
}

// add debug facility
void
LogTermAtom::add_debug_fc(int n) {
  for(list<int>::const_iterator i=fc.begin(); i!=fc.end(); i++) {
    if(*i == n) return; // no double debug
  }
  fc.push_back(n);
}

// del debug facility
void
LogTermAtom::del_debug_fc(int n) {
  for(list<int>::iterator i=fc.begin(); i!=fc.end();) {
    if(*i == n) {
      i = fc.erase(i);
      continue;
    }
    i++;
  }
}

// write buffers to syslog server without delete log strings
void
LogTermAtom::logwrite() {
//time_t t = time(0);
//struct tm res;
//struct tm *tt=localtime_r(&t,&res);

/*
  LogList *tmpl;
  LogData *tmpd;
  tmpl = logging->getl(output_id);
*/

  ///// any case output //////
  // write errors - level LOG_ERROR - 0
//	for(int i=0; i < tmpl->logs.size(); i++) {
//		if(tmpl->logs.get(i)->get_level() == LOG_ERROR) {
//			tmpd = tmpl->logs.get(i);
//			twrite(tmpd->get_string());
//		}
//	}
	// write events - level LOG_EVENT - 253
//	for(int i=0; i < tmpl->logs.size(); i++) {
//		if(tmpl->logs.get(i)->get_level() == LOG_EVENT) {
//			tmpd = tmpl->logs.get(i);
//			twrite(tmpd->get_string());
//		}
//	}
	// write informational - level LOG_INFORM - 254
//	for(int i=0; i < tmpl->logs.size(); i++) {
//		if(tmpl->logs.get(i)->get_level() == LOG_INFORM) {
//			tmpd = tmpl->logs.get(i);
//			twrite(tmpd->get_string());
//		}
//	}

//	// write to log file - level LOG_LOGGING - 255
//	int logfd = 0;
//	for(int i=0; i < tmpl->logs.size(); i++) {
//		if(tmpl->logs.get(i)->get_level() == LOG_LOGGING) {
//			tmpd = tmpl->logs.get(i);
//			twrite(tmpd->get_string());
//		}
//	}

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
/*
  if(!tmpl) return;

  for(list<LogData*>::iterator i=tmpl->logs.begin();i!=tmpl->logs.end();i++) {
    tmpd = *i;
    for(list<int>::iterator j=fc.begin(); j!=fc.end(); j++) {
      if(tmpd->get_level() == *j) {
        twrite((char*)tmpd->get_string().c_str());
        break;
      }
    }
  }
*/
}

/////// class LogTerm

LogTerm::LogTerm(Logging *l) {
  logging = l;
  pthread_mutex_init(&Mutex,0);
}

LogTerm::~LogTerm() {
  LogTermAtom *tmp;
  lock();
  for(list<LogTermAtom*>::iterator i=la.begin();i!=la.end() && !la.empty();) {
    tmp = *i;
    i = la.erase(i);
    delete tmp;
  }
  la.clear();
  unlock();
  pthread_mutex_destroy(&Mutex);
}

void
LogTerm::lock() {
  pthread_mutex_lock(&Mutex);
}

void
LogTerm::unlock() {
  pthread_mutex_unlock(&Mutex);
}

// add sock to list
void
LogTerm::add(int f) {
  lock();
  LogTermAtom *tmp;
  // search for previously added servers (prevent duplications)
  for(list<LogTermAtom*>::const_iterator i=la.begin();i!=la.end();i++) {
    tmp = *i;
    if(tmp->fh == f) {
      unlock();
      return; // already here
    }
  }
  tmp = new LogTermAtom(logging,f);
  la.push_back(tmp);
  unlock();
}

// delete sock from list
void
LogTerm::del(int f) {
  lock();
  LogTermAtom *tmp;
  // search for all servers
  for(list<LogTermAtom*>::iterator i=la.begin();i!=la.end();) {
    tmp = *i;
    if(tmp->fh == f) {
      i = la.erase(i);
      delete tmp;
      unlock();
      return;
    }
    i++;
  }
  unlock();
}

// add debug facility to sock
void
LogTerm::add_debug_fc(int s,int f) {
  lock();
  LogTermAtom *tmp;
  // search for previously added servers
  for(list<LogTermAtom*>::const_iterator i=la.begin();i!=la.end();i++) {
    tmp = *i;
    if(tmp->fh == s) {
      tmp->add_debug_fc(f);
      unlock();
      return;
    }
  }
  unlock();
}

// delete debug facility from sock
void
LogTerm::del_debug_fc(int s,int f) {
  lock();
  LogTermAtom *tmp;
  // search for previously added servers
  for(list<LogTermAtom*>::const_iterator i=la.begin();i!=la.end();i++) {
    tmp = *i;
    if(tmp->fh == s) {
      tmp->del_debug_fc(f);
      unlock();
      return;
    }
  }
  unlock();
}

// write from buffer to all syslog servers
void
LogTerm::logwrite() {
  lock();
  LogTermAtom *tmp;
  // search for previously added servers
  if(la.size() > 0) {
    for(list<LogTermAtom*>::const_iterator i=la.begin();i!=la.end();i++) {
      tmp = *i;
      tmp->logwrite();
    }
  }
  unlock();
}

///////////////// that's all /////////////////////
