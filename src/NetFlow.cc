// netflow handle
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

//

#include "global.h"

//using std::pair;

/*
The NetFlow packet fragments structures used from NetFlow
examples on Cisco Systems, Inc. web site
*/



// process
void
NfAggregator::process(Bit32 ip,char *buf) {
  if(!buf) return;
  // 1. check packet version
  int version = 0;
  char bbb1 = buf[0];
  char bbb2 = buf[1];
  version = bbb1 << 8 | bbb2;
  // 2. get interesting header information
  NetFlowHeader_v1 *h1 = 0;
  NetFlowHeader_v5 *h5 = 0;
  NetFlowHeader_v7 *h7 = 0;
//NetFlowHeader_v8 *h8 = 0;

  // packets body
  NetFlow_v1 *f1 = 0;
  NetFlow_v5 *f5 = 0;
  NetFlow_v7 *f7 = 0;
  //NetFlow_v8 *f8;

  //char str[400];
  char srcaddr[16];
  char dstaddr[16];
  char nexthop[16];
  time_t sut;
  time_t bt;
  time_t et;
  switch(version) {
   case 1:
    // version 1
    // %s     ip
    // %d     version
    // %lu    ntohl(h1->SysUptime)
    // %lu    ntohl(h1->unix_secs)
    // %lu    ntohl(h1->unix_nsecs)
    // %lu    ntohl(f1->First)
    // %lu    ntohl(f1->Last)
    // %d     f1->prot
    // %s     srcaddr
    // %d     ntohs(f1->srcport)
    // %s     dstaddr
    // %d     ntohs(f1->dstport)
    // %s     nexthop
    // %d     ntohs(f1->input)
    // %d     ntohs(f1->output)
    // %ld    ntohl(f1->dPkts)
    // %ld    ntohl(f1->dOctets)
    h1 = (NetFlowHeader_v1*) buf;
    for(int i=0; i < ntohs(h1->count); i++) {
      f1 = (NetFlow_v1*)(buf+sizeof(NetFlowHeader_v1)+sizeof(NetFlow_v1) * i);
      sut = (ntohl(h1->unix_secs) - ntohl(h1->SysUptime)/1000);
      bt = sut + ntohl(f1->First)/1000;
      et = sut + ntohl(f1->Last)/1000;
//    ::nfAggregator.add(ip,srcaddr,dstaddr,(unsigned int)ntohs(f1->srcport),
//      (unsigned int)ntohs(f1->dstport),(int)f1->prot,ntohl(f1->dOctets),
//        bt,et);
      //::nfAggregator.
      add(ip,f1->srcaddr,f1->dstaddr,ntohl(f1->dOctets),
        ntohl(f1->dPkts),ntohs(f1->input),bt,et);
    }
    break;

  case 5:
    // version 5
    // %s      ip
    // %d      version
    // %lu     ntohl(h5->SysUptime)
    // %lu     ntohl(h5->unix_secs)
    // %lu     ntohl(h5->unix_nsecs)
    // %lu     ntohl(f5->First)
    // %lu     ntohl(f5->Last)
    // %d      f5->prot
    // %s      srcaddr
    // %d      ntohs(f5->srcport)
    // %s      dstaddr
    // %d      ntohs(f5->dstport)
    // %s      nexthop
    // %d      ntohs(f5->input)
    // %d      ntohs(f5->output)
    // %ld     ntohl(f5->dPkts)
    // %ld     ntohl(f5->dOctets)
    // %d      ntohs(f5->src_as)
    // %d      ntohs(f5->dst_as)
    // %d      f5->src_mask
    // %d      f5->dst_mask
    // %d      f5->tos
    // %ld     ntohl(h5->flow_sequence)
    // %d      h5->engine_type
    // %d      h5->engine_id

    h5 = (NetFlowHeader_v5*) buf;
    for(int i=0; i < ntohs(h5->count); i++) {
      f5 = (NetFlow_v5*)(buf+sizeof(NetFlowHeader_v5)+sizeof(NetFlow_v5)*i);
      snprintf(srcaddr,sizeof(srcaddr),"%s",utl::sip(f5->srcaddr).c_str());
      snprintf(dstaddr,sizeof(dstaddr),"%s",utl::sip(f5->dstaddr).c_str());
      snprintf(nexthop,sizeof(nexthop),"%s",utl::sip(f5->nexthop).c_str());

      sut = (ntohl(h5->unix_secs) - ntohl(h5->SysUptime)/1000);
      bt = sut + ntohl(f5->First)/1000;
      et = sut + ntohl(f5->Last)/1000;

      //::nfAggregator.
      add(ip,f5->srcaddr,f5->dstaddr,ntohl(f5->dOctets),
        ntohl(f5->dPkts),ntohs(f5->input),bt,et);
    }
    break;

  case 7:			// netflow version 7
    h7 = (NetFlowHeader_v7*) buf;
    for(int i=0; i < ntohs(h7->count); i++) {
      f7 = (NetFlow_v7*)(buf+sizeof(NetFlowHeader_v7)+sizeof(NetFlow_v7) * i);
      snprintf(srcaddr,sizeof(srcaddr),"%s",utl::sip(f7->srcaddr).c_str());
      snprintf(dstaddr,sizeof(dstaddr),"%s",utl::sip(f7->dstaddr).c_str());
      snprintf(nexthop,sizeof(nexthop),"%s",utl::sip(f7->nexthop).c_str());

      sut = (ntohl(h7->unix_secs) - ntohl(h7->SysUptime)/1000);
      bt = sut + ntohl(f7->First)/1000;
      et = sut + ntohl(f7->Last)/1000;

      //::nfAggregator.
      add(ip,f7->srcaddr,f7->dstaddr,ntohl(f7->dOctets),
        ntohl(f7->dPkts),ntohs(f7->input),bt,et);
    }
    break;
/*
  case 8:			// netflow version 8
    h8 = (NetFlowHeader_v8*) buf;
    for(int i=0; i < ntohs(h8->count); i++) {
      f8 = (NetFlow_v8*)(buf + sizeof(NetFlowHeader_v8) +
        sizeof(NetFlow_v8) * i);
      FlowInfo *tmp = new FlowInfo;

		//// filling information from NetFlow packet ////
		// from header
		tmp->version = version;
		tmp->SysUptime = ntohl(h8->SysUptime);
		tmp->unix_secs = ntohl(h8->unix_secs);
		tmp->unix_nsecs = ntohl(h8->unix_nsecs);
		// v5 & v7 & v8
		tmp->flow_sequence = ntohl(h8->flow_sequence);
		// v5 & v8
		tmp->engine_type = h8->engine_type;
		tmp->engine_id = h8->engine_id;
		// v8
		tmp->aggregation = h8->aggregation;
		tmp->agg_version = h8>agg_version;
	        // information from body
		// all versions
	        tmp->First = ntohl(f8->First);
		tmp->Last = ntohl(f8->Last);
		memcpy(&tmp->srcaddr, &f8->srcaddr, IP4_ADDR_LEN);
		memcpy(&tmp->dstaddr, &f8->dstaddr, IP4_ADDR_LEN);
		memcpy(&tmp->nexthop, &f8->nexthop, IP4_ADDR_LEN);
		tmp->input = ntohs(f8->input);
		tmp->output = ntohs(f8->output);
		memcpy(&tmp->srcport, &f8->srcport, TRANS_ADDR_LEN);
		memcpy(&tmp->dstport, &f8->dstport, TRANS_ADDR_LEN);
		tmp->prot = f8->prot;
		tmp->tos = f8->tos;
		tmp->tcp_flags = f8->tcp_flags;
		tmp->dPkts = ntohl(f8->dPkts);
		tmp->dOctets = ntohl(f8->dOctets);
		// v5 & v7
		tmp->dst_as = 0;
		//memcpy(&tmp->dst_as, &f8->dst_as, AS_ADDR_LEN);
		tmp->src_as = 0;
		//memcpy(&tmp->src_as, &f8->src_as, AS_ADDR_LEN);
		tmp->dst_mask = 0; // f8->dst_mask;
		tmp->src_mask = 0; // f8->src_mask;
	        // v7
		tmp->flags = 0;
		//tmp->flags = f8->flags;
		tmp->router_sc = 0;
		//memcpy(&tmp->router_sc, &f8->router_sc, IP4_ADDR_LEN);
		// v8 aggregation (several types)
		memcpy(&tmp->dst_prefix, &f8->dst_prefix, IP4_ADDR_LEN);
		memcpy(&tmp->src_prefix, &f8->src_prefix, IP4_ADDR_LEN);
		////// that is all

	    	//data.add(tmp);			// store it all
	    }
	    break;
*/

/* 
 * case 9:					// netflow version 9
 * break;
 */
 
  default:
    startlog(LOG_NETFLOW)<<"Receive packet with unsupported version = "<<version<<endlog;
    break;
  }
  return;
}

