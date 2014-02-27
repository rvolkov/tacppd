// NetFlow handle
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

/*
The NetFlow packet fragments structures used from NetFlow
examples on Cisco Systems, Inc. web site
*/

// all comments which start from /** - this is comments for KDoc
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

#ifndef __NETFLOW_H__
#define __NETFLOW_H__

#include "global.h"

namespace tacpp {

// set up types:
typedef unsigned char Bit8;
typedef unsigned short Bit16;
//#if SIZEOF_LONG == 4
typedef unsigned long Bit32;
//#elif SIZEOF_LONG == 8
//typedef unsigned int Bit32;
//#endif

//Bit16 aaa;
//Bit8 bbb;
//Bit32 ccc;

#define IP4_ADDR_LEN    4
#define TRANS_ADDR_LEN  2
#define AS_ADDR_LEN     2

#define	  ushort        Bit16
#define	  uchar         Bit8
#define	  ulong         Bit32
#define	  ipaddrtype    Bit32

//// PACKET FORMAT:
//
// HDR_VER NetFlowHeader_vX NetFlow_vX NetFlow_vX ... NetFlow_vX
//
//

/**
@short NetFlow Export Version 1 Header Format
@version 1
@author i think, cisco
*/
struct NetFlowHeader_v1 {
  /**
  Current version=1 */
  ushort version;
  /**
  The number of records in PDU. */
  ushort count;
  /**
  Current time in msecs since router booted */
  ulong SysUptime;
  /**
  Current seconds since 0000 UTC 1970 */
  ulong unix_secs;
  /**
  Residual nanoseconds since 0000 UTC 1970 */
  ulong unix_nsecs;
} __attribute__((__packed__));

/**
@short NetFlow Export Version 5 Header Format
@version 1
@author i think, cisco
*/
struct NetFlowHeader_v5 {
  /**
  Current version=5 */
  ushort version;
  /**
  The number of records in PDU. */
  ushort count;
  /**
  Current time in msecs since router booted */
  ulong SysUptime;
  /**
  Current seconds since 0000 UTC 1970 */
  ulong unix_secs;
  /**
  Residual nanoseconds since 0000 UTC 1970 */
  ulong unix_nsecs;
  /**
  Sequence number of total flows seen */
  ulong flow_sequence;
  /**
  Type of flow switching engine (RP,VIP,etc.) */
  uchar engine_type;
  /**
  Slot number of the flow switching engine */
  uchar engine_id;
  /**
  zeros */
  ushort reserved;
} __attribute__((__packed__));

/**
@short NetFlow Export Version 7 Header Format
@version 1
@author i think, cisco
*/
struct NetFlowHeader_v7 {
  /**
  Current version=7 */
  ushort version;
  /**
  The number of records in PDU. */
  ushort count;
  /**
  Current time in msecs since router booted */
  ulong SysUptime;
  /**
  Current seconds since 0000 UTC 1970 */
  ulong unix_secs;
  /**
  Residual nanoseconds since 0000 UTC 1970 */
  ulong unix_nsecs;
  /**
  Sequence number of total flows seen */
  ulong flow_sequence;
  /**
  zeros */
  ulong reserved;
} __attribute__((__packed__));

/**
@short NetFlow Export Version 8 Header Format
@version 1
@author i think, cisco
*/
struct NetFlowHeader_v8 {
  /**
  Current version */
  ushort version;
  /**
  The number of records in PDU. */
  ushort count;
  /**
  Current time in msecs since router booted */
  ulong SysUptime;
  /**
  Current seconds since 0000 UTC 1970 */
  ulong unix_secs;
  /**
  Residual nanoseconds since 0000 UTC 1970 */
  ulong unix_nsecs;
  /**
  Seq counter of total flows seen */
  ulong flow_sequence;
  /**
  Type of flow switching engine */
  uchar engine_type;
  /**
  Slot number of the flow switching engine */
  uchar engine_id;
  /**
  Aggregation method being used */
  uchar aggregation;
  /**
  Version of the aggregation export=2 */
  uchar agg_version;
  /**
  zeros */
  ulong reserved;
} __attribute__((__packed__));

/**
@short flow record within a version 1 NetFlow Export datagram
@version 1
@author i think, cisco
*/
struct NetFlow_v1 {
  /**
  Source IP Address */
  ipaddrtype srcaddr;
  /**
  Destination IP Address */
  ipaddrtype dstaddr;
  /**
  Next hop router's IP Address */
  ipaddrtype nexthop;
  /**
  Input interface index */
  ushort input;
  /**
  Output interface index */
  ushort output;
  /**
  Packets sent in Duration (milliseconds between
  1st & last packet in this flow) */
  ulong dPkts;
  /**
  Octets sent in Duration (milliseconds between
  1st & last packet in this flow) */
  ulong dOctets;
  /**
  SysUptime at start of flow */
  ulong First;
  /**
  and of last packet of the flow */
  ulong Last;
  /**
  TCP/UDP source port number (.e.g, FTP, Telnet, etc.,or equivalent) */
  ushort srcport;
  /**
  TCP/UDP destination port number (.e.g, FTP,
  Telnet, etc.,or equivalent) */
  ushort dstport;
  /**
  pad to word boundary (unused - zeros) */
  ushort pad1;
  /**
  IP protocol, e.g., 6=TCP, 17=UDP, etc... */
  uchar prot;
  /**
  IP Type-of-Service */
  uchar tos;
  /**
  Cumulative OR of tcp flags */
  uchar tcp_flags;
  /**
  pad1 */
  uchar tcp_retx_cnt;
  /**
  pad2 */
  uchar tcp_retx_secs;
  /**
  pad3 */
  uchar tcp_misseq_cnt;
  /**
  zeros */
  ulong reserved;
} __attribute__((__packed__));

/**
@short flow record within a version 5 NetFlow Export datagram
@version 1
@author i think, cisco
*/
struct NetFlow_v5 {
  /**
  Source IP Address */
  ipaddrtype srcaddr;
  /**
  Destination IP Address */
  ipaddrtype dstaddr;
  /**
  Next hop router's IP Address */
  ipaddrtype nexthop;
  /**
  Input interface index */
  ushort input;
  /**
  Output interface index */
  ushort output;
  /**
  Packets sent in Duration (milliseconds between
  1st & last packet in  this flow) */
  ulong dPkts;
  /**
  Octets sent in Duration (milliseconds between
  1st & last packet in this flow) */
  ulong dOctets;
  /**
  SysUptime at start of flow */
  ulong First;
  /**
  and of last packet of the flow */
  ulong Last;
  /**
  TCP/UDP source port number (.e.g, FTP, Telnet, etc.,or equivalent) */
  ushort srcport;
  /**
  TCP/UDP destination port number (.e.g, FTP,
  Telnet, etc.,or equivalent) */
  ushort dstport;
  /**
  pad to word boundary */
  uchar pad1;
  /**
  Cumulative OR of tcp flags */
  uchar tcp_flags;
  /**
  IP protocol, e.g., 6=TCP, 17=UDP, etc... */
  uchar prot;
  /**
  IP Type-of-Service */
  uchar tos;
  /**
  dst peer/origin Autonomous System */
  ushort dst_as;
  /**
  source peer/origin Autonomous System */
  ushort src_as;
  /**
  destination route's mask bits */
  uchar dst_mask;
  /**
  source route's mask bits */
  uchar src_mask;
  /**
  pad to word boundary */
  ushort pad2;
} __attribute__((__packed__));

/**
@short flow record within a version 7 NetFlow Export datagram
@version 1
@author i think, cisco
*/
struct NetFlow_v7 {
  /**
  Source IP Address */
  ipaddrtype srcaddr;
  /**
  Destination IP Address */
  ipaddrtype dstaddr;
  /**
  Next hop router */
  ipaddrtype nexthop;
  /**
  input interface index */
  ushort input;
  /**
  output interface index */
  ushort output;
  /**
  Packets sent in Duration */
  ulong dPkts;
  /**
  Octets sent in Duration */
  ulong dOctets;
  /**
  SysUptime at start of flow. */
  ulong First;
  /**
  and of last packet of the flow. */
  ulong Last;
  /**
  TCP/UDP source port number or equivalent */
  ushort srcport;
  /**
  TCP/UDP dest port number or equivalent */
  ushort dstport;
  /**
  Shortcut mode(dest only,src only,full flows */
  uchar flags;
  /**
  TCP flags */
  uchar tcp_flags;
  /**
  IP protocol, e.g., 6=TCP, 17=UDP, ... */
  uchar prot;
  /**
  IP Type-of-Service */
  uchar tos;
  /**
  source AS# */
  ulong src_as;
  /**
  destination AS# */
  ulong dst_as;
  /**
  source subnet mask */
  uchar src_mask;
  /**
  destination subnet mask */
  uchar dst_mask;
  /**
   */
  ushort pad;
  /**
  Router which is shortcut by switch */
  ipaddrtype router_sc;
} __attribute__((__packed__));


// The aggregated flow records within version 8 NetFlow Export datagrams

/**
@short For ASMatrix v8 aggregation scheme:
@version 1
@author i think, cisco
*/
struct NetFlow_v8_ASMatrix {
  /**
  Number of flows */
  ulong flows;
  /**
  Packets sent in Duration */
  ulong dPkts;
  /**
  Octets sent in Duration */
  ulong dOctets;
  /**
  SysUptime at start of flow */
  ulong First;
  /**
  and of last packet of flow */
  ulong Last;
  /**
  originating AS of source address */
  ushort src_as;
  /**
  originating AS of destination address */
  ushort dst_as;
  /**
  Input interface index */
  ushort input;
  /**
  Output interface index */
  ushort output;
} __attribute__((__packed__));

/**
@short For ProtocolPortMatrix v8 aggregation scheme:
@version 1
@author i think, cisco
*/
struct NetFlow_v8_ProtocolPortMatrix {
  /**
  Number of flows */
  ulong flows;
  /**
  Packets sent in Duration */
  ulong dPkts;
  /**
  Octets sent in Duration. */
  ulong dOctets;
  /**
  SysUptime at start of flow */
  ulong First;
  /**
  and of last packet of flow */
  ulong Last;
  /**
  IP protocol, e.g., 6=TCP, 17=UDP, ... */
  uchar prot;
  /**
   */
  uchar pad;
  /**
   */
  ushort reserved;
  /**
  TCP/UDP source port number or equivalent */
  ushort srcport;
  /**
  TCP/UDP dest port number or equivalent */
  ushort dstport;
} __attribute__((__packed__));

/**
@short For SourcePrefixMatrix v8 aggregation scheme:
@version 1
@author i think, cisco
*/
struct NetFlow_v8_SourcePrefixMatrix {
  /**
  Number of flows */
  ulong flows;
  /**
  Packets sent in Duration */
  ulong dPkts;
  /**
  Octets sent in Duration. */
  ulong dOctets;
  /**
  SysUptime at start of flow */
  ulong First;
  /**
  and of last packet of flow */
  ulong Last;
  /**
  Source prefix */
  ipaddrtype src_prefix;
  /**
  source address prefix mask bits */
  uchar src_mask;
  /**
  padding */
  uchar pad;
  /**
  originating AS of source address */
  ushort src_as;
  /**
  Input interface index */
  ushort input;
} __attribute__((__packed__));

/**
@short For DestinationPrefixMatrix v8 aggregation scheme:
@version 1
@author i think, cisco
*/
struct NetFlow_v8_DestinationPrefixMatrix {
  /**
  Number of flows */
  ulong flows;
  /**
  Packets sent in Duration */
  ulong dPkts;
  /**
  Octets sent in Duration. */
  ulong dOctets;
  /**
  SysUptime at start of flow */
  ulong First;
  /**
  and of last packet of flow */
  ulong Last;
  /**
  Destination prefix */
  ipaddrtype dst_prefix;
  /**
  destination address prefix mask bits */
  uchar dst_mask;
  /**
   */
  uchar pad;
  /**
  originating AS of destination address */
  ushort dst_as;
  /**
  Output interface index */
  ushort output;
} __attribute__((__packed__));

/**
@short For PrefixMatrix v8 aggregation scheme:
@version 1
@author i think, cisco
*/
struct NetFlow_v8_PrefixMatrix {
  /**
  Number of flows */
  ulong flows;
  /**
  Packets sent in Duration */
  ulong dPkts;
  /**
  Octets sent in Duration. */
  ulong dOctets;
  /**
  SysUptime at start of flow */
  ulong First;
  /**
  and of last packet of flow */
  ulong Last;
  /**
  Source prefix */
  ipaddrtype src_prefix;
  /**
  Destination prefix */
  ipaddrtype dst_prefix;
  /**
  destination address prefix mask bits */
  uchar dst_mask;
  /**
  source address prefix mask bits */
  uchar src_mask;
  /**
   */
  ushort reserved;
  /**
  originating AS of source address */
  ushort src_as;
  /**
  originating AS of destination address */
  ushort dst_as;
  /**
  Input interface index */
  ushort input;
  /**
  Output interface index */
  ushort output;
} __attribute__((__packed__));


/////////////////////////////////////////////
//    this is for collecting information
/////////////////////////////////////////////

#undef ulong
#undef ushort
#undef uchar
#undef ipaddrtype

/**
@short data for aggregator
@author rv
@version 1
*/
class NfData {
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
  input interface index */
  unsigned int ifin_;
  /**
  bytes */
  unsigned long octets_;
  /**
  packets */
  unsigned long packets_;
  /**
  protocol */
  int protocol_;
  /**
  start time */
  time_t startt_;
  /**
  stop time */
  time_t stopt_;
 public:
  /**
  init short */
  NfData(unsigned long,unsigned long,unsigned int,time_t,time_t);
  /**
  clear */
  ~NfData();
  /**
  add traffic to counter */
  void add(unsigned long,unsigned long,time_t);
  /**
  is this entry expired, and we need in write it? */
  bool isExpired(time_t);
  /**
  flow input ifindex */
  unsigned int getIfIn();
  /**
  flow start time */
  time_t getStart();
  /**
  flow stop time */
  time_t getStop();
  /**
  octets counter value */
  unsigned long getOctets();
  /**
  packets counter value */
  unsigned long getPackets();
};

/**
@short netflow aggregator
@author rv
@version 1
*/
class NfAggregator {
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
  netflow aggregator buffer */
  map<Bit32,map<Bit32,map<Bit32,NfData*> > > nfb_;
  /**
  thread for aggregator process */
  pthread_t thrvar_;
  /**
  write process mask */
  bool writeProcess_;
  /**
  exp time */
  int expireTime_;
  /**
  queue size */
  int buf_size_;
 public:
  /**
  process */
  void process(Bit32,char*);
  /**
  init */
  NfAggregator();
  /**
  clear */
  ~NfAggregator();
  /**
   run process */
  void runProcess();
  /**
  add */
  void add(Bit32,Bit32,Bit32,unsigned long,unsigned long,unsigned int,time_t,time_t);
  /**
  expire aggregation table and write it to database or file */
  void expire();
  /**
  size */
  int size();
  /**
  nodes */
  int nodesSize();
  /**
  sources */
  int sourceSize();
  /**
  write process */
  bool getWriteProcess();
  /**
  wr process */
  void setWriteProcess(bool);
  /**
  expiration time */
  int expireTime();
  /**
  add expire time */
  void addExpireTime();
  /**
  del expire time */
  void delExpireTime();
};

using std::pair;

};

namespace netflow_v1r1 {

};

namespace netflow = netflow_v1r1;

#endif //__NETFLOW_H__
