// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
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

#ifndef __GTPPRIME_H__
#define __GTPPRIME_H__

#include "global.h"

namespace tacpp {

struct gtpp_hdr6 {
//  unsigned int htype : 1; //header type - bit 1 = 1 - this is 6 bytes long header
//  unsigned int gtpspec : 3; // bits 2,3,4 = 111 - gtp spec
//  unsigned int ptype : 1;   //protocol type, bit 5 = 0b0
//  unsigned int pver : 3;    // version number - bits 6,7,8 - 0b000
  unsigned char flag;		// 1 byte
  unsigned char message_type;  // 1 byte
  unsigned short length;       // 2 bytes
  unsigned short sequence;     // 2 bytes
} __attribute__((__packed__));

#define GTPP_HDR6_LEN	6

struct gtpp_hdr20 {
// bit 1 = 0 - 20 bytes long header
// 2,3,4
// 5
// 6,7,8
  
};

// message types
#define MSGT_ECHO_REQUEST		1
#define MSGT_ECHO_RESPONSE		2
#define MSGT_VERSION_NOT_SUP	3
#define MSGT_NODE_ALIVE_REQ		4
#define MSGT_NODE_ALIVE_RESP	5
#define MSGT_REDIR_REQ			6
#define MSGT_REDIR_RESP			7
#define MSGT_DATA_RECORD_REQ	240
#define MSGT_DATA_RECORD_RESP	241

// header type
#define HEADER_6BYTES	1
#define HEADER_20BYTES	0

// cause values
#define CAUSE_RECORD_ACCEPTED				128
#define CAUSE_NO_RESOURE					199
#define CAUSE_SERVICE_NO_SUPPORT			200
#define CAUSE_SYSTEM_FAILURE				204
#define CAUSE_MANDATORY_IE_INCORRECT		201
#define CAUSE_MANDATORY_IE_MISSING			202
#define CAUSE_OPTIONAL_IE_INCORRECT			203
#define CAUSE_INVALID_MESSAGE_FORMAT		193
#define CAUSE_VERSION_NOT_SUPPORTED			198
#define CAUSE_REQUEST_NOT_FULLFILLED		255
#define CAUSE_REQUEST_ALREADY_FULLFILLED	253
#define CAUSE_REQUEST_RTPDPA_FULLFILLED		252


struct gtpp_qs_csg_echo_resp {
  unsigned char ietype;   // 1 byte - IE type
  unsigned char rcounter; // 1 byte - restart counter
} __attribute__((__packed__));
#define GTPP_QS_CSG_ECHO_RESP_LEN	2

// node alive request
struct node_alive_rq {
  unsigned char ietype;   // 1 byte - IE type
  unsigned short length;  // 2 byte
  unsigned int address;   // 4 byte
} __attribute__((__packed__));
#define NODE_ALIVE_RQ_LEN	7

// Data Record Transfer request
struct data_record_transfer_req {
  unsigned char  ptc_ie;   // packet transfer command ie
  unsigned char  ptc;      // packet transfer command

