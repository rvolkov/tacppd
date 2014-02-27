// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
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

#ifndef __RADIUS_H__
#define __RADIUS_H__

#include "global.h"

namespace tacpp {

// RADIUS codes

// send from NAS to server
// must have attribute User-Name
// must have NAS-Identifier or NAS-IP-Address
// must have User-Password or CHAP-Password
// must have NAS-Port or NAS-Port-Type
#define RC_ACCESS_REQUEST       1

// send from server to NAS
// Identifier must be saved from packet Access-Request
#define RC_ACCESS_ACCEPT        2
#define RC_ACCESS_REJECT        3
#define RC_ACCOUNTING_REQUEST   4
#define RC_ACCOUNTING_RESPONCE  5
#define RC_ACCESS_CHALLENGE     11
#define RC_STATUS_SERVER        12
#define RC_STATUS_CLIENT        13


// Radius Identifier


// Length

#define RADIUS_HEADER_LEN       20

// Authenticator

// Request authenticator

// Response Authenticator





// Radius Attributes
#define RC_USER_NAME	    	  	1
#define RC_USER_PASSWORD	    	2
#define RC_CHAP_PASSWORD        3
#define RC_NAS_IP_ADDRESS       4
#define RC_NAS_PORT		        	5
#define RC_SERVICE_TYPE			    6
#define RC_FRAMED_PROTOCOL		  7
#define RC_FRAMED_IP_ADDRESS		8
#define RC_FRAMED_IP_NETMASK		9
#define RC_FRAMED_ROUTING       10
#define RC_FILTER_ID            11
#define RC_FRAMED_MTU           12
#define RC_FRAMED_COMPRESSION   13
#define RC_LOGIN_IP_HOST        14
#define RC_LOGIN_SERVICE        15
#define RC_LOGIN_TCP_PORT       16
#define RC_UNUSSIGNED1          17
#define RC_REPLY_MESSAGE        18
#define RC_CALLBACK_NUMBER      19
#define RC_CALLBACK_ID          20
#define RC_UNUSSIGNED2          21
#define RC_FRAMED_ROUTE         22
#define RC_FRAMED_IPX_NETWORK   23
#define RC_STATE                24
#define RC_CLASS                25
#define RC_VENDOR_SPECIFIC      26
#define RC_SESSION_TIMEOUT      27
#define RC_IDLE_TIMEOUT         28
#define RC_TERMINATION_ACTION   29
#define RC_CALLED_STATION_ID    30
#define RC_CALLING_STATION_ID   31
#define RC_NAS_IDENTIFIER       32
#define RC_PROXY_STATE          33
#define RC_LOGIN_LAT_SERVICE    34
#define RC_LOGIN_LAT_NODE       35
#define RC_LOGIN_LAT_GROUP      36
#define RC_FRAMED_APPLETALK_LINK	37
#define RC_FRAMED_APPLETALK_NETWORK	38
#define RC_FRAMED_APPLETALK_ZONE	39
#define RC_CHAP_CHALLENGE       60
#define RC_NAS_PORT_TYPE        61
#define RC_PORT_LIMIT           62
#define RC_LOGIN_LAT_PORT       63

/**
@short radius header
*/
struct radius_header {
  /**
   */
  unsigned char Code;
  /**
   */
  unsigned char Identifier;
  /**
   */
  unsigned short int Length;
  /**
   */
  unsigned char Authenticator[16];
  // unsigned char attributes[Length]
} __attribute__((__packed__));

/**
@short attribute header
*/
struct attribute_header {
  /**
   */
  unsigned char type;
  /**
   */
  unsigned char length;
  // unsigned char value[length]
} __attribute__((__packed__));

/**
@version 1
@author rv
*/
class RadiusPacket {
  /**
   */
  unsigned char Identifier;
 public:
};

};

#endif //__RADIUS_H__