//////// NfData

NfData::NfData(unsigned long o,unsigned long pk,unsigned int ii,time_t ft,time_t lt) {
  pthread_mutex_init(&mutex_,0);
  lock();
  octets_ = o;
  packets_ = pk;
  ifin_ = ii;
  startt_ = ft;
  stopt_ = lt;
  unlock();
}

NfData::~NfData() {
  pthread_mutex_destroy(&mutex_);
}

void
NfData::lock() {
  pthread_mutex_lock(&mutex_);
}

void
NfData::unlock() {
  pthread_mutex_unlock(&mutex_);
}

void
NfData::add(unsigned long r,unsigned long p,time_t t) {
  lock();
  octets_ += r;
  stopt_ = t;
  packets_ += p;
  unlock();
}

bool
NfData::isExpired(time_t expire_time) {
  // count difference from first flow start and last flow stop times
  time_t r = stopt_ - startt_;
  if(r > expire_time) return true;

  // difference from current server time and last flow stop
  r = time(0) - stopt_;
  if(r > expire_time) return true;

  // difference from current server time and first flow start
  r = time(0) - startt_;
  if(r > (expire_time * 2)) return true;

  return false;
}

unsigned int
NfData::getIfIn() {
  return ifin_;
}

time_t
NfData::getStart() {
  return startt_;
}

