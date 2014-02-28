// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: Tiep.cc
// description: tacppd interchange/exchange protocol

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"

//////////// common TIP/TEP things //////////

TiepPacket::TiepPacket() {
  pthread_mutex_init(&mutex_tpk_,0);
}

TiepPacket::~TiepPacket() {
  pthread_mutex_destroy(&mutex_tpk_);
}

void
TiepPacket::lock() {
  pthread_mutex_lock(&mutex_tpk_);
}

void
TiepPacket::unlock() {
  pthread_mutex_unlock(&mutex_tpk_);
}

// generate tip/tep header
struct tiep_pak_hdr*
TiepPacket::createhdr(int type,int entrynum,int pri) {
  lock();
  struct tiep_pak_hdr *ret;
  ret = new struct tiep_pak_hdr;
  ret->type = (unsigned int)htons(type);
  ret->entrynum = (unsigned int)htons(entrynum);
  // fill varkey from random digit generator
  ret->varkey = (unsigned int)htons(1 + (int)(32000.0 * rand()/(RAND_MAX + 1.0)));
  ret->priority = htons(pri);
//std::cerr<<"createhdr varkey="<<ret->varkey<<" type="<<type<<"/"<<(int)ret->type<<" num="<<entrynum<<"/"<<ret->entrynum<<std::endl;
  unlock();
  return ret;
}

// read tip/tep header and return packet type
int
TiepPacket::packet_type(char *buf) {
  lock();
  struct tiep_pak_hdr *hdr = (struct tiep_pak_hdr*)buf;
  int ret = (int)ntohs(hdr->type);
  unlock();
  return ret;
}

// packet varkey
int
TiepPacket::packet_varkey(char *buf) {
  lock();
  struct tiep_pak_hdr *hdr = (struct tiep_pak_hdr*)buf;
  int ret = (int)ntohs(hdr->varkey);
  unlock();
  return ret;
}

// number of entries
int
TiepPacket::packet_num(char *buf) {
  lock();
  struct tiep_pak_hdr *hdr = (struct tiep_pak_hdr*)buf;
  int ret = (int)ntohs(hdr->entrynum);
  unlock();
  return ret;
}

// priority
int
TiepPacket::packet_pri(char *buf) {
  lock();
  struct tiep_pak_hdr *hdr = (struct tiep_pak_hdr*)buf;
  int ret = (int)ntohs(hdr->priority);
  unlock();
  return ret;
}

// create_md5_hash(): create an md5 hash of the "the user's key"
// Write resulting hash into the array pointed to by "hash".
// The caller must allocate sufficient space for the resulting hash
// (which is 16 bytes long). The resulting hash can safely be used as
// input to another call to create_md5_hash, as its contents are copied
// before the new hash is generated.
void
TiepPacket::create_md5_hash(unsigned char *hash,unsigned char *k,unsigned char *vk) {
  unsigned char *md_stream, *mdp;
  int md_len=0;
  MD5_CTX mdcontext;

  md_len = strlen((char*)k)+strlen((char*)vk);
  md_stream = new unsigned char[md_len];
  mdp = md_stream;
  bcopy(k, mdp, strlen((char*)k));
  mdp += strlen((char*)k);
  bcopy(vk, mdp, strlen((char*)vk));
  mdp += strlen((char*)vk);

  MD5_Init(&mdcontext);
  MD5_Update(&mdcontext, md_stream, md_len);
  MD5_Final(hash, &mdcontext);

  delete [] md_stream;
  return;
}

// Overwrite input data with en/decrypted version by generating an
// MD5 hash and xor'ing data with it.
// When more than 16 bytes of hash is needed, the MD5 hash is performed
// again with the same values as before, but with the previous hash value
// appended to the MD5 input stream.
void
TiepPacket::md5_xor(unsigned char* data,unsigned char *k,unsigned char *vk,int data_len) {
  unsigned char hash[MD5_LEN]; // the md5 hash
  bzero(hash, MD5_LEN);
  create_md5_hash(hash,k,vk);
  for(int i = 0; i < data_len; i += MD5_LEN) {
    for (int j = 0; (j<MD5_LEN) && ((i+j)<data_len); j++) {
      data[i + j] ^= hash[j];
    }
  }
  return;
}

char*
TiepPacket::encrypt(char *b,int varkey,char *key,int len) {
  char vk[16];
  lock();
  char buf[len];
  bzero(buf,sizeof(buf));
  memcpy(buf,b,len);
  snprintf(vk,sizeof(vk)-1,"%d",varkey);
  if(!utl::scomp(key,"none")) {
    md5_xor((unsigned char*)buf,(unsigned char*)key,
      (unsigned char*)vk,len);
  }
  char *out = new char[len];
  memcpy(out,buf,len);
  unlock();
  return out;
}

