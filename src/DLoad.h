// low-level modules dynamic load
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

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

#ifndef __DLOAD_H__
#define __DLOAD_H__

#include "global.h"

#define PATH_TO_SNMP_MODULES      "snmp"
#define PATH_TO_BILLING_MODULES   "resrc"

namespace tacpp {

/**
@short dynamic loading
@version 1
@author rv
*/
class DLoad {
  /**
  handle */
  void *handle;
 public:
  /**
  init */
  DLoad();
  /**
  common module load */
  string load(const char*);
  /**
  load database module */
  string dbload(const char*);
  /**
  load snmp module */
  string snmpload(const char*);
  /**
  load billing module */
  string bilload(const char*);
  /**
  free */
  ~DLoad();
  /**
  should return pointer */
  void *getfunc(const char *FuncName);
};

};

#endif //__DLOAD_H__
