// log files output
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"

// class LogFile

// constructor
LogFile::LogFile(Logging *l,char *path) {
  pthread_mutex_init(&mutex_,0); // log file mutex
  lock();
  logging = l;
  snprintf(logplace,sizeof(logplace)-1,"%s",path);
  output_id = logging->add(); // create output id
  pthread_mutex_init(&mutex_error_,0);
  pthread_mutex_init(&mutex_event_,0);
  pthread_mutex_init(&mutex_inform_,0);
  pthread_mutex_init(&mutex_log_,0);
  pthread_mutex_init(&mutex_debug_,0);
  unlock();
  add_debug_fc(LOG_ERROR);
  add_debug_fc(LOG_EVENT);
  add_debug_fc(LOG_INFORM);
  add_debug_fc(LOG_LOGGING);
}

// destructor
LogFile::~LogFile() {
  lock();
  for(list<int>::iterator i=fc.begin();i!=fc.end() && !fc.empty();) {
    i = fc.erase(i);
  }
  fc.clear();
  logging->del(output_id); // del debug output
  pthread_mutex_destroy(&mutex_error_);
  pthread_mutex_destroy(&mutex_event_);
  pthread_mutex_destroy(&mutex_inform_);
  pthread_mutex_destroy(&mutex_log_);
  pthread_mutex_destroy(&mutex_debug_);
  unlock();
  pthread_mutex_destroy(&mutex_);
}

void
LogFile::lock() {
  pthread_mutex_lock(&mutex_);
}

void
LogFile::unlock() {
  pthread_mutex_unlock(&mutex_);
}

// add debug facility
void
LogFile::add_debug_fc(int n) {
  lock();
  for(list<int>::const_iterator i=fc.begin();i!=fc.end();i++) {
    if(*i == n) {
      unlock();
      return;
    }
  }
  fc.push_back(n);
  unlock();
}

// del debug facility
void
LogFile::del_debug_fc(int n) {
  lock();
  if(!fc.empty()) {
    for(list<int>::iterator i=fc.begin();i!=fc.end();) {
      if(*i == n) {
        i = fc.erase(i);
        continue;
      }
      i++;
    }
  }
  unlock();
}

// is such debug fc presented
bool
LogFile::is_debug_fc(int n) {
  lock();
  bool ret = false;
  for(list<int>::const_iterator i=fc.begin();i!=fc.end();i++) {
    if(*i == n) {
      ret = true;
      break;
    }
  }
  unlock();
  return ret;
}

