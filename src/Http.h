// http server
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

#ifndef __HTTP_H__
#define __HTTP_H__

#include "global.h"

namespace tacpp {

// place for http dir
#define HTTP_DIR	"http"
// connect timeout
#define HTTP_TIMEOUT	2
// default filenames
#define INVALID_AUTH_FILE   "/invalid_auth.html"
#define INVALID_FILE        "/invalid_file.html"

// types of users and it's directories
#define HTTP_ADMIN_DIR    1
#define HTTP_USER_DIR     2

// maximum fields in form
#define MAX_FORM_FIELDS   20

/**
@short http header data
@version 1
@author rv
*/
class HttpHeaderData {
 public:
  /**
  init */
  HttpHeaderData();
#define HTTP_METHOD_LEN       20
  /**
  method */
  char method[HTTP_METHOD_LEN];
#define HTTP_REQUEST_LEN      100
  /**
  request */
  char request[HTTP_REQUEST_LEN];
#define USER_AGENT_LEN        80
  /**
  user agent */
  char User_Agent[USER_AGENT_LEN];
#define HTTP_HOST_LEN         30
  /**
  host */
  char Host[HTTP_HOST_LEN];
#define ACCEPT_LEN            300
  /**
  accept */
  char Accept[ACCEPT_LEN];
  /**
  accept encoding */
#define ACCEPT_ENC_LEN        10
  char Accept_Encoding[ACCEPT_ENC_LEN];
  /**
  accept language */
#define ACCEPT_LAN_LEN        10
  char Accept_Language[ACCEPT_LAN_LEN];
  /**
  accept charset */
#define ACCEPT_CHR_LEN        20
  char Accept_Charset[ACCEPT_CHR_LEN];
  /**
  via */
#define VIA_LEN               80
  char Via[VIA_LEN];
  /**
  forwarded for */
#define X_FORW_LEN            30
  char X_Forwarded_For[X_FORW_LEN];
  /**
  cache control */
#define CACHE_CONT_LEN        20
  char Cache_Control[CACHE_CONT_LEN];
  /**
  connection */
#define CONNECTION_LEN        30
  char Connection[CONNECTION_LEN];
  /**
  authoriz */
#define AUTHOR_LEN            30
  char Authorization[AUTHOR_LEN];
  /**
  username */
#define HTTP_USERNAME_LEN     50
  char username[HTTP_USERNAME_LEN];
  /**
  pwd */
#define HTTP_PASSWORD_LEN     50
  char password[HTTP_PASSWORD_LEN];
#define HTTP_DATA_LEN         200
  /**
  data */
  char data_string[HTTP_DATA_LEN];
  /**
  Cookie */
  string Cookie;
};

/**
@short http packet
@version 1
@author rv
*/
class HttpPacket : public Packet {
 public:
  /**
  socket descr */
  int fid;
  /**
  init */
  HttpPacket(int,struct in_addr);
  /**
  read http packet */
  string hread();
  /**
  write http packet */
  bool hwrite(string);
};

/**
@short http header processing
@version 1
@author rv
*/
class HttpHeader : public HttpPacket {
  /**
  http "Date:value" string */
  string http_date();
 public:
  /**
  init */
  HttpHeader(int fh,struct in_addr addr) : HttpPacket(fh, addr) {}
  /**
  receive req */
  HttpHeaderData *get_request();
  /**
  send html header */
  void header_html();
  /**
  send gif header */
  void header_gif();
  /**
  send jpg header */
  void header_jpg();
  /**
  send authen request header */
  void header_auth();
  /**
  send striped header for perl out */
  void header_perl();
  /**
  send java.class header (application/octet-stream) */
  void header_class();
  /**
  send .css header (text/css) */
  void header_css();
  /**
  send .js header (application/x-javascript) */
  void header_js();
  /**
  send \r\n to client */
  void send_stop();
};

/**
@short data from form
@version 1
@author rv
*/
struct formsdata {
  char *varname;
  char *data;
};

/**
@short http file processing
@version 1
@author rv
*/
class HttpFile : public HttpHeader {
  /**
  ifstream */
  std::ifstream htmlf;
  /**
  open file */
  bool openfile(char*);
  /**
  read file and send it */
  void readwrite();
  /**
  close file */
  void closefile();
  /**
  send */
  void send_file_from_file(char*);
  /**
  output from perl */
  void run_perl_file(char*,HttpHeaderData*);
 public:
  /**
  dir */
  int dir;
  /**
  init */
  HttpFile(int,struct in_addr);
  /**
  send file */
  void send_file(char*,HttpHeaderData*);
  /**
  send file without header */
  void send_file_only(char*);
  /**
  send authen request */
  void send_authen();
  /**
  send no this file response */
  void send_nofile();
};

/**
@short http server
@version 1
@author rv
*/
class HttpServer : public HttpFile {
  /**
  addr */
  char address[30];
 public:
  /**
  init */
  HttpServer(int,struct in_addr);
  /**
  auth */
  bool authentication(char*,char*,char*,char*);
  /**
  proc */
  void process();
};

};

#endif //__HTTP_H__
