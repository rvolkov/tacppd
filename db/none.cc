// DB module for NULL engine
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// look at http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// Support will not be provided under any circumstances for this program by
// tacppd.com, it's employees, volunteers or directors, unless a commercial
// support agreement and/or explicit prior written consent has been given.
// Visit http://tacppd.com for more information

// modifications:
//

#include "ipaddr.h"
#include "db_module.h"

extern "C" {
  void db_init(); // init database
  void db_destroy(); // destroy database data
  int db_open(char*,int,char*,char*,char*,char*); // open database
  bool db_close(int); // close database
  bool db_query(int,DbTable*,char*);  // send query
  char *db_function(int,char*,char*); // call db function
  bool db_get(int,DbTable*,int,char*); // get information
  bool db_create(int,DbTable*,char*); // create table
  bool db_create_idx(int,DbTable*,char*); // create table index
  bool db_add(int,DbTable*,char*); // add information to db table
  bool db_update(int,DbTable*,char*); // update db table
  bool db_del(int,DbTable*,char*);  // delete entry from database
}

void db_init() {}

void db_destroy() {}

int db_open(char *a,int b,char *c,char *d,char *e,char *f) {
	return 0;
}
bool db_close(int i) {
	return true;
}
bool db_query(int i,DbTable *a,char *b) {
	return true;
}
char *db_function(int i,char *a,char *b) {
  return "";
}
bool db_get(int i,DbTable *a,int b,char *c) {
	return true;
}
bool db_create(int i,DbTable *a,char *b) {
	return true;
}
bool db_create_idx(int i,DbTable *a,char *b) {
	return true;
}
bool db_add(int i,DbTable *a,char *b) {
	return true;
}
bool db_update(int i,DbTable *a,char *b) {
	return true;
}
bool db_del(int i,DbTable *a,char *b) {
	return true;
}

/********************************************************/
