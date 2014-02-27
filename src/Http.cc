// http server
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: Http.cc
// description: http server for embedded tacppd

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"

#ifdef WITH_PERL
  #include <EXTERN.h>
  #include <perl.h>
//  PerlInterpreter *my_perl = NULL;
#endif //WITH_PERL

#define HTTP_USE_KOI8R 1

/////////// Basic HTTP functions ////////////

// constructor
HttpPacket::HttpPacket(int fh, struct in_addr addr):Packet(fh) {
  fid = fh;
}

// read string (before \n)
string
HttpPacket::hread() {
  unsigned char s[2];
  char buf[500];
  bzero(buf,sizeof(buf));
  int i=0;
  bzero(s,sizeof(s));
  while(sockread(s, 1, HTTP_TIMEOUT)>0 && i<500 &&
      s[0]!='\n' && s[0]!='\r') {
    buf[i++] = s[0];
    bzero(s,sizeof(s));
  }
  if(s[0]=='\r') sockread(s, 1, HTTP_TIMEOUT);
  string ostr(buf);
  return ostr;
}

// write line to client and add \n to end of line
bool
HttpPacket::hwrite(string str) {
  string s;
  s = str + "\r\n";
  if(sockwrite((unsigned char *)s.c_str(), s.size(), HTTP_TIMEOUT) > 0)
    return true;
  return false;
}

///////////////// HttpHeaderData ////////////

HttpHeaderData::HttpHeaderData() {
  bzero(method, sizeof(method));
  bzero(request, sizeof(request));
  bzero(User_Agent, sizeof(User_Agent));
  bzero(Host, sizeof(Host));
  bzero(Accept, sizeof(Accept));
  bzero(Accept_Encoding, sizeof(Accept_Encoding));
  bzero(Accept_Language, sizeof(Accept_Language));
  bzero(Accept_Charset, sizeof(Accept_Charset));
  bzero(Via, sizeof(Via));
  bzero(X_Forwarded_For, sizeof(X_Forwarded_For));
  bzero(Cache_Control, sizeof(Cache_Control));
  bzero(Connection, sizeof(Connection));
  bzero(Authorization, sizeof(Authorization));
  bzero(password, sizeof(password));
  bzero(username, sizeof(username));
  bzero(data_string, sizeof(data_string));
}


///////////////////// HttpHeader ////////////////////

// http date string
string
HttpHeader::http_date() {
  char str[40];
  const time_t t = time(NULL);
  char str1[40];
  ctime_r(&t,str1);
  //snprintf(str1,sizeof(str1)-1,"%s",ctime(&t));
  str1[strlen(str1)-1] = '\0';    // strip \n after ctime
  snprintf(str,sizeof(str)-1,"Date: %s",str1);
  string ostr(str);
  return ostr;
}

// send \n for end of output
void
HttpHeader::send_stop() {
  hwrite("");
}

// send html header
void
HttpHeader::header_html() {
  hwrite("HTTP/1.1 200 OK");
  hwrite((char*)http_date().c_str());
  hwrite("Server: tacppd/0.0.7");
  hwrite("Connection: close");
  hwrite("Content-type: text/html");
#ifdef HTTP_USE_KOI8R
  hwrite("Content-language: koi8-r");
#endif //HTTP_USE_KOI8R
//  hwrite("Vary: accept-charset, user-agent");
  send_stop();
}

// send perl http header
void
HttpHeader::header_perl() {
  hwrite("HTTP/1.1 200 OK");
  hwrite((char*)http_date().c_str());
#ifdef HTTP_USE_KOI8R
  hwrite("Content-language: koi8-r");
#endif //HTTP_USE_KOI8R
//  hwrite("Server: tacacs++ www-monitor");
//  hwrite("Connection: close");
//  hwrite("Content-language: ru");
//  hwrite("Vary: accept-charset, user-agent");
#ifndef WITH_PERL
  hwrite("Content-type: text/html");
  send_stop();
  hwrite("<HTML><BODY>No perl support in this build</BODY></HTML>");
#endif //WITH_PERL
}

// send gif header
void
HttpHeader::header_gif() {
  hwrite("HTTP/1.1 200 OK");
  hwrite((char*)http_date().c_str());
  hwrite("Content-type: image/gif");
  send_stop();
}