time_t
NfData::getStop() {
  return stopt_;
}

unsigned long
NfData::getOctets() {
  return octets_;
}

unsigned long
NfData::getPackets() {
  return packets_;
}

//////// NfAggregator
extern void nfAggregatorProcess();

NfAggregator::NfAggregator() {
  writeProcess_ = false;
  expireTime_ = 900;
  buf_size_ = 0;
  pthread_mutex_init(&mutex_,0);
}

void
NfAggregator::runProcess() {
  pthread_create(&thrvar_,NULL,(void*(*)(void*))&nfAggregatorProcess,(void*)NULL);
  pthread_detach(thrvar_);
}

NfAggregator::~NfAggregator() {
  lock();
  NfData *tmp;
  map<Bit32,map<Bit32,map<Bit32,NfData*> > >::iterator p1;
  map<Bit32,map<Bit32,NfData*> >::iterator p2;
  map<Bit32,NfData*>::iterator p3;

  for(p1=nfb_.begin(); p1!=nfb_.end();) {
    for(p2=(p1->second).begin(); p2!=(p1->second).end();) {
      for(p3=(p2->second).begin(); p3!=(p2->second).end();) {
        tmp = (p3->second);
        (p2->second).erase(p3);
        p3 = (p2->second).begin();
        delete tmp;
      }
      (p1->second).erase(p2);
      p2=(p1->second).begin();
    }
    nfb_.erase(p1);
    p1=nfb_.begin();
  }
  unlock();
  pthread_mutex_destroy(&mutex_);
}

void
NfAggregator::lock() {
  pthread_mutex_lock(&mutex_);
}

void
NfAggregator::unlock() {
  pthread_mutex_unlock(&mutex_);
}

int
NfAggregator::size() {
  lock();
  int nfdsize = buf_size_;
  unlock();
  return nfdsize;
}

