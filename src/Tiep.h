// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: Tiep.h
// description: tacppd interchange/exchange protocol

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

// all comments which start from /** - this is comments for KDoc */
//  for classes
//    @short [short description of the class]
//    @author [class author]
//    @version [class version]
//    @see [references to other classes or methods]
//  for methods
//    @see [references]
//    @return [sentence describing the return value]
//    @exception [list the exeptions that could be thrown]
//    @param [name id] [description]	- can be multiple

#ifndef __TIEP_H__
#define __TIEP_H__

#include "global.h"

/**
@short packet header for exchange between tacppds
*/
struct tiep_pak_hdr {
  unsigned int type;          // 4 bytes: packet type
  unsigned int varkey;        // 4 bytes: crypto variable key
  unsigned int entrynum;      // 4 bytes: number of entries in packet body
  unsigned int priority;      // 4 bytes: tacppd priority value
} __attribute__((__packed__));

/**
@short tiep packet handler
@author rv
@version 1
*/
//class TiepPacket : public md5 {
class TiepPacket {
  /**
  thread locker */
  pthread_mutex_t mutex_tpk_;
  /**
  set thread lock */
  void lock();
  /**
  remove thread lock */
  void unlock();
 public:
  /**
  init (constructor) */
  TiepPacket();
  /**
  destroy (destructor) */
  ~TiepPacket();
  /**
  create packet header
  @return ptr to filled tiep packet header */
  struct tiep_pak_hdr *createhdr(int,int,int);
  /**
  type of packet
  @return type of packet id number */
  int packet_type(char*);
  /**
  packet varkey
  @return random int as packet key */
  int packet_varkey(char*);
  /**
  number of entries
  @return int number of entries into packet */
  int packet_num(char*);
  /**
  packet priority
  @return packet priority value */
  int packet_pri(char*);
  /**
  create md5 hash */
  void create_md5_hash(unsigned char*,unsigned char*,unsigned char*);
  /**
  do xor with md5 hash string */
  void md5_xor(unsigned char*,unsigned char*,unsigned char*,int);
  /**
  encrypt tiep packet body
  @return ptr to encrypted packet body */
  char *encrypt(char*,int,char*,int);
  /**
  decrypt tiep packet body
  @return ptr to decrypted packet body */
  char *decrypt(char*,int,char*,int);
};

/////////////// TIP ////////////////

// devicetrack packet type id
#define TIP_DT_PACKET       1
// usertrack packet type id
#define TIP_UT_PACKET       2
// keepalive packet
#define TIP_KA_PACKET       3
// keepalive start packet
#define TIP_START_PACKET    4

// maximum entries into one TIP packet
#define TIP_MAX_ENT         7

/**
@short DeviceTrack packet body entry (in packet always only one entry)
*/
struct dt_pak_entry {
  /**
  4 bytes: device ip */
  unsigned int device;
  /**
  4 bytes: current device status - 1-up/0-down */
  unsigned int status;
  /**
  4 bytes: check number */
  unsigned int checknum;
} __attribute__((__packed__));

/**
@short UserTrack packet body entry (in packet several entryes)
*/
struct ut_pak_entry {
  /**
  32 bytes: username */
  char username[32];
  /**
  4 bytes: device ip */
  unsigned int device;
  /**
  4 bytes: user ip addr */
  unsigned int userip;
  /**
  4 bytes: check number */
  unsigned int checknum;
  /**
  32 bytes: device port name */
  char port[32];
  /**
  32 bytes: user source/destination phone */
  char phone[32];
  /**
  4 bytes: 1-up/0-down - user change status */
  unsigned int status;
} __attribute__((__packed__));

/**
@short tip packet handler
@author rv
@version 1
*/
class TipPacket : public TiepPacket {
  /**
  thread locker */
  pthread_mutex_t mutex_tp_;
  /**
  set thread lock */
  void lock();
  /**
  remove thread lock */
  void unlock();
 public:
  /**
  init (constructor) */
  TipPacket();
  /**
  clear (destructor) */
  ~TipPacket();
  /**
  send data to multicast address
  @return true if success */
  bool sendbuf(ipaddr,int,int,char*,int);
  /**
  devicetrack packet */
  struct dt_pak_entry *read_dt(char*,int,char*);
  /**
  usertrack packet */
  struct ut_pak_entry *read_ut(char*,int,char*);
  /**
  returns devicetrack whole packet length
  @return int packet len */
  int packetlen_dt(int);
  /**
  return usertrack whole packet length
  @return int packet len */
  int packetlen_ut(int);
};

/**
@short tip system
@author rv
@version 1
*/
class TipSystem : public TipPacket {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set thread lock */
  void lock();
  /**
  remove thread lock */
  void unlock();
  /**
  prepare device track list for tip */
  list<struct dt_pak_entry*> dt_pak;
  /**
  users track list for tip */
  list<struct ut_pak_entry*> ut_pak;
 public:
  /**
  init */
  TipSystem();
  /**
  destroy */
  ~TipSystem();
  /**
  clear device track list */
  void clear_dt();
  /**
  clear user track list */
  void clear_ut();
  /**
  add entry to device track sending packet list */
  void add_dt(ipaddr,bool);
  /**
  add entry to user track sending packet list */
  void add_ut(const string,ipaddr,ipaddr,const string,const string,bool);
  /**
  send devicetrack packet to all bundle numbers */
  void send_dt();
  /**
  send usertrack packet to all bundle */
  void send_ut();
  /**
  send keepalive packet */
  void send_ka();
  /**
  send keepalive start packet */
  void send_start();
};

///////////// TEP ///////////////

// devicetrack packet type id
#define TEP_DT_PACKET       6
// usertrack packet type id
#define TEP_UT_PACKET       7

/**
@short tep packet handler
@version 0
*/
class TepPacket : public TiepPacket {
 public:
  /**
  send data to other side */
  bool sendbuf(ipaddr,int,char*,int);
  /**
  devicetrack packet */
//  struct dt_pak_entry *read_dt(char*,int,char*);
  /**
  usertrack packet */
//  struct ut_pak_entry *read_ut(char*,int,char*);
  /**
  returns devicetrack whole packet length */
//  int packetlen_dt(int);
  /**
  return usertrack whole packet length */
//  int packetlen_ut(int);
};

/**
@short tep system
@version 0
*/
class TepSystem : public TepPacket {
 public:
};

#endif //__TIEP_H__
