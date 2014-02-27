// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2004 by tacppd team and contributors
// see http://tacppd.org for more information
//
// file: Snmp.h
// description: header file for SNMP functions

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

/*
This software use NET-SNMP package which have next notice:

Various copyrights apply to this package, listed in 2 seperate parts
below.  Please make sure that you read both parts.  The first part is
the copyright for all the code up until 2001, and the second part
applies to the derivative work done since then.

---- Part 1: CMU/UCD copyright notice: (BSD like) -----


       Copyright 1989, 1991, 1992 by Carnegie Mellon University

		  Derivative Work - 1996, 1998-2000
Copyright 1996, 1998-2000 The Regents of the University of California

			 All Rights Reserved

Permission to use, copy, modify and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU and The Regents of
the University of California not be used in advertising or publicity
pertaining to distribution of the software without specific written
permission.

CMU AND THE REGENTS OF THE UNIVERSITY OF CALIFORNIA DISCLAIM ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL CMU OR
THE REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
FROM THE LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


---- Part 2: NAI copyright notice (BSD) -----

Copyright (c) 2001, NAI Labs
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

*  Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

*  Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

*  Neither the name of the NAI Labs nor the names of its contributors
   may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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

#ifndef __SNMP_H__
#define __SNMP_H__

#include "global.h"
#include "snmp_module.h"

// this is for working with UCD-SNMP/NET-SNMP functions
extern "C" {
#ifdef USEUCDSNMP
  #include <ucd-snmp/ucd-snmp-config.h>
  #include <ucd-snmp/ucd-snmp-includes.h>
  //extern struct tree *tree_head; // it was need for old UCD-SNMP
  //static int quick_print = 0;
#endif /*USEUCDSNMP*/
#ifdef USENETSNMP
  #include <net-snmp/net-snmp-config.h>
  //#include <net-snmp/utilities.h>
  #include <net-snmp/net-snmp-includes.h>
#endif /*USENETSNMP*/
}

#define SNMP_COMMUN_LEN   30
#define SNMP_TYPE_LEN     20

//SnmpD *snmp_get(char*,char*,char*);
//bool snmp_puts(char*,char*,char*,char*,char);


namespace tacpp {

class Device_data;

class Snmp {
  /**
  thread locker */
  pthread_mutex_t Mutex;
  /**
  set thread lock */
  void lock();
  /**
  remove thread lock */
  void unlock();
 public:
  /**
  counter */
  int count;
  /**
  init */
  Snmp();
  /**
  free */
  ~Snmp();
  /**
  get users list */
  SnmpDevice *get(Device_data*);
  /**
  drop port */
  bool drop(char*,char*);
  /**
  store traffic into database */
  void store_traffic(Device_data*,SnmpDevice*);
};

};

#endif //__SNMP_H__
