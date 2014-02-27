// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: Tacacs.h
// description: tacacs+ low-level api

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

/*
This software have parts of code from Cisco's tac_plus daemon
with next copyright notice:

  Copyright (c) 1995-2000 by Cisco systems, Inc.

  Permission to use, copy, modify, and distribute modified and
  unmodified copies of this software for any purpose and without fee is
  hereby granted, provided that (a) this copyright and permission notice
  appear on all copies of the software and supporting documentation, (b)
  the name of Cisco Systems, Inc. not be used in advertising or
  publicity pertaining to distribution of the program without specific
  prior permission, and (c) notice be given in supporting documentation
  that use, modification, copying and distribution is by permission of
  Cisco Systems, Inc.

  Cisco Systems, Inc. makes no representations about the suitability
  of this software for any purpose.  THIS SOFTWARE IS PROVIDED ``AS
  IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
  WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
  FITNESS FOR A PARTICULAR PURPOSE.
*/

// modifications:
//

// all comments which start from /**<CR> this is comment for KDoc */
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

#ifndef __TACACS_H__
#define __TACACS_H__

#include "global.h"

namespace tacpp {

//////////////////////////////////////////////////////////////////

#define TAC_PLUS_MAX_PACKET   512 //256   // we do not handle bigger packets

#define ECHO              0     // echo enable (?)
#define NOECHO            1     // echo disable (?)

#define MSCHAP_DIGEST_LEN     49        // digest length

#define TAC_PLUS_READ_TIMEOUT     256     // seconds
#define TAC_PLUS_WRITE_TIMEOUT    20      // seconds

#define NAS_PORT_MAX_LEN          255     // port name length

// types of authentication
#define TACACS_ENABLE_REQUEST   1     // Enable Requests
#define TACACS_ASCII_LOGIN      2     // Inbound ASCII Login
#define TACACS_PAP_LOGIN        3     // Inbound PAP Login
#define TACACS_CHAP_LOGIN       4     // Inbound CHAP login
#define TACACS_ARAP_LOGIN       5     // Inbound ARAP login
#define TACACS_PAP_OUT          6     // Outbound PAP request
#define TACACS_CHAP_OUT         7     // Outbound CHAP request
#define TACACS_ASCII_ARAP_OUT   8     // Outbound ASCII and ARAP request
#define TACACS_ASCII_CHPASS     9     // ASCII change password request
#define TACACS_PPP_CHPASS       10    // PPP change password request
#define TACACS_ARAP_CHPASS      11    // ARAP change password request
#define TACACS_MSCHAP_LOGIN     12    // MS-CHAP inbound login
#define TACACS_MSCHAP_OUT       13    // MS-CHAP outbound login
#define TACACS_CHAP_PASSWORD    14    // request for CHAP cleartext passwd
#define TACACS_PAP_PASSWORD     15    // request for PAP password

// services
#define TAC_PLUS_AUTHEN_LOGIN       1
#define TAC_PLUS_AUTHEN_CHPASS      2
#define TAC_PLUS_AUTHEN_SENDPASS    3   // deprecated (no support here)
#define TAC_PLUS_AUTHEN_SENDAUTH    4

// status of reply packet, that client get from server in authen
#define TAC_PLUS_AUTHEN_STATUS_PASS       1
#define TAC_PLUS_AUTHEN_STATUS_FAIL       2
#define TAC_PLUS_AUTHEN_STATUS_GETDATA    3
#define TAC_PLUS_AUTHEN_STATUS_GETUSER    4
#define TAC_PLUS_AUTHEN_STATUS_GETPASS    5
#define TAC_PLUS_AUTHEN_STATUS_RESTART    6
#define TAC_PLUS_AUTHEN_STATUS_ERROR      7
#define TAC_PLUS_AUTHEN_STATUS_FOLLOW     0x21

// methods of authorization
#define TAC_PLUS_AUTHEN_METH_NOT_SET      0     // 0x00
#define TAC_PLUS_AUTHEN_METH_NONE         1     // 0x01
#define TAC_PLUS_AUTHEN_METH_KRB5         2     // 0x03
#define TAC_PLUS_AUTHEN_METH_LINE         3     // 0x03
#define TAC_PLUS_AUTHEN_METH_ENABLE       4     // 0x04
#define TAC_PLUS_AUTHEN_METH_LOCAL        5     // 0x05
#define TAC_PLUS_AUTHEN_METH_TACACSPLUS   6     // 0x06   use this?
#define TAC_PLUS_AUTHEN_METH_GUEST        8     // 0x08
#define TAC_PLUS_AUTHEN_METH_RADIUS       16    // 0x10
#define TAC_PLUS_AUTHEN_METH_KRB4         17    // 0x11
#define TAC_PLUS_AUTHEN_METH_RCMD         32    // 0x20

// priv_levels (don't know why only 4?)
#define TAC_PLUS_PRIV_LVL_MAX             15    // 0x0f
#define TAC_PLUS_PRIV_LVL_ROOT            15    // 0x0f
#define TAC_PLUS_PRIV_LVL_USER            1     // 0x01
#define TAC_PLUS_PRIV_LVL_MIN             0     // 0x00

// authen types
#define TAC_PLUS_AUTHEN_TYPE_ASCII    1   // 0x01      ascii
#define TAC_PLUS_AUTHEN_TYPE_PAP      2   // 0x02      pap
#define TAC_PLUS_AUTHEN_TYPE_CHAP     3   // 0x03      chap
#define TAC_PLUS_AUTHEN_TYPE_ARAP     4   // 0x04      arap
#define TAC_PLUS_AUTHEN_TYPE_MSCHAP   5   // 0x05      mschap

// authen services
#define TAC_PLUS_AUTHEN_SVC_NONE      0   // 0x00
#define TAC_PLUS_AUTHEN_SVC_LOGIN     1   // 0x01
#define TAC_PLUS_AUTHEN_SVC_ENABLE    2   // 0x02
#define TAC_PLUS_AUTHEN_SVC_PPP       3   // 0x03
#define TAC_PLUS_AUTHEN_SVC_ARAP      4   // 0x04
#define TAC_PLUS_AUTHEN_SVC_PT        5   // 0x05
#define TAC_PLUS_AUTHEN_SVC_RCMD      6   // 0x06
#define TAC_PLUS_AUTHEN_SVC_X25       7   // 0x07
#define TAC_PLUS_AUTHEN_SVC_NASI      8   // 0x08
#define TAC_PLUS_AUTHEN_SVC_FWPROXY   9   // 0x09
#define TAC_PLUS_AUTHEN_SVC_H323_VSA  10  // 0x0A

// authorization status
#define TAC_PLUS_AUTHOR_STATUS_PASS_ADD   1     // 0x01
#define TAC_PLUS_AUTHOR_STATUS_PASS_REPL  2     // 0x02
#define TAC_PLUS_AUTHOR_STATUS_FAIL       16    // 0x10
#define TAC_PLUS_AUTHOR_STATUS_ERROR      17    // 0x11
#define TAC_PLUS_AUTHOR_STATUS_FOLLOW     33    // 0x21

// accounting flag
#define TAC_PLUS_ACCT_FLAG_MORE       0x1   // deprecated
#define TAC_PLUS_ACCT_FLAG_START      0x2
#define TAC_PLUS_ACCT_FLAG_STOP       0x4
#define TAC_PLUS_ACCT_FLAG_WATCHDOG   0x8

// accounting status
#define TAC_PLUS_ACCT_STATUS_SUCCESS    1   // 0x01
#define TAC_PLUS_ACCT_STATUS_ERROR      2   // 0x02
#define TAC_PLUS_ACCT_STATUS_FOLLOW     33  // 0x21

// versions
#define TAC_PLUS_MAJOR_VER_MASK     0xf0
#define TAC_PLUS_MAJOR_VER          0xc0
#define TAC_PLUS_MINOR_VER_0        0x0
#define TAC_PLUS_VER_0    (TAC_PLUS_MAJOR_VER | TAC_PLUS_MINOR_VER_0)
#define TAC_PLUS_MINOR_VER_1        0x01
#define TAC_PLUS_VER_1    (TAC_PLUS_MAJOR_VER | TAC_PLUS_MINOR_VER_1)

// type of packets
#define TAC_PLUS_AUTHEN         1     // authentication
#define TAC_PLUS_AUTHOR         2     // authorization
#define TAC_PLUS_ACCT           3     // accounting

// crypto flag
#define TAC_PLUS_ENCRYPTED    0x0   // packet is encrypted
#define TAC_PLUS_CLEAR        0x1   // packet is not encrypted

// hash
#define HASH_TAB_SIZE         157   // user and group hash table sizes

// type of request
#define TAC_PLUS_AUTHEN_TYPE_ASCII    1
#define TAC_PLUS_AUTHEN_TYPE_PAP      2
#define TAC_PLUS_AUTHEN_TYPE_CHAP     3
#define TAC_PLUS_AUTHEN_TYPE_ARAP     4
#define TAC_PLUS_AUTHEN_TYPE_MSCHAP   5

// service of request
#define TAC_PLUS_AUTHEN_SVC_LOGIN     1
#define TAC_PLUS_AUTHEN_SVC_ENABLE    2
#define TAC_PLUS_AUTHEN_SVC_PPP       3
#define TAC_PLUS_AUTHEN_SVC_ARAP      4
#define TAC_PLUS_AUTHEN_SVC_PT        5
#define TAC_PLUS_AUTHEN_SVC_RCMD      6
#define TAC_PLUS_AUTHEN_SVC_X25       7
#define TAC_PLUS_AUTHEN_SVC_NASI      8
//
#define TAC_PLUS_AUTHEN_FLAG_NOECHO         0x1
//
#define TAC_PLUS_CONTINUE_FLAG_ABORT        0x1

//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////
// working with all tacacs+ packets
//////////////////////////////////////////////////

//////// HEADER

/**
@short All tacacs+ packets have the same header format
*/
#define TAC_PLUS_HDR_SIZE     12
struct tac_plus_pak_hdr {
  unsigned char version;    // 1 byte: version
  unsigned char type;       // 1 byte: type
  unsigned char seq_no;     // 1 byte: packet sequence number
  unsigned char encryption; // 1 byte: packet is encrypted or cleartext
  int session_id;           // 4 bytes: session identifier
  int datalength;           // 4 bytes: length of encrypted data following header
} __attribute__((__packed__));
typedef struct tac_plus_pak_hdr HDR;

/**
@short authentication start packet
*/
#define TAC_AUTHEN_START_FIXED_FIELDS_SIZE    8
struct authen_start {
  unsigned char action;
  unsigned char priv_lvl;
  unsigned char authen_type;
  unsigned char service;
  unsigned char user_len;
  unsigned char port_len;
  unsigned char rem_addr_len;
  unsigned char data_len;
  // <user_len bytes of char data>
  // <port_len bytes of char data>
  // <rem_addr_len bytes of u_char data>
  // <data_len bytes of u_char data>
} __attribute__((__packed__));

/**
@short authentication reply packet
*/
#define TAC_AUTHEN_REPLY_FIXED_FIELDS_SIZE    6
struct authen_reply {
  unsigned char status;           // 1 byte
  unsigned char flags;            // 1 byte
  unsigned short msg_len;         // 2 bytes
  unsigned short data_len;        // 2 bytes
  // <msg_len bytes of char data>
  // <data_len bytes of u_char data>
} __attribute__((__packed__));

/**
@short authentication CONTINUE packet
*/
#define TAC_AUTHEN_CONT_FIXED_FIELDS_SIZE       5
struct authen_cont {
  unsigned short user_msg_len;    // 2 bytes
  unsigned short user_data_len;   // 2 bytes
  unsigned char flags;            // 1 byte
  // <user_msg_len bytes of u_char data>
  // <user_data_len bytes of u_char data>
} __attribute__((__packed__));

/**
@short An authorization request packet
*/
#define TAC_AUTHOR_REQ_FIXED_FIELDS_SIZE      8
struct author {
  unsigned char authen_method;    // 1 byte
  unsigned char priv_lvl;         // 1 byte
  unsigned char authen_type;      // 1 byte
  unsigned char service;          // 1 byte
  unsigned char user_len;         // 1 byte
  unsigned char port_len;         // 1 byte
  unsigned char rem_addr_len;     // 1 byte
  unsigned char arg_cnt;          // 1 byte: the number of args
  // <arg_cnt u_chars containing the lengths of args 1 to arg n>
  // <user_len bytes of char data>
  // <port_len bytes of char data>
  // <rem_addr_len bytes of u_char data>
  // <char data for each arg>
} __attribute__((__packed__));

/**
@short authorization reply
*/
const int TAC_AUTHOR_REPLY_FIXED_FIELDS_SIZE = 6;
struct author_reply {
  unsigned char status;         // 1 byte
  unsigned char arg_cnt;        // 1 byte
  unsigned short msg_len;       // 2 bytes
  unsigned short data_len;      // 2 bytes
  // <arg_cnt u_chars containing the lengths of arg 1 to arg n>
  // <msg_len bytes of char data>
  // <data_len bytes of char data>
  // <char data for each arg>
} __attribute__((__packed__));

/**
@short accounting packet
*/
#define TAC_ACCT_REQ_FIXED_FIELDS_SIZE        9
struct acct {
  unsigned char flags;            // 1 byte
  unsigned char authen_method;    // 1 byte
  unsigned char priv_lvl;         // 1 byte
  unsigned char authen_type;      // 1 byte
  unsigned char authen_service;   // 1 byte
  unsigned char user_len;         // 1 byte
  unsigned char port_len;         // 1 byte
  unsigned char rem_addr_len;     // 1 byte
  unsigned char arg_cnt;          // 1 byte: the number of cmd args
  // one u_char containing size for each arg
  // <user_len bytes of char data>
  // <port_len bytes of char data>
  // <rem_addr_len bytes of u_char data>
  // char data for args 1 ... n
} __attribute__((__packed__));

/**
@short accounting reply packet
*/
#define TAC_ACCT_REPLY_FIXED_FIELDS_SIZE      5
struct acct_reply {
  unsigned short msg_len;         // 2 bytes
  unsigned short data_len;        // 2 bytes
  unsigned char status;           // 1 byte: status - does server ok
} __attribute__((__packed__));

//////////////////////////////////////////////////////////////////////