int
NfAggregator::nodesSize() {
  lock();
  int ret = nfb_.size();
  unlock();
  return ret;
}

int
NfAggregator::sourceSize() {
  int nfdsize = 0;
  map<Bit32,map<Bit32,map<Bit32,NfData*> > >::iterator p1;
  lock();
  for(p1=nfb_.begin(); p1!=nfb_.end();p1++) {
    nfdsize += (p1->second).size();
  }
  unlock();
  return nfdsize;
}

void
NfAggregator::add(Bit32 d,Bit32 sa,Bit32 da,unsigned long o,unsigned long pk,
      unsigned int ii,time_t bt,time_t et) {
  NfData *tmp;
  map<Bit32,map<Bit32,map<Bit32,NfData*> > >::iterator p1;
  map<Bit32,map<Bit32,NfData*> >::iterator p2;
  map<Bit32,NfData*>::iterator p3;
  p1 = nfb_.find(d); // device
  if(p1 == nfb_.end()) {
    lock();
    buf_size_++;
    map<Bit32,map<Bit32,NfData*> > aaa;
    map<Bit32,NfData*> bbb;
    tmp = new NfData(o,pk,ii,bt,et);
    bbb.insert(pair<Bit32,NfData*>(da,tmp));
    aaa.insert(pair<Bit32,map<Bit32,NfData*> >(sa,bbb));
    nfb_.insert(pair<Bit32, map<Bit32,map<Bit32,NfData*> > >(d,aaa));
    unlock();
    return;
  }
  p2 = (p1->second).find(sa);
  if(p2 == (p1->second).end()) {
    lock();
    buf_size_++;
    map<Bit32,NfData*> aaa;
    tmp = new NfData(o,pk,ii,bt,et);
    aaa.insert(pair<Bit32,NfData*>(da,tmp));
    (p1->second).insert(pair<Bit32,map<Bit32,NfData*> >(sa,aaa));
    unlock();
    return;
  }
  p3 = (p2->second).find(da);
  if(p3 == (p2->second).end()) {
    lock();
    buf_size_++;
    tmp = new NfData(o,pk,ii,bt,et);
    (p2->second).insert(pair<Bit32,NfData*>(da,tmp));
    unlock();
    return;
  }
  // find such dest addr
  lock();
  (p3->second)->add(o,pk,et); // add data to created flow
  unlock();
}

bool
NfAggregator::getWriteProcess() {
  return writeProcess_;
}

void
NfAggregator::setWriteProcess(bool p) {
  writeProcess_ = p;
}

