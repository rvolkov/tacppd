// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: none.cc
// description: void SNMP handling module

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// Support will not be provided under any circumstances for this program by
// tacppd.com, it's employees, volunteers or directors, unless a commercial
// support agreement and/or explicit prior written consent has been given.
// Visit http://tacppd.com for more information

// modifications:
//

#include "snmp_module.h"

///////////////// methods for processing //////////////////

extern "C" {
  bool getnas(SnmpDevice*,SnmpD*(*)(char*,char*,char*),char*);
  bool dropnas(SnmpDevice*,char*,SnmpD*(*)(char*,char*,char*),
    bool(*)(char*,char*,char*,char*,char),char*);
};

//////////////////// void NAS //////////////////////
bool
getnas(SnmpDevice *sd,SnmpD*(*aaa)(char*,char*,char*),char *derr) {
  return false;
}
bool
dropnas(SnmpDevice *sd,char *port,SnmpD*(*aaa)(char*,char*,char*),
    bool(*yyy)(char*,char*,char*,char*,char),char *derr) {
  return true;
}

////////////////////////////////////////////////////