      ///// this is new session handler code /////

/**
LAYER 2 of tacacs+ (first layer - low-level packet work)
@short Low-level tacacs+ packet processing - encrypt/decrypt
@see Packet md5
@author rv
@version 1
*/
class TacacsPacket : public Packet {
//class TacacsPacket : public Packet, public md5 {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  host specific unique session id */
  unsigned int session_id_;
  /**
  have we received an abort flag? */
  int aborted_;
  /**
  seq. no. of last packet exchanged */
  int seq_no_;
  /**
  socket for this connection */
  int sock_;
  /**
  time of last packet exchange */
  time_t last_exch_;
  /**
  the key */
  string key_;
  /**
  line number key was found on */
  int keyline_;
  /**
  name of connected peer */
  string peer_;
  /**
  version of last packet read */
  unsigned char version_;
 public:
  /**
  init */
  TacacsPacket(const char*,const char*,int,unsigned int);
  /**
  drop, clear */
  ~TacacsPacket();
  /**
  returns type of packet (authentication, authoriz, accounting) */
  int selector(unsigned char*);
  /**
  session_id_ */
  unsigned int getSessionId();
  /**
  set session id */
  void setSessionId(unsigned int);
  /**
  aborted_ */
  int getAborted();
  /**
  set aborted flag */
  void setAborted(int);
  /**
  seq_no_ */
  int getSeqNo();
  /**
  set sequence number */
  void setSeqNo(int);
  /**
  @return sock_ value */
  int getSock();
  /**
  set sock_ value */
  void setSock(int);
  /**
  last_exch_ */
  time_t getLastExch();
  /**
  set last exch value */
  void setLastExch(time_t);
  /**
  key_ */
  string getKey();
  /**
  set key value
  @param key key value */
  void setKey(string);
  /**
  keyline_ */
  int getKeyline();
  /**
  set key line */
  void setKeyline(int);
  /**
  peer_ */
  string getPeer();
  /**
  set peer */
  void setPeer(string);
  /**
  version_ */
  unsigned char getVersion();
  /**
  set version */
  void setVersion(unsigned char);
  /**
  free memory after avp */
//  void free_avpairs(char **avp);
  /**
  create hash */
  void create_md5_hash(int, unsigned char,
      unsigned char, unsigned char*, unsigned char*);
  /**
  do xor with hash */
  bool md5_xor(HDR*, unsigned char*);
  /**
  read and decrypt packet from NAS */
  unsigned char *read_packet();
  /**
  read and decrypt packet without header (if header was readed before) */
  unsigned char *read_packet(unsigned char*);
  /**
  send packet to NAS, encrypt it */
  bool write_packet(unsigned char *buf);
};

class TacacsSessionContainer;

/**
@short read packets and add it to packet queue
@version 1
@author rv
@see Packet
*/
class TacacsPacketReader : public Packet {
  /**
  mutex locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  read only tacacs+ packet header */
  unsigned char *read_packet_header();
  /**
  queue for sessions */
  list<TacacsSessionContainer*> tacacsSessionQueue_;
 public:
  /**
  init */
  TacacsPacketReader(char*,int);
  /**
  free */
  ~TacacsPacketReader();
};

/////////////////////////////////////////////////////////////////////

// identify several internal authen errors
class TacErrAuthen {};

/**
@short Layer 3: low-level functions + process use from AAA db_authentication
@author rv
@version 1
*/
class TacacsServerAuthentication : public AAA {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  username string */
  string user_;
  /**
  port name string */
  string port_;
  /**
  rem_addr string */
  string rem_addr_;
  /**
  cisco id */
  ipaddr cisco_;
  /**
  result code */
  int res_;
  /**
  enable password */
  string enable_;
  /**
  request type */
  int request_;
 public:
  /**
  get user */
  string getUser();
  /**
  set user */
  void setUser(string);
  /**
  get port */
  string getPort();
  /**
  set port */
  void setPort(string);
  /**
  get rem addr */
  string getRemAddr();
  /**
  set rem addr */
  void setRemAddr(string);
  /**
  data */
  char data[DATA_LEN];
  /**
  cisco */
  ipaddr getCisco();
  /**
  set cisco */
  void setCisco(ipaddr);
  /**
  get res */
  int getRes();
  /**
  set res */
  int setRes(int);
  /**
  getted password */
  char password[DB_PASSWORD_LEN];
  /**
  chap/pap password */
  char pwstring[DATA_LEN];
  /**
  get enable */
  string getEnable();
  /**
  set enable */
  void setEnable(string);
  /**
  CHAP/PAP flag */
  bool chap;
  /**
  enable request */
  int en_req;
  /**
  buf cont */
  char *buf_cont;
  /**
  req */
  int getRequest();
  /**
  */
  void setRequest(int);
  /**
  get start */
  int get_start(unsigned char*,unsigned char*,char*);
  /**
  send reply */
  bool send_reply(const int,const char*,const char*,int,int);
  /**
  get cont */
  int get_cont(unsigned char*,unsigned char*,char*,char*,int);
  /**
  loginstring */
//  char loginstring[32];
  /**
  pwdstring */
//  char pwdstring[32];
  /**
  ptr to tacacs packet handler */
  TacacsPacket *t;
  /**
  init */
  TacacsServerAuthentication(TacacsPacket*);
  /**
  free */
  ~TacacsServerAuthentication();
};

/**
@short use from AAA db_authorization
@author rv
@version 1
*/
class TacacsServerAuthorization : public AAA {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
 public:
  /**
  method */
  int method;
  /**
  priv level */
  int priv_lvl;
  /**
  authen type */
  int authen_type;
  /**
  authen service */
  int authen_service;
  /**
  user */
  char user[USERNAME_LEN];
  /**
  port */
  char port[PORT_LEN];
  /**
  rem addr */
  char rem_addr[REM_ADDR_LEN];
  /**
  for user information storage */
  db_user dbd;
  /**
  store service= */
  char *curService;
  /**
  store protocol= */
  char *curProtocol;
  /**
  added flag */
  int added;
  /**
  replaced flag */
  int replaced;
  /**
  start av */
  int start_av;
  /**
  for av, which we are receive from cisco */
  char *avpair[MAX_AVP];
  /**
  for av, which we are send to cisco */
  char *bavpair[MAX_AVP];
  /**
  keep current avpair */
  char *nas_avp;
  /**
  preg */
  regex_t preg;
  /**
  nmatch */
  size_t nmatch;
  /**
  pmatch */
  regmatch_t pmatch[2];
  /**
  flag1 */
  bool flag1;
  /**
  flag2 */
  bool flag2;
  /**
  flag3 */
  bool flag3;
  /**
  flag4 */
  bool flag4;
  /**
  j */
  int j;
  /**
  avps */
  char *avps[MAX_AVP];
  /**
  get authorization request */
  bool get_request(unsigned char*,unsigned char*,int*,int*,int*,int*,char*,
      char*,char*,char **avpair);
  /**
  send res */
  bool send_response(const int,const char*,const char*,char**);
  /**
  is this value optional? */
  bool optional(char*);
  /**
  is this value mandatory? */
  bool mandatory(char*);
  /**
  is attrs match? */
  bool match_attr(char*,char*);
  /**
  is values match? */
  bool match_value(char*,char*);
  /**
  get value from avp */
  char *get_value(char*);
  /**
  default authorization flag */
  bool def_author;
  /**
  packet processor ptr */
  TacacsPacket *t;
  /**
  init */
  TacacsServerAuthorization(TacacsPacket*, bool);
  /**
  free */
  ~TacacsServerAuthorization();
};

/**
@short TACACS+ server-side accounting methods, add it to database/logfile
@see AAA db_accounting
@author rv
@version 1
*/
class TacacsServerAccounting : public AAA {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
 public:
  /**
  avpairs */
  list<string> avpair;
  /**
  get tacacs+ accounting request */
  int get_request(unsigned char*,unsigned char*,int*,int*,int*,int*,char*,
     char*,char*,char*);
  /**
  send reply */
  bool send_reply(char*,char*,const int);
  /**
  ptr to packet processor */
  TacacsPacket *t;
  /**
  init */
  TacacsServerAccounting(TacacsPacket*);
  /**
  free */
  ~TacacsServerAccounting();
};

/**
@short this is entry in sessions control queue
@author rv
@version 1
*/
class TacacsSessionContainer {
  /**
  thread locker */
  pthread_mutex_t mutex_;
  /**
  set lock */
  void lock();
  /**
  remove lock */
  void unlock();
  /**
  thread locker */
  pthread_mutex_t mutex2_;
  /**
  ptr */
  TacacsServerAuthentication *tac_authen;
  /**
  ptr */
  TacacsServerAuthorization *tac_author;
  /**
  ptr */
  TacacsServerAccounting *tac_account;
  /**
  time created */
  time_t created_;
  /**
  loginstring */
  string loginstring_;
  /**
  pwdstring */
  string pwdstring_;
  /**
  author */
  bool authoriz_;
  /**
  key */
  string key_;
  /**
  sock */
  int sock_;
  /**
  sess id */
  unsigned int session_id_;
  /**
  peer */
  ipaddr peer_;
 public:
  /**
  set lock */
  void lock2();
  /**
  remove lock */
  void unlock2();
  /**
  ptr to packet processor */
  TacacsPacket *tac;
  /**
  get created */
  time_t getCreated();
// /**
//  set created */
//  void setCreated(time_t);
  /**
  get loginstring */
  string getLoginstring();
  /**
  set loginstring */
  void setLoginstring(string);
  /**
  get pwdstring
  @return pwdstring */
  string getPwdstring();
  /**
  set pwdstring */
  void setPwdstring(string);
  /**
  get author */
  bool getAuthoriz();
  /**
  set author */
  void setAuthoriz(bool);
  /**
  get key */
  string getKey();
  /**
  set key */
  void setKey(string);
  /**
  get sock */
  int getSock();
  /**
  set sock */
  void setSock(int);
  /**
  get sess id */
  unsigned int getSessionId();
  /**
  set sess id */
  void setSessionId(unsigned int);
  /**
  get peer */
  ipaddr getPeer();
  /**
  set peer */
  void setPeer(ipaddr);
  /**
  init */
  TacacsSessionContainer(ipaddr,int,int,char*,char*,char*,bool);
  /**
  free */
  ~TacacsSessionContainer();
  /**
  processor */
  void process(unsigned char*,unsigned char*);
};

/////////////////////////////////////////////////////

/**
@short it will help us for write tacacs+ proxy
@author rv
@version 0
*/
class TacacsClient : public TacacsPacket {
 public:
  /**
  send start */
  bool authen_send_start(const char* port,
      const char* username,
      const int type,
      const char* data);
  /**
  get reply */
  int authen_get_reply(char* server,
      char* datas);
  /**
  send cont */
  bool authen_send_cont(const char* user_msg,
      const char* data);
  /**
  send req */
  bool author_send_request(const int method,
      const int priv_lvl,
      const int authen_type,
      const int authen_service,
      const char *user,
      const char *port,
      char **avpair);
  /**
  get resp */
  int author_get_response(char *server_msg,
      char *data,
      char **avpair);
  /**
  send req */
  bool account_send_request(const int flag,
      const int method,
      const int priv_lvl,
      const int authen_type,
      const int authen_service,
      const char *user,
      const char *port,
      char **avpair);
  /**
  get reply */
  int account_get_reply(char *server_msg,char *data);
  /**
  init */
  TacacsClient(char *k,char *p,int f):TacacsPacket(k,p,f,0) {}
};

};

#endif //__TACACS_H__