// send jpg header
void
HttpHeader::header_jpg() {
  hwrite("HTTP/1.1 200 OK");
  hwrite((char*)http_date().c_str());
  hwrite("Content-type: image/jpeg");
  send_stop();
}

// send java.class header
void
HttpHeader::header_class() {
  hwrite("HTTP/1.1 200 OK");
  hwrite((char*)http_date().c_str());
#ifdef HTTP_USE_KOI8R
  hwrite("Content-language: koi8-r");
#endif //HTTP_USE_KOI8R
  hwrite("Content-type: application/octet-stream");
  send_stop();
}

// send javascript.js header
void
HttpHeader::header_js() {
  hwrite("HTTP/1.1 200 OK");
  hwrite((char*)http_date().c_str());
#ifdef HTTP_USE_KOI8R
  hwrite("Content-language: koi8-r");
#endif //HTTP_USE_KOI8R
  hwrite("Content-type: application/x-javascript");
  send_stop();
}

// send .css header
void
HttpHeader::header_css() {
  hwrite("HTTP/1.1 200 OK");
  hwrite((char*)http_date().c_str());
#ifdef HTTP_USE_KOI8R
  hwrite("Content-language: koi8-r");
#endif //HTTP_USE_KOI8R
  hwrite("Content-type: text/css");
  send_stop();
}

// send authentication html header
void
HttpHeader::header_auth() {
  hwrite("HTTP/1.1 401 Authorization Required");
  hwrite((char*)http_date().c_str());
  hwrite("Server: tacppd/0.0.7");
  hwrite("WWW-Authenticate: Basic realm=\"www\"");
  hwrite("Connection: close");
  hwrite("Content-Type: text/html");
  send_stop();
}

// get and decode http request from browser
HttpHeaderData*
HttpHeader::get_request() {
  HttpHeaderData *d = new HttpHeaderData();
  bool post_method = false;
  char str[400];
  strncpy(str,hread().c_str(),sizeof(str)-1);
  while(strlen(str) > 0) {

cerr<<"read from client:"<<str<<std::endl;

    if(utl::scomp(str,"GET ",4)) {
      snprintf(d->method,sizeof(d->method)-1,"GET");
      int i=4, j=0;
      while(str[i]!='?' && str[i]!=' ' && str[i]!='\0' &&
           j<(int)sizeof(d->request)) {
        d->request[j++] = str[i];
        i++;
      }
      if(str[i] == '?') {   // present GET data
        i++; j=0;
        while(str[i]!=' ' && str[i]!='\0' &&
              j<(int)sizeof(d->data_string)) {
          d->data_string[j++] = str[i];
          i++;
        }
      }
    }
    if(utl::scomp(str,"POST ",5)) {
      snprintf(d->method,sizeof(d->method)-1,"POST");
      post_method = true;
      int i=5, j=0;
      while(str[i]!=' ' && str[i]!='\0' && j<(int)sizeof(d->request)) {
        d->request[j++] = str[i];
        i++;
      }
    }
    if(utl::scomp(str,"User-Agent: ",12))
      strncpy(d->User_Agent,str+12,sizeof(d->User_Agent)-1);
    if(utl::scomp(str,"Host: ",6))
      strncpy(d->Host,str+6,sizeof(d->Host)-1);
    if(utl::scomp(str,"Accept: ",8))
      strncpy(d->Accept,str+8,sizeof(d->Accept)-1);
    if(utl::scomp(str,"Accept-Encoding: ",17))
      strncpy(d->Accept_Encoding,str+17,sizeof(d->Accept_Encoding)-1);
    if(utl::scomp(str,"Accept-Language: ",17))
      strncpy(d->Accept_Language,str+17,sizeof(d->Accept_Language)-1);
    if(utl::scomp(str,"Accept-Charset: ",16))
      strncpy(d->Accept_Charset,str+16,sizeof(d->Accept_Charset)-1);
    if(utl::scomp(str,"Via: ",5))
      strncpy(d->Via,str+5,sizeof(d->Via)-1);
    if(utl::scomp(str,"X-Forwarded-For: ",17))
      strncpy(d->X_Forwarded_For,str+17,sizeof(d->X_Forwarded_For)-1);
    if(utl::scomp(str,"Cache-Control: ",15))
      strncpy(d->Cache_Control,str+15,sizeof(d->Cache_Control)-1);
    if(utl::scomp(str,"Connection: ",12))
      strncpy(d->Connection,str+12,sizeof(d->Connection)-1);
    if(utl::scomp(str,"Cookie: ",8)) {
      d->Cookie = (str+8);
cerr<<"Catch Cookie = "<<d->Cookie<<endl;
    }
    if(utl::scomp(str,"Authorization: ",15)) {
      strncpy(d->Authorization,str+15,sizeof(d->Authorization)-1);
      if(strlen(str) > 21) {
        char buf[255];
        bzero(buf,sizeof(buf));
        char user[HTTP_USERNAME_LEN];
        char pwd[HTTP_PASSWORD_LEN];
        bzero(user,sizeof(user));
        bzero(pwd,sizeof(pwd));
        string sss = utl::base64Decode(str+21);
        snprintf(buf,sizeof(buf)-1,"%s",sss.c_str());
        int i=0;
        for(;buf[i]!='\0' && buf[i]!=':'; i++) {
          user[i] = buf[i];
          user[i+1] ='\0';
        }
        i++;
        int j=0;
        for(;buf[i]!='\0'; i++) {
          pwd[j++] = buf[i];
          pwd[j]='\0';
        }
        strncpy(d->username,user,sizeof(d->username)-1);
        strncpy(d->password,pwd,sizeof(d->password)-1);
      }
    }
    strncpy(str, hread().c_str(), sizeof(str));
  }
  if(post_method) {
    strncpy(d->data_string,hread().c_str(),sizeof(d->data_string));
cerr<<"POST data_string = "<<d->data_string<<endl;
  }
  return(d);
}


