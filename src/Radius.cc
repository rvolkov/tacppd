// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: Radius.cc
// description: RADIUS low-level functions

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"

/*

// client access request
void
RadiusPacket::send_access_request() {
  radius_header p;
  p.Code = RC_ACCESS_REQUEST;
//Identifier =
}

// server get access request
void
RadiusPacket::get_access_request() {
  radius_header *p = read_packet();
  Identifier = p->Identifier;
}


// server send access accept
void
RadiusPacket::send_access_accept() {
  radius_header p;
  p.Code = RC_ACCESS_ACCEPT;
  p.Identifier = Identifier;
}

// client get access accept
void
RadiusPacket::get_access_accept() {

}

*/

////////////////////// that's all ///////////////////////