void
NfAggregator::expire() {
  time_t t;
  // data for file output
  char spath[150];
  int lognfd = -1;
//  char strd[60];
  // create log/debug system
  snprintf(spath,sizeof(spath)-1,"%s/log",::MAINDIR);
  char logn[255]; // netflow log file name
  bzero(logn,sizeof(logn));
  struct tm res;
  struct tm *tt;
  string out;
  int closer = 0;
  NfData *tmp;
  int cid = 0;
  bool db_opened = false;

  map<Bit32,map<Bit32,map<Bit32,NfData*> > >::iterator p1;
  map<Bit32,map<Bit32,NfData*> >::iterator p2;
  map<Bit32,NfData*>::iterator p3;
  map<Bit32,NfData*>::iterator tmp3;

  bool expa_;

  while(true) {
    for(p1=nfb_.begin(); p1!=nfb_.end() && !nfb_.empty();p1++) {
      if((p1->second).size() == 0) {
        lock();
        nfb_.erase(p1);
        p1 = nfb_.begin();
        unlock();
        continue;
      }
      for(p2=(p1->second).begin();p2!=(p1->second).end();p2++) {
        if((p2->second).size() == 0) {
          lock();
          (p1->second).erase(p2);
          p2 = (p1->second).begin();
          unlock();
          continue;
        }
        for(p3=(p2->second).begin();p3!=(p2->second).end();) {
          lock();
          tmp = (p3->second);
          if(!tmp) {
            p3++;
            unlock();
            continue;
          }
          expa_ = tmp->isExpired(expireTime_);
          unlock();
          if(expa_) {
            lock();
            Bit32 deva = (p1->first);
            Bit32 srca = (p2->first);
            Bit32 dsta = (p3->first);
            time_t startt = tmp->getStart();
            time_t stopt = tmp->getStop();
            unsigned long octets = tmp->getOctets();
            unsigned long packets = tmp->getPackets();
            int ifin = tmp->getIfIn();
            tmp3 = p3;
            p3++;
            (p2->second).erase(tmp3);
            buf_size_--;
            delete tmp;
            unlock();

            if(!db_opened) {
              cid = ::coreData->getDb()->open_nf();
              db_opened = true;
              if(cid == 0) {
                db_opened = false;
                // can't open database for flow info writing, write it all to file
                char logntmp[100];
                snprintf(logntmp,sizeof(logntmp)-1,"%s",logn);
                t = time(0);
                tt = localtime_r(&t,&res);
                snprintf(logn,sizeof(logn)-1,"%s/nf%04d%02d%02d.log",spath,
                    1900+tt->tm_year,tt->tm_mon+1,tt->tm_mday);
                if(strcmp(logntmp,logn) != 0) {
std::cerr<<"Close log 1 ("<<logntmp<<"!="<<logn<<")"<<std::endl;
                  if(lognfd >= 0) close(lognfd);
                  lognfd = -1;
                  closer = 0;
                }
                if(lognfd < 0) {
                  lognfd = open(logn, O_CREAT | O_WRONLY | O_APPEND, 0600);
//std::cerr<<"Open logfile"<<std::endl;
                  if(!lognfd) {
                    startlog(LOG_ERROR)<<"Cant open redudancy netflow log file"<<endlog;
                  }
                }
                if(lognfd >= 0) {
                  out = "";
                  // +strd+
                  out = out+utl::sip(deva)+" "+utl::toString(startt,0)+" "+
                    utl::toString(stopt,0)+" "+utl::sip(srca)+" "+
                      utl::sip(dsta)+" "+utl::toString(octets)+" "+
                        utl::toString(packets)+" "+utl::toString(ifin);
                  write(lognfd, out.c_str(), out.size());
                  write(lognfd, "\n", 1);
                }
              }
            }
            if(db_opened) {
              ::coreData->getDb()->add_nf(cid,deva,srca,dsta,startt,stopt,octets,packets,ifin);
            }
            // system for close files and database connections
            // after 2000 entryes (it will be opened again)
            closer++;
            if(closer > 2000) {
              if(db_opened) ::coreData->getDb()->close_nf(cid);
              cid = 0;
              db_opened = false;
std::cerr<<"Close log 2"<<std::endl;
              if(lognfd >= 0) close(lognfd);
              lognfd = -1;
              closer = 0;
            }
          } else {
            lock();
            p3++;
            unlock();
          }
        }
        int aaa = size();
        // this constants entered manually as test results
        if(aaa < 5000) usleep(50);
        if(aaa < 10000) usleep(20);
        if(aaa < 15000) usleep(10);
        if(aaa < 20000) usleep(5);
        if(aaa < 29000) usleep(1);
      }
      int ccc = size();
      // this constants entered manually as test results
      if(ccc < 20000) usleep(5);
      if(ccc < 29000) usleep(1);
    }
    int bbb = size();
    // this constants entered manually as test results
    if(bbb < 20000) usleep(5);
    if(bbb < 29000) usleep(1);
  }
}

int
NfAggregator::expireTime() {
  return expireTime_;
}

void
NfAggregator::addExpireTime() {
  if(expireTime_ < 900) expireTime_++;
}

void
NfAggregator::delExpireTime() {
  if(expireTime_ > 180) expireTime_--;
}

void
nfAggregatorProcess() {
//  sleep(10); // wait some for init other parts
  ::nfAggregator.expire();  // expire netflow aggregator buffer
  pthread_exit(0);	// we have to correctly end thread
}

///////////// this is all /////////////