////////////// HttpFile //////////////////

HttpFile::HttpFile(int fh,struct in_addr addr) : HttpHeader(fh,addr) {
  dir = 0;
}

// open file
bool
HttpFile::openfile(char *f) {
  int type = dir;
//if(dir == 0) return false;
//strncpy(html_file,f,sizeof(html_file)-1);
  string html_full_path;
  if(type == 0)
    html_full_path = html_full_path + ::MAINDIR + "/http" + f;
  else if(type == HTTP_ADMIN_DIR)
    html_full_path = html_full_path + ::MAINDIR + "/http/admin" + f;
  else
    html_full_path = html_full_path + ::MAINDIR + "/http/user" + f;
//std::cout<<"### Open file: "<<html_full_path.c_str()<<std::endl;
  htmlf.open(html_full_path.c_str(),std::ios::in);
  if(!htmlf) return false;
  return true;
}

// read file from disk and send it to tcp socket
void
HttpFile::readwrite() {
  char s[2];
  bzero(s,sizeof(s));
  while(htmlf.get(s[0]))
    sockwrite((unsigned char*)s, 1, HTTP_TIMEOUT);
}

// close file
void
HttpFile::closefile() {
  htmlf.close();
}

string
get_path_from_full(string s) {
  char str[300];
  snprintf(str,sizeof(str)-1,"%s",s.c_str());
  int len = strlen(str);
  while(str[len] != '/' && len>1) {
    str[len--] = 0;
  }
  str[len] = 0;
  string out = str;
//cerr<<"path for chdir = "<<out<<endl;
  return out;
}

// send html or binary file from disk without any analysys
void
HttpFile::send_file_from_file(char *file) {
//cerr<<"send_file_from_file="<<file<<endl;
  if(openfile(file)) {
    readwrite();
    closefile();
  } else send_nofile();
}

#ifdef WITH_PERL
// create Perl Dynamic Loader
//static void xs_init (pTHX);

EXTERN_C void boot_DynaLoader(pTHX_ CV* cv);
//EXTERN_C void boot_Socket(pTHX_ CV* cv);

EXTERN_C void
xs_init(pTHX) {
  char *file = __FILE__;
  // DynaLoader is a special case
  newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
//  newXS("Socket::bootstrap", boot_Socket, file);
}
#endif //WITH_PERL

#define HTTP_PARAMS_NUMBER	3
#define HTTP_ENV_NUMBER	    20

