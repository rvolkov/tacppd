// low-level modules dynamic load
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: DLoad.cc
// description: dynamic loading

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"

// constructor
DLoad::DLoad() {
  handle = 0;
}

// loader
string
DLoad::load(const char *module) {
  char *error = 0;
  string errm;
  handle = dlopen(module,RTLD_NOW);
  //handle = dlopen(module,RTLD_GLOBAL);
  if((error = (char*)dlerror())) {
    errm = errm + "DSO: " + error;
    return errm;
  }
  return "";
}

// db loader
string
DLoad::dbload(const char *module) {
  string modpath;
  modpath = modpath + ::MAINDIR + "/" + PATH_TO_DB_MODULES + "/" + module;
  return(load((char*)modpath.c_str()));
}

// snmp loader
string
DLoad::snmpload(const char *module) {
  string modpath;
//std::cout<<"try to load "<<module<<std::endl;
  modpath = modpath + ::MAINDIR + "/" + PATH_TO_SNMP_MODULES + "/" + module;
  return(load((char*)modpath.c_str()));
}

// billing loader
string
DLoad::bilload(const char *module) {
  string modpath;
//std::cout<<"try to load "<<module<<std::endl;
  modpath = modpath + ::MAINDIR + "/" + PATH_TO_BILLING_MODULES + "/" + module;
  return(load((char*)modpath.c_str()));
}

// destructor
DLoad::~DLoad() {
  if(handle != 0) dlclose(handle);
}

// get pointer to loaded function
// the bug can be here - error doesn't return to config terminal
void*
DLoad::getfunc(const char *FuncName) {
  char *error = 0;
  void *h;
  h = dlsym(handle,FuncName);
  if((error = (char*)dlerror())) {
    startlog(LOG_ERROR)<<"Dlsym err: "<<error<<endlog;
    return 0;
  }
  return h;
}

/////////////// that's all ////////////////
