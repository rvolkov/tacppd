// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: Utils.h
// description: set of some useful utilities

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

#ifndef __UTILS_H__
#define __UTILS_H__

#include "global.h"

namespace tacpp {

namespace utl {
  bool scomp(const char*, const char*);
  /**
  compare num symbols in 2 strings */
  bool scomp(const char*, const char*, int);
  /**
  lower all symbols for char* */
//  string lower(const char*);
  /**
  lower all symbols for string */
  string lower(const string);
  /**
  upper all symbols for char* type */
//  string upper(const char*);
  /**
  upper all symbols for string type */
  string upper(const string);
  /**
  allocate memory for new string by 'new' and copy data to it */
  char *newstr(char*);
  /**
  strip spaces around string */
//  char *strip(char*);
  /**
  convert time() to ISO datetime format */
  string time2iso(time_t);
  /**
  convert ISO string to time_t */
  time_t iso2time(const char*);
  /**
  convert time() to SQL datetime format */
  string time2sql(time_t);
  /**
  convert cisco's h323 time format from av-pair to ISO type */
  string h323time2iso(const char*);
  /**
  convert cisco's h323 time format from av-pair to traditional SQL type */
  /*char *h323time2sql(const char*);*/
  /**
  convert cisco's h323 time to time_t */
  time_t h323time2time(const char*);
  /**
  */
  int get_digit(const char*);
  /**
  */
  bool cron_parse(const char*,int);
  /**
  */
  bool check_time(const char*);
  /**
  */
  int getDigitFromIp(const char*,const int);
  /**
  */
  bool isIp(const char*);
  /**
  delete string by 'delete []'*/
  void delstr(char*);
  /**
  compare attr with mask */
  bool match(const char*,const char*);
  /**
  */
  bool matchNstrict(const char *b,const char *mask);
  /**
  compare parametr value with parametr description*/    
  bool isCorrespondW(const char * pDesc,const char * str);
  /**
  compare value with mask, return compared value if any math present */
  string vmatch(const char*,const char*);
  /**
  split service id  name@host:port or host:port to elements */
  string splitId(const char*,const int);
  /**
  split string with ' ' as split symbol */
  string split(const char*,const int);
  /**
  get AVpair name */
  string getAvpName(const char*);
  /**
  get AVpair value */
  string getAvpValue(const char*);
  /**
  get AVpair modifyer value */
  string getAvpMod(const char*);
  /**
  insert symbol to position */
  //char *inssym(const char*,char,int);
  /**
  remove symbol from position */
  //char *delsym(const char*,int);
  /**
  encode string to base64 */
  string base64Encode(const char*);
  /**
  decode string from base64 */
  string base64Decode(const char*);
  /**
  convert int to string */
  string toString(const int);
  /**
  convert uint to string */
  string toString(const unsigned int);
  /**
  convert ulong to string */
  string toString(const unsigned long);
  /**
  convert float to string */
  string toString(const float);
  /**
  convert double to string */
  string toString(const double);
  /**
   */
  string toString(time_t,int);
  /**
   */
  string toSQL(const string);
  /**
   */
  string portConvert(const string);
  /**
  show ip addr in string from Bit32 */
  string sip(const long unsigned int);
}; // utl

}; // tacpp

#endif // __UTILS_H__