// run perl files
void
HttpFile::run_perl_file(char *file, HttpHeaderData *h) {

#ifdef WITH_PERL
  PerlInterpreter *my_perl = NULL;

  int type = dir;
  string perl_full_path;
  if(type == 0)
    perl_full_path = perl_full_path + ::MAINDIR + "/http" + file;
  else if(type == HTTP_ADMIN_DIR)
    perl_full_path = perl_full_path + ::MAINDIR + "/http/admin" + file;
  else
    perl_full_path = perl_full_path + ::MAINDIR + "/http/user" + file;

  // b) http_request
//  string http_request;
//  http_request = http_request + "request=" + h->request;
  // c) user_agent
//  string user_agent;
//  user_agent = user_agent + "agent=" + h->User_Agent;
  // d) host
//  string host;
//  host = host + "host=" + h->Host;
  // e) via
//  string via;
//  via = via + "via=" + h->Via;
  // f) x_forwarded_for
//  string x_forw;
//  x_forw = x_forw + "x_forw=" + h->X_Forwarded_For;
  // g) username
//  string usrn;
//  usrn = usrn + "user=" + h->username;

  // h) data
  string hdata;
  hdata = hdata + h->data_string;

  int http_params_num = 0;

  // 6. create perl request arguments
  char *args[HTTP_PARAMS_NUMBER];
  args[http_params_num++] = utl::newstr("tacppd");
  args[http_params_num++] = utl::newstr((char*)perl_full_path.c_str());

  if(utl::scomp(h->method,"GET"))
    args[http_params_num++] = utl::newstr((char*)hdata.c_str());
  args[http_params_num] = 0;

//  args[2] = utl::newstr((char*)http_request.c_str());
//  args[3] = utl::newstr((char*)user_agent.c_str());
//  args[4] = utl::newstr((char*)host.c_str());
//  args[5] = utl::newstr((char*)via.c_str());
//  args[6] = utl::newstr((char*)x_forw.c_str());
//  args[7] = utl::newstr((char*)usrn.c_str());
//  args[8] = utl::newstr((char*)hdata.c_str());
//  args[9] = 0;

  // set environment variables
  char *envs[HTTP_ENV_NUMBER];
  int http_env_num = 0;
  string str;

  //setenv("QUERY_STRING",(char*)hdata.c_str(),1);
  str = str + "QUERY_STRING=" + hdata;
  envs[http_env_num++] = utl::newstr((char*)str.c_str());
  str.erase();
  //setenv("SCRIPT_NAME",(char*)perl_full_path.c_str(),1);
  str = str + "SCRIPT_NAME=" + perl_full_path;
  envs[http_env_num++] = utl::newstr((char*)str.c_str());
  str.erase();
  //setenv("REQUEST_METHOD",h->method,1);
  str = str + "REQUEST_METHOD=" + h->method;
  envs[http_env_num++] = utl::newstr((char*)str.c_str());
  str.erase();
  //setenv("SERVER_SOFTWARE","tacppd/0.0.7",1);
  str = str + "SERVER_SOFTWARE=tacppd/0.0.7";
  envs[http_env_num++] = utl::newstr((char*)str.c_str());
  str.erase();

  //setenv("SERVER_NAME","10.1.1.1",1);

  //setenv("REMOTE_HOST",h->Host,1);
  str = str + "REMOTE_HOST=" + h->Host;
  envs[http_env_num++] = utl::newstr((char*)str.c_str());
  str.erase();

  //setenv("REMOTE_ADDR",,1);

  //setenv("AUTH_TYPE",h->Authorization,1);
  str = str + "AUTH_TYPE=" + h->Authorization;
  envs[http_env_num++] = utl::newstr((char*)str.c_str());
  str.erase();

  //setenv("REMOTE_USER",h->username,1);
  str = str + "REMOTE_USER=" + h->username;
  envs[http_env_num++] = utl::newstr((char*)str.c_str());
  str.erase();

  str = str + "HTTP_X_FORWARDED_FOR=" + h->X_Forwarded_For;
  envs[http_env_num++] = utl::newstr((char*)str.c_str());
  str.erase();

  //setenv("CONTENT_TYPE","application/x-www-form-urlencoded",1);
  str = str + "CONTENT_TYPE=application/x-www-form-urlencoded";
  envs[http_env_num++] = utl::newstr((char*)str.c_str());
  str.erase();

  //setenv("CONTENT_LENGTH",utl::toString(hdata.size()).c_str(),1);
  str = str + "CONTENT_LENGTH=" + utl::toString(hdata.size());
  envs[http_env_num++] = utl::newstr((char*)str.c_str());
  str.erase();

  if(h->Cookie.size() > 0) {
    str = str + "HTTP_COOKIE=" + h->Cookie;
cerr<<"Set variable "<<str<<endl;
    envs[http_env_num++] = utl::newstr((char*)str.c_str());
    str.erase();
  }

  envs[http_env_num] = 0;

//  if(utl::scomp(h->method,"POST"))
//    write(STDOUT_FILENO,hdata.c_str(),hdata.size());
//    cout<<hdata<<endl;

  // we should redirect output from stdout to connect descriptor

//std::cerr<<"CERR output before"<<std::endl;
//std::cout<<"COUT output before"<<std::endl;
  int o = dup(STDOUT_FILENO);
  int n = dup(fid);
  dup2(n,STDOUT_FILENO);

  int o2 = dup(STDIN_FILENO);
  int n2[2];
  pipe(n2);
  dup2(n2[0],STDIN_FILENO);

  // also we should write POST data to STDIN of this script when it run
//  if(utl::scomp(h->method,"POST")) {
//      PerlIO_write(PerlIO_stdin(),hdata.c_str(),hdata.size());
//      write(STDIN_FILENO,hdata.c_str(),hdata.size());
//  }

  chdir(get_path_from_full(perl_full_path).c_str());

  // execute perl-file
  //PerlInterpreter *my_perl = NULL;
  if((my_perl=perl_alloc()) == NULL) {
    std::cerr<<"No memory for Perl interpreter"<<std::endl;
    // !!!! this situation should be ended properly !!!!
  }
  //PerlInterpreter *my_perl = perl_alloc();
  perl_construct(my_perl);
std::cerr<<"Run perl file "<<perl_full_path<<std::endl;
  int parsestat = perl_parse(my_perl,xs_init,http_params_num,args,envs);

std::cerr<<"Perl parsestat = "<<parsestat<<std::endl;

  if(utl::scomp(h->method,"POST")) {
//    PerlIO_write(PerlIO_stdin(),hdata.c_str(),hdata.size());
    write(n2[1],hdata.c_str(),hdata.size());
  }
  if(!parsestat) {
    int runstat = perl_run(my_perl);
std::cerr<<"Perl runstat = "<<runstat<<std::endl;
    
  }
  perl_destruct(my_perl);
  perl_free(my_perl);
  // ok

  // return stdout back
  dup2(o,STDOUT_FILENO);

  // return stdin back
  dup2(o2,STDIN_FILENO);
  close(n2[0]);
  close(n2[1]);
  close(n);
  close(o);
  close(o2);

//std::cerr<<"CERR output after"<<std::endl;
//std::cout<<"COUT output after"<<std::endl;

  for(int i=0; args[i]!=0; i++) delete [] args[i];
  for(int i=0; envs[i]!=0; i++) delete [] envs[i];

#endif //WITH_PERL
}