// write logging buffers to file
void
LogFile::logwrite() {
  // log file name
  char logf[255]; // log file name
  time_t t = time(0);
  struct tm res;
  struct tm *tt = localtime_r(&t,&res);
  // tacacs+ log file name
  snprintf(logf,sizeof(logf)-1,"%s/%04d%02d%02d.log",logplace,
      1900+tt->tm_year,tt->tm_mon+1,tt->tm_mday);
  // events file name
  char eventf[255];
  snprintf(eventf,sizeof(eventf)-1,"%s/events%02d%02d.log",logplace,
    tt->tm_mon+1,tt->tm_mday);
  // errors file name
  char errorf[255];
  snprintf(errorf,sizeof(errorf)-1,"%s/errors%02d%02d.log",logplace,
    tt->tm_mon+1,tt->tm_mday);
  // informational file name
  char infof[255];
  snprintf(infof,sizeof(infof)-1,"%s/inform%02d%02d.log",logplace,
    tt->tm_mon+1,tt->tm_mday);
  // debug file name
  char debugf[255];
  snprintf(debugf,sizeof(debugf)-1,"%s/debug%02d%02d.log",logplace,
    tt->tm_mon+1,tt->tm_mday);

//std::cerr<<"ZZZ 1"<<std::endl;

  LogList *tmpl = 0;
  LogData *tmpd = 0;

  lock();
  tmpl = logging->getl(output_id);
//  unlock();

  ///// any case output //////
  int errorfd = -1;
  int eventfd = -1;
  int infofd = -1;
  int logfd = -1;
  int debugfd = -1;
  char nnn[5]; // number of debug faci

//std::cerr<<"ZZZ 2"<<std::endl;
//  lock();
  tmpd = 0;
  while((tmpd = tmpl->getEntry()) != 0) {
    string sss = tmpd->get_string();
    int ddd = tmpd->get_string().size();
    int level = tmpd->get_level();
//    int lsize = tmpl->size();
    delete tmpd;
    unlock();

    // write to errors file - level LOG_ERROR - 0
    if(level == LOG_ERROR) {
//      if(lsize < 200) {
        if(errorfd < 0) {
          pthread_mutex_lock(&mutex_error_);
          errorfd = open(errorf, O_CREAT | O_WRONLY | O_APPEND, 0600);
        }
        if(errorfd >= 0) {
          write(errorfd, sss.c_str(), ddd);
          write(errorfd, "\n", 1);
        } else {
          std::cerr<<"FATAL: can not open errors log file "<<errorf<<std::endl;
        }
//      }
      lock();
      continue;
    }

    // write to events file - level LOG_EVENT - 253
    if(level == LOG_EVENT) {
//      if(lsize < 200) { // write this only if size < 100
        if(eventfd < 0) {
          pthread_mutex_lock(&mutex_event_);
          eventfd = open(eventf, O_CREAT | O_WRONLY | O_APPEND, 0600);
        }
        if(eventfd >= 0) {
          write(eventfd, sss.c_str(), ddd);
          write(eventfd, "\n", 1);
        } else {
          std::cerr<<"FATAL: can not open events log file "<<eventf<<std::endl;
        }
//      }
      lock();
      continue;
    }

    // write to informational file - level LOG_INFORM - 254
    if(level == LOG_INFORM) {
//      if(lsize < 200) { // write this only if size < 100
        if(infofd < 0) {
          pthread_mutex_lock(&mutex_inform_);
          infofd = open(infof, O_CREAT | O_WRONLY | O_APPEND, 0600);
        }
        if(infofd >= 0) {
          write(infofd, sss.c_str(), ddd);
          write(infofd, "\n", 1);
        } else {
          std::cerr<<"FATAL: can not open info log file "<<infof<<std::endl;
        }
//      }
      lock();
      continue;
    }

    // write to log file - level LOG_LOGGING - 255
    if(level == LOG_LOGGING) {
      if(logfd < 0) { // logging MUST be outputed all!!!
        pthread_mutex_lock(&mutex_log_);
        logfd = open(logf, O_CREAT | O_WRONLY | O_APPEND, 0600);
      }
      if(logfd >= 0) {
        write(logfd, sss.c_str(), ddd);
        write(logfd, "\n", 1);
      } else {
        std::cerr<<"FATAL: can not open accounting log file "<<logf<<std::endl;
      }
      lock();
      continue;
    }
    // write to debug file - levels 1-252 from list
//    if(lsize < 100) { // output only if size < 100
      lock();
      for(list<int>::const_iterator j=fc.begin();j!=fc.end();j++) {
        if(level == *j) {
          snprintf(nnn,sizeof(nnn)-1,"%d ",*j);
          if(debugfd < 0) {
            pthread_mutex_lock(&mutex_debug_);
            debugfd = open(debugf, O_CREAT | O_WRONLY | O_APPEND, 0600);
          }
          if(debugfd >= 0) {
            write(debugfd, sss.c_str(), ddd);
            write(debugfd, "\n", 1);
          } else {
            std::cerr<<"FATAL: can not open debug log file "<<debugf<<std::endl;
          }
          break;
        }
      }
      unlock();
//    }
    lock();
  }
  if(errorfd >= 0) {
    close(errorfd);
    pthread_mutex_unlock(&mutex_error_);
  }
  if(eventfd >= 0) {
    close(eventfd);
    pthread_mutex_unlock(&mutex_event_);
  }
  if(infofd >= 0) {
    close(infofd);
    pthread_mutex_unlock(&mutex_inform_);
  }
  if(logfd >= 0) {
    close(logfd);
    pthread_mutex_unlock(&mutex_log_);
  }
  if(debugfd >= 0) {
    close(debugfd);
    pthread_mutex_unlock(&mutex_debug_);
  }
  unlock();
}

///////////////// that's all /////////////////////
