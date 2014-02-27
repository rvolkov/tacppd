// tacppd config and core classes information
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

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

#ifndef __COREDATA_H__
#define __COREDATA_H__

#include "global.h"

namespace tacpp {

const string DEBUG_TYPE = "DEbug";
class CoreDebug;
const string ACCESS_TYPE = "ACCess";
class CoreAccess;
const string POOL_TYPE = "POOl";
class CorePool;
const string LISTENER_TYPE = "LISTener";
class CoreListener;
const string MANAGER_TYPE = "MANager";
class CoreManager;
const string DEVICE_TYPE = "DEVice";
class CoreDevice;
const string DB_TYPE = "DATabase";
class CoreDb;
const string BILLING_TYPE = "BILling";
class CoreBilling;
const string BUNDLE_TYPE = "BUNdle";
class CoreBundle;
const string PEER_TYPE = "PEer";
class CorePeer;

#define CORE_NAME_LEN       30
#define CORE_DESCR_LEN      50

/**
@short universal data
@version 1
@author rv
*/
class Core_data {
  /**
  thread locker */
  pthread_mutex_t Mutex_;
  /**
  set thread lock */
  void lock();
  /**
  remove thread lock */
  void unlock();
 public:
  /**
  set all ptr to 0 */
  void zero_all();
  /**
  ptr to access descr */
  CoreAccess    *coreAccess;
  /**
  ptr to device descr */
  CoreDevice    *coreDevice;
  /**
  ptr to deb system */
  CoreDebug     *coreDebug;
  /**
  ptr */
  CoreManager   *coreManager;
  /**
  ptr */
  CorePool      *corePool;
  /**
  ptr */
  CoreDb        *coreDb;
  /**
  ptr */
  CoreListener  *coreListener;
  /**
  ptr */
  CoreBundle    *coreBundle;
  /**
  ptr */
  CoreBilling   *coreBilling;
  /**
  ptr */
  CorePeer      *corePeer;
  /**
  CoreAccess constructor */
  Core_data(const string,const string,CoreAccess*);
  /**
  CoreDevice constructor */
  Core_data(const string,const string,CoreDevice*);
  /**
  CoreDebug constructor */
  Core_data(const string,const string,CoreDebug*);
  /**
  CoreManager constructor */
  Core_data(const string,const string,CoreManager*);
  /**
  CorePool constructor */
  Core_data(const string,const string,CorePool*);
  /**
  CoreDb constructor */
  Core_data(const string,const string,CoreDb*);
  /**
  CoreListener constructor */
  Core_data(const string,const string,CoreListener*);
  /**
  CoreBundle constructor */
  Core_data(const string,const string,CoreBundle*);
  /**
  CoreBilling constructor */
  Core_data(const string,const string,CoreBilling*);
  /**
  CorePeer constructor */
  Core_data(const string,const string,CorePeer*);
  /**
  virtual destructor (this is abstract class) */
  virtual ~Core_data();
  /**
  name of core component */
  string name;
  /**
  description of core component */
  string descr;
  /**
  build whole config */
  string buildcf();
  /**
  build config string */
  virtual string buildcfstr(int)=0;
  /**
  apply config */
  virtual string applycf(char * const*mask,const string cmdname,const list<string>paramList,Command * cmd=NULL)throw (CmdErr)=0;
  /**
  build configuration for one type */
  string buildcf(int,string);
};

/**
@short data handler
@version 1
*/
class CoreData {
  /**
  core objects list */
  list<Core_data*> core_;
  /**
  pthread locker */
  pthread_mutex_t Mutex_;
  /**
  set lock */
  void lock();
  /**
  delete lock */
  void unlock();
 public:
  /**
  init */
  CoreData();
  /**
  free */
  ~CoreData();
  /**
  sizeof */
  int size();
  /**
  get object by number */
  Core_data *get(int);
  /**
  get by object type name */
  Core_data *get(const char*);
  Core_data *get(const string);
  /**
  get name from number */
  string getn(int);
  /**
  show configuration */
  string showcf(char*);
  /**
  write configuration to file */
  char *writecf(char*,char*);
  /**
  brief help about all objects */
  string help();
  /**
   */
  CoreAccess *getAccess();
  /**
   */
  CoreDevice *getDevice();
  /**
   */
  CoreDebug *getDebug();
  /**
   */
  CoreManager *getManager();
  /**
   */
  CorePool *getPool();
  /**
   */
  CoreDb *getDb();
  /**
  return CoreListener data */
  CoreListener *getListener();
  /**
   */
  CoreBundle *getBundle();
  /**
   */
  CoreBilling *getBilling();
  /**
   */
  CorePeer *getPeer();
};

};

#endif //__COREDATA_H__