// send file to client
void
HttpFile::send_file(char *file, HttpHeaderData *d) {
  if(utl::scomp(file+(strlen(file)-5),".html") ||
      utl::scomp(file+strlen(file)-4,".htm")) {
    header_html();
    send_file_from_file(file);
    send_stop();
    return;
  } else if(utl::scomp(file+(strlen(file)-4),".gif")) {
    header_gif();
    send_file_from_file(file);
    send_stop();
    return;
  } else if(utl::scomp(file+(strlen(file)-4),".jpg")) {
    header_jpg();
    send_file_from_file(file);
    send_stop();
    return;
  } else if(utl::scomp(file+(strlen(file)-4),".css")) {
    header_css();
    send_file_from_file(file);
    send_stop();
    return;
  } else if(utl::scomp(file+(strlen(file)-6),".class")) {
    header_class();
    send_file_from_file(file);
    send_stop();
    return;
  } else if(utl::scomp(file+(strlen(file)-3),".js")) {
    header_js();
    send_file_from_file(file);
    send_stop();
    return;
  }
#ifdef WITH_PERL
  else if(utl::scomp(file+(strlen(file)-3),".pl")) {
    if(d) {
      header_perl();
      run_perl_file(file,d);
      send_stop();
    }
    return;
  }
#endif //WITH_PERL
  send_nofile();
}

// send file to client without header
void
HttpFile::send_file_only(char *file) {
  if(utl::scomp(file+(strlen(file)-5),".html") ||
      utl::scomp(file+strlen(file)-4,".htm")) {
    send_file_from_file(file);
  }
#ifdef PERL_SUPPORT
  else if(utl::scomp(file+(strlen(file)-3),".pl")) {
    run_perl_file(file);
  }
#endif //PERL_SUPPORT
}