  unsigned char  drp_ie;   // data record packet ie
  unsigned short length;   // length (number bytes)
  unsigned char  ndr;      // number of data records
  unsigned char  drf;      // data record format
  unsigned short drfv;     // data record format version
  unsigned short drlength; // length
//  unsigned short drt;      // data record type
} __attribute__((__packed__));
#define DATA_RECORD_TRANSFER_REQ_LEN	13

// data record
struct data_record {
  unsigned short drt;    // data record type
  unsigned short length; // length
//  char *ptr;		 // pointer to other part
} __attribute__((__packed__));

// Data Record Transfer response
struct data_record_transfer_resp {
  unsigned char ietype: 8;  // 1 byte - IE type
  unsigned char cause: 8;   // 1 byte
  unsigned char ietypen: 8; // 1 byte
  unsigned short length: 16; // 2 bytes
  unsigned short seq: 16;    // 2 bytes
} __attribute__((__packed__));
#define DATA_RECORD_TRANSFER_RESP_LEN	7

// drt service authorization resp
struct drt_service_authorization_resp {
  unsigned short drt;
} __attribute__((__packed__));
#define DRT_SERVICE_AUTHORIZATION_RESP_LEN

// qmm
struct quota_management_message_csg {
  unsigned short ietype;
  unsigned short length;
} __attribute__((__packed__));

// qmm
struct quota_management_message_qs {
  unsigned char ietype;
//  unsigned short type;
  unsigned short length;
} __attribute__((__packed__));

#define MT_HTTP_HEADER					0x0001
#define MT_HTTP_STATS					0x0002
#define MT_TCP							0x0004
#define MT_UDP							0x0005
#define MT_IP							0x0007
#define MT_QUOTA_DOWNLOAD_NTF			0x0008
#define MT_QUOTA_RETURN_NTF				0x0009
#define MT_AUTHORIZATION_FAIL_NTF		0x0010
#define MT_SERVICE_STOP_NTF				0x0011
#define MT_HTTP_STAT_INTER				0x0012
#define MT_TCP_INTER					0x0014
#define MT_UDP_INTER					0x0015
#define MT_IP_INTER						0x0017
#define MT_USER_PROFILE_RQ				0x0020
#define MT_USER_PROFILE_RP				0x0021
#define MT_SERVICE_AUTHORIZATION_RQ		0x0022
#define MT_SERVICE_AUTHORIZATION_RS		0x0023
#define MT_SERVICE_REAUTHORIZATION_RQ	0x0024
#define MT_SERVICE_STOP					0x0025
#define MT_QUOTA_RETURN					0x0026
#define MT_CONTENT_AUTH_RQ				0x0027
#define MT_CONTENT_AUTH_RS				0x0028
#define MT_QUOTA_RETURN_RQ				0x0029
#define MT_SERVICE_STOP_RQ				0x002A
#define MT_SERVICE_VRFQ_RQ				0x002B
#define MT_SERVICE_VRFQ_RS				0x002C
#define MT_USER_DISCONNECT_RQ			0x002D
#define MT_QUOTA_PUSH_RQ				0x002E
#define MT_QUOTA_RET_ACC				0x002F
#define MT_QUOTA_PUSH_RS				0x0030
#define MT_FTP							0x0031
#define MT_CONTINUE						0x0032
#define MT_SMTP							0x0033
#define MT_POP3							0x0034
#define MT_WAP_STATS					0x0035
#define MT_RTSP							0x0036
#define MT_FIXED_WAP					0x0037
#define MT_FIXED_HTTP					0x0038
#define MT_FIXED_HTTP_INTER				0x003A
#define MT_VAR_HTTP						0x0039
#define MT_SERVICE_USAGE				0x0040
#define MT_FIXED_RTSP					0x0042
#define MT_FIXED_RTSP_INTER				0x0043


// TLVs
#define TLV_L4FLOW						0x0001
#define TLV_L3FLOW						0x0002
#define TLV_IPSTATS						0x0003
#define TLV_CORRELATOR					0x0004
#define TLV_TCPSTATS					0x0005
#define TLV_TIMESTAMP					0x0006
#define TLV_IPPACKETCOUNT				0x0007
#define TLV_BROWSER						0x0008
#define TLV_HOSTNAME					0x0009
#define TLV_URL							0x000a
#define TLV_USERID						0x000b
#define TLV_CUSTOMERSTRING				0x000c
#define TLV_SERVICETYPE					0x000d
#define TLV_FROM						0x000e
#define TLV_APPRETURNCODE				0x000f
#define TLV_BILLINGPLAN					0x0010
#define TLV_CAUSE						0x0011
#define TLV_QUADRANS					0x0012
#define TLV_USAGE						0x0013
#define TLV_USERINDEX					0x0014
#define TLV_SERVICEID					0x0015
#define TLV_SESSIONID					0x0016
#define TLV_TIMEOUT						0x0017
#define TLV_REMAINQUOTA					0x0018
#define TLV_SERVICELIST					0x0019
#define TLV_FTPUSERNAME					0x001a
#define TLV_FTPDIR						0x001b
#define TLV_FTPFLOW						0x001c
#define TLV_FTPFILENAME					0x001d
#define TLV_FTPCOMMAND					0x001e
#define TLV_RTSPFLOW					0x001f
#define TLV_WTPINFO						0x0020
#define TLV_WSPCLASS					0x0021
#define TLV_WSPPDU						0x0022
#define TLV_WAPIP						0x0023
#define TLV_WAPTRANS					0x0024
#define TLV_WAPCONTENT					0x0025
#define TLV_WAPABORT					0x0026
#define TLV_REFUND						0x0027
#define TLV_HTTPMETHOD					0x0028
#define TLV_REPORTSTRING				0x0029
#define TLV_WAPWSP						0x002a
#define TLV_CONNSERVICE					0x002b
#define TLV_QUALIFIEDQUADRANS			0x002e
#define TLV_REPORTRADIUS				0x0030
#define TLV_CONTINUATION				0x0031
#define TLV_WEIGHT						0x0032
#define TLV_AUTHACTION					0x0033
#define TLV_REQUEST						0x0034
#define TLV_REQUESTID					0x0035
#define TLV_QUALIFIEDQUOTAREMAIN		0x0036
#define TLV_SESSTYPE					0x0037
#define TLV_CHANTYPE					0x0038
#define TLV_TRANTYPE					0x0039
#define TLV_TRANTYPE					0x0039
#define TLV_HOSTNAMEFIX					0x003a
#define TLV_TARIFCLASS					0x003b
#define TLV_CONTENTPROV					0x003c
#define TLV_CONTENTDELIVERED			0x003d
#define TLV_BCDTIMESTAMP				0x003e
#define TLV_LARGEURL					0x003f
#define TLV_SERVICEVERIFY				0x0040
#define TLV_SERVICEIPVOLUME				0x0041
#define TLV_SERVICESTARTTIME			0x0042
#define TLV_MAILRCPTS					0x0045
#define TLV_QSFLAGS						0x0044
#define TLV_SERVICETCPVOLUME			0x0046
#define TLV_COMPLETETRANS				0x0047
#define TLV_IMAP						0x0048
#define TLV_QUALIFIEDUSAGE				0x0049


struct tlv_userindex {
  unsigned short length;
  unsigned short ipaddress;
  char *userid;
};
	
};

#endif // __GTPPRIME_H__