char*
TiepPacket::decrypt(char *b,int varkey,char *key,int len) {
  lock();
  char vk[16];
  char buf[len];
  bzero(buf,sizeof(buf));
  memcpy(buf,b,len);
  snprintf(vk,sizeof(vk)-1,"%d",varkey);
  if(!utl::scomp(key,"none")) {
    md5_xor((unsigned char*)buf,(unsigned char*)key,
      (unsigned char*)vk,len);
  }
  char *out = new char[len];
  memcpy(out,buf,len);
  unlock();
  return out;
}

//////////////////// TIP ////////////////////

TipPacket::TipPacket() {
  pthread_mutex_init(&mutex_tp_,0);
}

TipPacket::~TipPacket() {
  pthread_mutex_destroy(&mutex_tp_);
}

void
TipPacket::lock() {
  pthread_mutex_lock(&mutex_tp_);
}

void
TipPacket::unlock() {
  pthread_mutex_unlock(&mutex_tp_);
}

// send multicast packet
bool
TipPacket::sendbuf(ipaddr ip,int port,int ttlval,char *bufx,int len) {
  if(!bufx) return false;
  socklen_t socketfd;
  socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(socketfd < 0) {
    startlog(LOG_ERROR)<<"TipPacket::sendbuf socket functions returned value < 0"<<endlog;
    return false;
  }
  socklen_t size_sin = sizeof(struct sockaddr);
  struct sockaddr_in TheirAddress;
  bzero((char*)&TheirAddress,sizeof(struct sockaddr)); //TheirAddress));
  TheirAddress.sin_family = AF_INET; // Denotes an internet address
  TheirAddress.sin_port = htons(port);
  bzero((char*)&TheirAddress.sin_addr,sizeof(struct in_addr));
  TheirAddress.sin_addr.s_addr = inet_addr(ip.get().c_str());
  unsigned char loop = 0;        // disable return to loopback
  setsockopt(socketfd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
  unsigned char ttl = (unsigned char)ttlval;    // multicast ttl value
  setsockopt(socketfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

// disable SIGPIPE
#ifdef SOLARIS
  void (*old_sigpipe)(int) = 0;
#else
  sig_t old_sigpipe = 0;
#endif
  old_sigpipe = signal (SIGPIPE, SIG_IGN);

// MSG_NOSIGNAL is not very widely available, so i disabled it.
// all it does it down of SIGPIPE during function calls apparently
// ignoring signal is more portable, but maybe should add configure
// test to see if MSG_NOSIGNAL is available as it is more correct

//  int sz = sendto(socketfd,bufx,len,MSG_NOSIGNAL,
//      (struct sockaddr*)&TheirAddress,size_sin);

  int sz = sendto(socketfd,bufx,len,0x0,
      (struct sockaddr*)&TheirAddress,size_sin);

// back the way it was..
  signal (SIGPIPE, old_sigpipe);

  close(socketfd);
  if(sz < 0) {
    startlog(LOG_TIP)<<"TIP: UDP sendto returned value <0"<<endlog;
    return false;
  }
  startlog(LOG_TIP)<<"Send multicast TIP packet"<<endlog;
  return true;
}

// devicetrack packet
struct dt_pak_entry*
TipPacket::read_dt(char *buf,int n,char *key) {
  lock();
  int num = packet_num(buf);
  int varkey = packet_varkey(buf);
  char *p = buf;
  p += sizeof(struct tiep_pak_hdr); // this is packet body
//std::cerr<<"read_dt num="<<num<<" varkey="<<varkey<<std::endl;
  struct dt_pak_entry *ret = 0;
  if(num > 0) {
    for(int i=0; i<num && i!=n; i++) {  // move to 'n' position
      p += sizeof(struct dt_pak_entry);
    }
    ret = new struct dt_pak_entry;
    //bzero(ret,sizeof(struct dt_pak_entry));
    char *rrr = decrypt((char*)p,varkey,key,
      sizeof(struct dt_pak_entry));
    memcpy((void*)ret,rrr,sizeof(struct dt_pak_entry));
    //memcpy((void*)ret,p,sizeof(struct dt_pak_entry));
    delete [] rrr;

    struct in_addr a;
    memcpy((void*)&a,(void*)&ret->device,sizeof(struct in_addr));
//std::cerr<<" read dt ip="<<inet_ntoa(a)<<" status="<<ret->status<<" varkey="<<varkey<<" check="<<ret->checknum<<" key="<<key<<std::endl;
    if((int)ret->checknum != varkey) { // wrong key
      startlog(LOG_TIP)<<"TipPacket::read_dt - wrong crypto key: checknum="<<ret->checknum<<" != varkey="<<packet_varkey(buf)<<endlog;
      // wrong crypto key
      delete ret;
      ret = 0;
    }
  }
  unlock();
  return ret;
}

// usertrack packet
struct ut_pak_entry*
TipPacket::read_ut(char *buf,int n,char *key) {
  lock();
  char *p = buf + sizeof(struct tiep_pak_hdr);
  //p += sizeof(struct tiep_pak_hdr);      // header
  int num = packet_num(buf);
  if(n > num) {
    unlock();
    return 0;                  // out of data
  }
  int varkey = packet_varkey(buf);
  for(int i=0; i<num && i!=n; i++) {
    p += sizeof(struct ut_pak_entry);
  }
  struct ut_pak_entry *ret = 0;
//std::cerr<<"Try do understand UT packet"<<std::endl;
  if(num > 0) {
    ret = new struct ut_pak_entry;
    bzero(ret,sizeof(struct ut_pak_entry));
    char *rrr = decrypt((char*)p,varkey,key,
        sizeof(struct ut_pak_entry));
    memcpy((void*)ret,rrr,sizeof(struct ut_pak_entry));
    delete [] rrr;
//std::cerr<<"read_ut - decrypt UT packet, varkey="<<varkey<<" chknum="<<ret->checknum<<" key="<<key<<std::endl;
    if((int)ret->checknum != varkey) {  // wrong key
//std::cerr<<"TipPacket::read_ut - chknum="<<ret->checknum<<" != "<<varkey<<" usern="<<ret->username<<std::endl;
      // wrong crypto key
      delete ret;
      ret = 0;
    } else {
//std::cerr<<"TipPacket::read_ut - compare chknum success"<<std::endl;
    }
  }
  unlock();
  return ret;
}

// returns devicetrack whole packet length
int
TipPacket::packetlen_dt(int num) {
  lock();
  int len = sizeof(struct tiep_pak_hdr) + (num * sizeof(struct dt_pak_entry));
  unlock();
  return len;
}

// return usertrack whole packet length
int
TipPacket::packetlen_ut(int num) {
  lock();
  int len = sizeof(struct tiep_pak_hdr) + num * sizeof(struct ut_pak_entry);
  unlock();
  return len;
}


////////// TipSystem

TipSystem::TipSystem() {
  pthread_mutex_init(&mutex_,0);
}

TipSystem::~TipSystem() {
  pthread_mutex_destroy(&mutex_);
}

void
TipSystem::lock() {
  pthread_mutex_lock(&mutex_);
}

void
TipSystem::unlock() {
  pthread_mutex_unlock(&mutex_);
}

// clear dt list
void
TipSystem::clear_dt() {
  lock();
  struct dt_pak_entry *dtmp;
  for(list<struct dt_pak_entry*>::iterator i=dt_pak.begin();
      i!=dt_pak.end() && !dt_pak.empty();) {
    dtmp = *i;
    i = dt_pak.erase(i);
    delete dtmp;
  }
  dt_pak.clear();
  unlock();
}

// clear ut list
void
TipSystem::clear_ut() {
  lock();
  struct ut_pak_entry *utmp;
  for(list<struct ut_pak_entry*>::iterator i=ut_pak.begin();
      i!=ut_pak.end() && !ut_pak.empty();) {
    utmp = *i;
    i = ut_pak.erase(i);
    delete utmp;
  }
  ut_pak.clear();
  unlock();
}

// add device information to packet creator
void
TipSystem::add_dt(ipaddr ip,bool s) {
  lock();
  struct dt_pak_entry *tmp = new struct dt_pak_entry;
  tmp->device = inet_addr(ip.get().c_str());
  if(s) tmp->status = 1;
  else tmp->status = 0;
  tmp->checknum = 0;        // must be equal varkey in header
  dt_pak.push_back(tmp);
  unlock();
}

// add user entry to packet creator
void
TipSystem::add_ut(const string u,ipaddr di,ipaddr ui,
                    const string p,const string t,bool s) {
  lock();
  struct ut_pak_entry *tmp = new struct ut_pak_entry;
  tmp->device = inet_addr(di.get().c_str());
  tmp->userip = inet_addr(ui.get().c_str());
  snprintf(tmp->username,sizeof(tmp->username)-1,"%s",u.c_str());
  snprintf(tmp->port,sizeof(tmp->port)-1,"%s",p.c_str());
  snprintf(tmp->phone,sizeof(tmp->phone)-1,"%s",t.c_str());
  if(s) tmp->status = 1;
  else tmp->status = 0;
  tmp->checknum = 0; // must be equal varkey in header
  ut_pak.push_back(tmp);
  unlock();
}

// send devicetrack packets to all
void
TipSystem::send_dt() {
  lock();
  if(dt_pak.size() == 0) {
    unlock();
    return; // nothing to send
  }
  struct dt_pak_entry *tmp;
  struct dt_pak_entry dttmp;
  if(!::coreData->getBundle()->isBundle()) {
    unlock();
    return; // no bundle
  }
  int len = 0;

  char key[32];
  snprintf(key,sizeof(key)-1,"%s",::coreData->getBundle()->getKey().c_str());

  int j = 0;
  list<struct dt_pak_entry*>::iterator it=dt_pak.begin();
  while(j < (int)dt_pak.size() && it!=dt_pak.end()) {
    char *buf;
    len = packetlen_dt(dt_pak.size());
//std::cerr<<"send_dt len="<<len<<std::endl;
    buf = new char[len+1];
    bzero(buf,sizeof(char)*(len+1));
    struct tiep_pak_hdr *hdr;
    if((dt_pak.size() - j) > TIP_MAX_ENT)
      hdr = createhdr(TIP_DT_PACKET,TIP_MAX_ENT,::coreData->getBundle()->getPri());
    else
      hdr = createhdr(TIP_DT_PACKET,(dt_pak.size()-j),::coreData->getBundle()->getPri());
    memcpy((void*)buf,(void*)hdr,sizeof(struct tiep_pak_hdr));
    int varkey = packet_varkey(buf);
    delete hdr;
    char *p = buf + sizeof(struct tiep_pak_hdr);      // packet body
    int k = 0;
    for(;j<(int)dt_pak.size() && k<TIP_MAX_ENT && it!=dt_pak.end(); j++,k++,it++) { // create packet body
      tmp = 0;
      tmp = *it;
      tmp->checknum = varkey;
      dttmp.checknum = varkey;
      dttmp.device = tmp->device;
      dttmp.status = tmp->status;
      struct in_addr a;
      memcpy((void*)&a,(void*)&dttmp.device,sizeof(struct in_addr));
//std::cerr<<"   prepare ip="<<inet_ntoa(a)<<" status="<<tmp->status<<" varkey="<<varkey<<" key="<<key<<std::endl;
      char *rrr = encrypt((char*)&dttmp,varkey,key,
        sizeof(struct dt_pak_entry));
      memcpy((void*)p,rrr,sizeof(struct dt_pak_entry));
      //memcpy((void*)p,&dttmp,sizeof(struct dt_pak_entry));
      delete [] rrr;
      p += sizeof(struct dt_pak_entry);      // move to new entry
    }
    if(k > 0) {
      sendbuf(::coreData->getBundle()->getMgroup(),
        ::coreData->getBundle()->getPort(),
          ::coreData->getBundle()->getTtl(),buf,packetlen_dt(k));// send packet
    }
    delete [] buf;
  }
  unlock();
}

// send usertrack packet
void
TipSystem::send_ut() {
  struct ut_pak_entry *tmp;
  struct ut_pak_entry utmp;
  if(!::coreData->getBundle()->isBundle()) {
    return; // no bundle created
  }
  lock();
  if(ut_pak.size() == 0) {
    unlock();
    return; // nothing to send
  }
  char key[32];
  snprintf(key,sizeof(key)-1,"%s",::coreData->getBundle()->getKey().c_str());
  int j = 0;
  int k = 0;
  int len = packetlen_ut(ut_pak.size());
  int lent = len + 1;
  char *bufp = new char[lent];
  struct tiep_pak_hdr *hdr = 0;
  char *p = 0;
  int varkey = 0;
  list<struct ut_pak_entry*>::iterator it=ut_pak.begin();
  while(j < (int)ut_pak.size() && it != ut_pak.end()) {
//    len = packetlen_ut(ut_pak.size());
//cerr<<"Tiep send_ut len="<<len<<endl;
//    bufp = new char[len + 1];
    bzero(bufp,(len+1)*sizeof(char));
    if((ut_pak.size() - j) > TIP_MAX_ENT) {
      hdr = createhdr(TIP_UT_PACKET,TIP_MAX_ENT,::coreData->getBundle()->getPri());
    } else {
      hdr = createhdr(TIP_UT_PACKET,(ut_pak.size() - j),::coreData->getBundle()->getPri());
    }
    memcpy(bufp,(void*)hdr,sizeof(struct tiep_pak_hdr));
    varkey = packet_varkey(bufp);
    delete hdr;
    p = bufp + sizeof(struct tiep_pak_hdr);
    k = 0;
    for(;j<(int)ut_pak.size() && k<TIP_MAX_ENT && it!=ut_pak.end(); j++,k++,it++) {
      tmp = *it;
      tmp->checknum = varkey;
      utmp.checknum = varkey;
      snprintf(utmp.username,sizeof(utmp.username)-1,"%s",tmp->username);
      utmp.device = tmp->device;
      utmp.userip = tmp->userip;
      snprintf(utmp.port,sizeof(utmp.port)-1,"%s",tmp->port);
      snprintf(utmp.phone,sizeof(utmp.phone)-1,"%s",tmp->phone);
      utmp.status = tmp->status;
      char *rrr = encrypt((char*)&utmp,varkey,key,sizeof(struct ut_pak_entry));
      memcpy((void*)p,rrr,sizeof(struct ut_pak_entry));
      delete [] rrr;
      p += sizeof(struct ut_pak_entry);
    }
    if(k > 0) {
      unlock();
      sendbuf(::coreData->getBundle()->getMgroup(),
        ::coreData->getBundle()->getPort(),
          ::coreData->getBundle()->getTtl(),bufp,packetlen_ut(k)); // send packet
      lock();
    }
  }
  delete [] bufp;
  unlock();
}

// send keepalive packet to all bundles
void
TipSystem::send_ka() {
  lock();
  char *buf;
  int len = packetlen_dt(0);
  buf = new char[len+1];
  bzero(buf,sizeof(char)*(len+1));
  struct tiep_pak_hdr *hdr;
  hdr = createhdr(TIP_KA_PACKET,0,::coreData->getBundle()->getPri());
  memcpy((void*)buf,(void*)hdr,sizeof(struct tiep_pak_hdr));
  delete hdr;
  unlock();
  sendbuf(::coreData->getBundle()->getMgroup(),
    ::coreData->getBundle()->getPort(),
      ::coreData->getBundle()->getTtl(),buf,len);    // send packet
  delete [] buf;
}

void
TipSystem::send_start() {
  lock();
  char *buf;
  int len = packetlen_dt(0);
  buf = new char[len+1];
  bzero(buf,sizeof(char)*(len+1));
  struct tiep_pak_hdr *hdr;
  hdr = createhdr(TIP_START_PACKET,0,::coreData->getBundle()->getPri());
  memcpy((void*)buf,(void*)hdr,sizeof(struct tiep_pak_hdr));
  //int varkey = packet_varkey(buf);
  delete hdr;
std::cerr<<"Send TIP start packet to "<<::coreData->getBundle()->getMgroup().get()<<std::endl;
  sendbuf(::coreData->getBundle()->getMgroup(),
    ::coreData->getBundle()->getPort(),
      ::coreData->getBundle()->getTtl(),buf,len);    // send packet
  delete [] buf;
  unlock();
}

///////////////////// TEP ////////////////////

// send udp packet
bool
TepPacket::sendbuf(ipaddr ip,int port,char *bufx,int len) {
  if(!bufx) return false;
  socklen_t socketfd;
  socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  socklen_t size_sin=sizeof(struct sockaddr);
  struct sockaddr_in TheirAddress;
  bzero((char*)&TheirAddress,sizeof(TheirAddress));
  TheirAddress.sin_family = AF_INET; // Denotes an internet address
  TheirAddress.sin_port = htons(port);
  //inet_aton(ip.get().c_str(),&TheirAddress.sin_addr); // set destination ip addr
  TheirAddress.sin_addr.s_addr = inet_addr(ip.get().c_str());

// disable SIGPIPE
#ifdef SOLARIS
  void (*old_sigpipe)(int) = 0;
#else
  sig_t old_sigpipe = 0;
#endif
  old_sigpipe = signal (SIGPIPE, SIG_IGN);

  int sz;
  sz = sendto(socketfd,bufx,len,0,
        (struct sockaddr*)&TheirAddress,size_sin);
//  int sz = sendto(socketfd,bufx,len,0x0,
//        (struct sockaddr*)&TheirAddress,size_sin);

// back the way it was..
//  signal (SIGPIPE, old_sigpipe);

  close(socketfd);

  if(sz < 0) {
    startlog(LOG_TEP)<<"TEP: UDP sendto returned value <0"<<endlog;
    return false;
  }
  return true;
}

///////////////// that is all //////////////////