// send authentication request
void
HttpFile::send_authen() {
  header_auth();
  send_file_only(INVALID_AUTH_FILE);
  send_stop();
}

// send no such file
void
HttpFile::send_nofile() {
  HttpHeaderData *d = 0;
  if(openfile(INVALID_FILE)) {
    closefile();
    send_file(INVALID_FILE,d);
  } else {
    header_html();
    hwrite("<HTML>");
    hwrite("TACPPD internal error - no such file or directory");
    hwrite("</HTML>");
    send_stop();
  }
}


/////////////////// HttpServer Process //////////////////

HttpServer::HttpServer(int fh,struct in_addr addr) : HttpFile(fh,addr) {
    snprintf(address,sizeof(address)-1,"%s",inet_ntoa(addr));
}

// forms processor
//void
//HttpServer::process_forms(HttpHeaderData *d) {
//  if(strlen(d->data_string) == 0) {
//    send_nofile();
//    return;
//  }
//  char function[50];
//  strncpy(function,d->request+18,sizeof(function)-1);
//  startlog(LOG_HTTP)<<"Request to CGI function "<<function<<" with data "<<d->data_string<<endlog;
//  if(utl::scomp(function,"show-active-users")) {// function
//  }
//  send_nofile();
//}

// data string from forms processing
//struct formsdata*
//HttpServer::forms_data(char *str) {
//  if(!str || strlen(str)==0) return 0;
//  int num=1;
//  for(int i=0; str[i] != 0; i++) {
//    if(str[i]=='&') num++;
//  }
//  struct formsdata *ret;
//  ret = new formsdata[num+1];
//  int i=0;
//  char s[1032];
//  char s1[32];
//  char s2[1000];
//  int j=0;
//  int k=0;
//  int l=0;
//  while(str[i]!=0) {
//    j=0;
//    while(str[i]!='&' && str[i]!=0) {
//      s[j++]=s[i++];
//      s[j]=0;
//    }
//    j=0;
//    k=0;
//    while(s[j]!='=' && s[j]!=0) {
//      s1[k++]=s[j++];
//      s1[k]=0;
//    }
//    j++;
//    k = 0;
//    while(s[j]!=0) {
//      s2[k++]=s[j++];
//      s2[k]=0;
//    }
//    if(l<=num) {
//      ret[l].varname = utl::newstr(s1);
//      ret[l].data = utl::newstr(s2);
//      l++;
//      ret[l].varname=0;
//      ret[l].data=0;
//    }
//  }
//  return ret;
//}

// db_authentication external function
//extern bool
//db_authentication(char*,char*,char*,char*,char*);

// http authorization via database
bool
HttpServer::authentication(char *u,char *p,char *a,char *t) {
  return false;
}

// process http connections
void
HttpServer::process() {
  //char str[300];
  HttpHeaderData *d;

  //startlog(LOG_EVENT)<<"Now in Http::process"<<endlog;
  d = get_request();
  if(!d->username || strlen(d->username)==0 ||
      !d->password || strlen(d->password)==0) { // no user/pwd
    send_authen();
    delete d;
    return;
  }
//std::cout<<"GET request: "<<d->request<<" GET string: "<<d->data_string<<std::endl;
//std::cout<<"Received user/password line: "<<d->username<<"/"<<d->password<<std::endl;
  // we should check username/password
  if(!::coreData->getManager()->check(d->username,d->password,address,"http")) {
    // check by database
    if(!authentication(d->username,d->password,address,"http")) {
      startlog(LOG_INFORM)<<"Try to access to web monitor as user "<<d->username<<" with wrong avp"<<endlog;
      startlog(LOG_EVENT)<<"Access to web monitor as user "<<d->username<<" with wrong avp"<<endlog;
      send_authen();
      delete d;
      return;
    } else {
      dir = HTTP_USER_DIR;
    }
  } else {
    dir = HTTP_ADMIN_DIR;
  }
  if(d->request[strlen(d->request)-1] == '/') {
    char sss[200];
    bzero(sss,sizeof(sss));
    snprintf(sss,sizeof(sss)-1,"%sindex.html",d->request);
    //strcat(sss,d->request);
    //strcat(sss,"index.html");
    send_file(sss,d);
  } else {
    send_file(d->request,d);
  }
  delete d;
  return;
}

///////////////// that's all, folks /////////////////////
