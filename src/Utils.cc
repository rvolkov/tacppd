// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: Utils.cc
// description: several useful utilities which i can't insert in other places

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"

// compare 2 strings (can compare NULLs)
bool
tacpp::utl::scomp(const char *s1,const char *s2) {
  if(!s1 && !s2) return true;
  if(!s1 || !s2) return false;
  if(strcmp(s1,s2) == 0) return true;
  return false;
}

string
tacpp::utl::lower(const string s) {
  char str[800];
  snprintf(str,sizeof(str)-1,"%s",s.c_str());
  for(int i=0; str[i]; i++) {
    str[i] = (char)tolower((int)str[i]);
  }
  string sout(str);
  return sout;
}

string
tacpp::utl::upper(const string s) {
  char str[800];
  snprintf(str,sizeof(str)-1,"%s",s.c_str());
  for(int i=0; str[i]; i++) {
    str[i] = (char)toupper((int)str[i]);
  }
  string ostr(str);
  return ostr;
}

// compare number of chars in strings
bool
tacpp::utl::scomp(const char *s1,const char *s2,int num) {
  if(!s1 && !s2) return true;
  if(!s1 || !s2) return false;
  if((int)strlen(s1)<num || (int)strlen(s2)<num) return false;
  if(strncmp(s1,s2,num) == 0) return true;
  return false;
}

// the strdup clone with new operator
char*
tacpp::utl::newstr(char *s) {
  if(!s) return NULL;
  char *uuu;
  if(strlen(s) == 0) {
    uuu = new char[1];
    uuu[0] = '\0';
  } else {
    uuu = new char[strlen(s)+1];
    bzero(uuu,strlen(s)+1);
    strcpy(uuu,s);
  }
  return uuu;
}

// str delete with setting NULL
void
tacpp::utl::delstr(char *s) {
  if(!s) return;
  delete [] s;
  s = 0;
}

// strip spaces
//char*
//tacpp::utl::strip(char *str) {
//  if(str == NULL) return NULL;
//  int i = strlen(str);
//  while(str[i]==' ' || str[i]=='\0') str[i--]='\0';
//  return str;
//}

string
tacpp::utl::time2iso(time_t t) {
  struct tm *ttt;
  struct tm res;
  ttt = localtime_r(&t,&res);
  char str[64];
  bzero(str,sizeof(str));
  snprintf(str,sizeof(str)-1,"%04d-%02d-%02d %02d:%02d:%02d",
    1900+ttt->tm_year,ttt->tm_mon+1,ttt->tm_mday,
      ttt->tm_hour,ttt->tm_min,ttt->tm_sec);
  string outs(str);
  return outs;
}

// convert ISO datetime to time() ticks
// (2001-03-24 10:30:05)
time_t
tacpp::utl::iso2time(const char *iso) {
  if(!iso) return 0;
  if(strlen(iso) < 8) return 0;
  char str[20];
  struct tm ttt;
  int str_len = sizeof(str);

  // read year
  int k = 0;
  for(int i=0; i<4 && iso[i] && k<str_len; i++,k++) {
    str[k] = iso[i];
  }
  str[k] = 0;
  ttt.tm_year = atoi(str) - 1900;

  // read month
  k = 0;
  for(int i=5; i<7 && iso[i] && k<str_len; i++,k++) {
    str[k] = iso[i];
  }
  str[k] = 0;
  ttt.tm_mon = atoi(str) - 1;

  // read day
  k = 0;
  for(int i=8; i<10 && iso[i] && k<str_len; i++,k++) {
    str[k] = iso[i];
  }
  str[k] = 0;
  ttt.tm_mday = atoi(str);

  // read hour
  k = 0;
  for(int i=11; i<13 && iso[i] && k<str_len; i++,k++) {
    str[k] = iso[i];
  }
  str[k] = 0;
  ttt.tm_hour = atoi(str);

  // read minutes
  k = 0;
  for(int i=14; i<16 && iso[i] && k<str_len; i++,k++) {
    str[k] = iso[i];
  }
  str[k] = 0;
  ttt.tm_min = atoi(str);

  // read seconds
  k = 0;
  for(int i=17; i<19 && iso[i] && k<str_len; i++,k++) {
    str[k] = iso[i];
  }
  str[k] = 0;
  ttt.tm_sec = atoi(str);

  return mktime(&ttt);
}

string
tacpp::utl::time2sql(time_t t) {
  struct tm *ttt;
  struct tm res;
  ttt = localtime_r(&t,&res);
  char str[64];
  bzero(str,sizeof(str));
  char tz[10];

  bzero(tz,sizeof(tz));
#ifndef SOLARIS
#ifdef FREEBSD
  strncpy(tz,ttt->tm_zone,sizeof(tz)-1);
#else
#ifdef __USE_BSD
  strncpy(tz,ttt->tm_zone,sizeof(tz)-1);
#else
  strncpy(tz,ttt->__tm_zone,sizeof(tz)-1);
#endif //__USE_BSD
#endif //FREEBSD
#endif //SOLARIS
  snprintf(str,sizeof(str)-1,"%02d/%02d/%04d %02d:%02d:%02d %s",
    ttt->tm_mon+1,ttt->tm_mday,1900+ttt->tm_year,
    ttt->tm_hour,ttt->tm_min,ttt->tm_sec,tz);
  string outs(str);
  return outs;
}

// convert datetime formats
string
tacpp::utl::h323time2iso(const char *s) {
  if(!s || strlen(s)<6) return "1980-01-01 00:00:00";
  char str[64];
  char t[10];
  char m[4];
  char d[3];
  char y[5];
  int i;
  int mon = 0;

  bzero(str,sizeof(str));
  bzero(t,sizeof(t));
  bzero(m,sizeof(m));
  bzero(d,sizeof(d));
  bzero(y,sizeof(y));
  for(i=0; i < 8 && s[i]; i++) {
    t[i] = s[i];
  }
  i = i + 13;
  for(int j=0; j < 3 && s[i]; j++) {
    m[j] = s[i++];
  }
  i++;
  for(int j=0; s[i]!=' ' && j<2  && s[i]; j++) {
    d[j] = s[i++];
  }
  i++;
  for(int j=0; j < 4 && s[i]; j++) {
    y[j] = s[i++];
  }

  if(scomp(m,"Jan")) mon = 1;
  else if(scomp(m,"Feb")) mon = 2;
  else if(scomp(m,"Mar")) mon = 3;
  else if(scomp(m,"Apr")) mon = 4;
  else if(scomp(m,"May")) mon = 5;
  else if(scomp(m,"Jun")) mon = 6;
  else if(scomp(m,"Jul")) mon = 7;
  else if(scomp(m,"Aug")) mon = 8;
  else if(scomp(m,"Sep")) mon = 9;
  else if(scomp(m,"Oct")) mon = 10;
  else if(scomp(m,"Nov")) mon = 11;
  else if(scomp(m,"Dec")) mon = 12;

  int day = atoi(d);
  snprintf(str,sizeof(str)-1,"%s-%02d-%02d %s",y,mon,day,t);

  string outs(str);
  return outs;
}

// convert cisco's h323 time format from av-pair to traditional SQL type
/*
char*
tacpp::utl::h323time2sql(const char *s) {
  if(!s || strlen(s) == 0) return "";
  static char str[64];
  bzero(str,sizeof(str));
  char t[10];
  char m[4];
  char d[3];
  char y[5];
  char tz[6];
  int i;
  int mon;

  bzero(tz, sizeof(tz)-1);
  bzero(t, sizeof(t));
  bzero(m, sizeof(m));
  bzero(d, sizeof(d));
  bzero(y, sizeof(y));
  for(i=0; i < 8; i++) t[i]=s[i];
  i = i + 5;
  for(int j=0; j < 3; j++) tz[j]=s[i++];
  i = i + 5;
  for(int j=0; j < 3; j++) m[j]=s[i++];
  i++;
  for(int j=0; j < 2; j++) d[j]=s[i++];
  i++;
  for(int j=0; j < 4; j++) y[j]=s[i++];

  if(scomp(m,"Jan")) mon = 1;
  else if(scomp(m,"Feb")) mon = 2;
  else if(scomp(m,"Mar")) mon = 3;
  else if(scomp(m,"Apr")) mon = 4;
  else if(scomp(m,"May")) mon = 5;
  else if(scomp(m,"Jun")) mon = 6;
  else if(scomp(m,"Jul")) mon = 7;
  else if(scomp(m,"Aug")) mon = 8;
  else if(scomp(m,"Sep")) mon = 9;
  else if(scomp(m,"Oct")) mon = 10;
  else if(scomp(m,"Nov")) mon = 11;
  else if(scomp(m,"Dec")) mon = 12;

  snprintf(str,sizeof(str)-1,"%02d/%s/%s %s %s",mon,d,y,t,tz);
  return str;
}
*/

// convert cisco's h323 datetime format to time_t value
// h323="14:31:14.905 VSK Tue Nov 6 2001"
time_t
tacpp::utl::h323time2time(const char *s) {
  if(!s) return 0;
  if(strlen(s) < 5) return 0;
//std::cerr<<"h323time2time="<<s<<std::endl;
  char str[40];
  struct tm ttt;
  bzero((void*)&ttt,sizeof(struct tm));
  int k;
  int i = 0;
  int str_size = sizeof(str);

  // read hour
  k = 0;
  for(; s[i] && s[i]!=':' && k<str_size; i++,k++) {
    str[k] = s[i];
  }
  str[k] = 0;
  ttt.tm_hour = atoi(str);
  if(s[i]) i++;

  // read minutes
  k = 0;
  for(; s[i] && s[i]!=':' && k<str_size; i++,k++) {
    str[k] = s[i];
  }
  str[k] = 0;	
  ttt.tm_min = atoi(str);
  if(s[i]) i++;

  // read seconds
  k = 0;
  for(; s[i] && s[i]!='.' && k<str_size; i++,k++) {
    str[k] = s[i];
  }
  str[k] = 0;	
  ttt.tm_sec = atoi(str);
  if(s[i]) i++;

  // drop ms, timezone, day of week
  for(; s[i] && s[i]!=' '; i++);// ms
  if(s[i]) i++;
  for(; s[i] && s[i]!=' '; i++);// timezone
  if(s[i]) i++;
  for(; s[i] && s[i]!=' '; i++);// day of week
  if(s[i]) i++;

  // read month
  k = 0;
  for(; s[i] && s[i]!=' ' && k<str_size; i++,k++) {
    str[k] = s[i];
  }
  str[k] = 0;
  int mon = 0;
  if(scomp(str,"Jan")) mon = 1;
  else if(scomp(str,"Feb")) mon = 2;
  else if(scomp(str,"Mar")) mon = 3;
  else if(scomp(str,"Apr")) mon = 4;
  else if(scomp(str,"May")) mon = 5;
  else if(scomp(str,"Jun")) mon = 6;
  else if(scomp(str,"Jul")) mon = 7;
  else if(scomp(str,"Aug")) mon = 8;
  else if(scomp(str,"Sep")) mon = 9;
  else if(scomp(str,"Oct")) mon = 10;
  else if(scomp(str,"Nov")) mon = 11;
  else if(scomp(str,"Dec")) mon = 12;
  ttt.tm_mon = mon;

  // read day
  k = 0;
  for(; s[i] && s[i]!=' ' && k<str_size; i++,k++) {
    str[k] = s[i];
  }
  str[k] = 0;
  ttt.tm_mday = atoi(str);

  // read year
  k = 0;
  for(; s[i] && k<str_size; i++,k++) {
    str[k] = s[i];
  }
  str[k] = 0;
  ttt.tm_year = atoi(str) - 1900;
  return mktime(&ttt);
}

// convert "Async" to "As", "Serial" to "Se" and back
string
tacpp::utl::portConvert(const string in) {
  if(in.size() == 0) return "";
  char port[100];
  bzero(port,sizeof(port));
  char p[100];
  bzero(p,sizeof(p));
  snprintf(p,sizeof(p)-1,"%s",in.c_str());
  snprintf(port,sizeof(port)-1,"%s",p);
  if(scomp(p,"Serial",6))
    snprintf(port,sizeof(port)-1,"Se%s",p+6); // Serial
  else if(scomp(p,"Async",5))
    snprintf(port,sizeof(port)-1,"As%s",p+5); // Async
//  else if(scomp(p,"FXO",3))
//    snprintf(port,sizeof(port)-1,"FXO%s",p+4); // FXO
  if(scomp(p,"Se",2) && !scomp(p,"Seri",4))
    snprintf(port,sizeof(port)-1,"Serial%s",p+2); // Serial
  else if(scomp(p,"As",2) && !scomp(p,"Asyn",4))
    snprintf(port,sizeof(port)-1,"Async%s",p+2); // Async
  string sout = port;
  return sout;
}

// get digit
int
tacpp::utl::get_digit(const char *str) {
  int i;
  i=0;
  if(!str) return 0;
  if(strlen(str) == 0) return 0;
  char dig[20];
  while(isdigit(str[i]) && i<20) {
    dig[i] = str[i];
    i++;
  }
  dig[i] = '\0';
  return(atoi(dig));
}

// parse cron-style single entry
bool
tacpp::utl::cron_parse(const char *entry, int curtime) {
  if(!entry) return false;
  if(strlen(entry) == 0) return false;
  if(entry[0] == '*') return true;  // single * - return true
  int i = 0;
  int l = strlen(entry);

  while(i < l && entry[i] != '\0') {
    int dig = get_digit(entry+i);
    if(curtime == dig) return true; // match one of digits
    while(isdigit(*(entry+i))) i++;

    if(*(entry+i) == '-') {         // range of values
      i++;
      int dig2 = get_digit(entry+i);
      while(isdigit(*(entry+i))) i++;
      if(curtime >= dig && curtime <= dig2) return true;	
    }
    if(*(entry+i) == ',') i++;
  }
  return false;
}

// check currenttime with database template
bool
tacpp::utl::check_time(const char *entry) {
  if(!entry) return true;
  if(strlen(entry) < 3) return true;
  char c_min[100];
  char c_hour[100];
  char c_day[100];
  char c_month[100];
  char c_weekday[100];

  int i = 0;
  int j = 0;
  while(entry[i] != ' ' && j < (int)sizeof(c_min)) { // minutes
    c_min[j++] = entry[i++];
    c_min[j] = '\0';
  }
  while(entry[i] == ' ') i++;                   // spaces
  j = 0;
  while(entry[i]!=' ' && j<(int)sizeof(c_hour)) {    // hours
    c_hour[j++] = entry[i++];
    c_hour[j] = '\0';
  }
  while(entry[i] == ' ') i++;                   // spaces
  j = 0;
  while(entry[i]!=' ' && j<(int)sizeof(c_day)) {     // days
    c_day[j++] = entry[i++];
    c_day[j] = '\0';
  }
  while(entry[i] == ' ') i++;                   // spaces
  j = 0;
  while(entry[i]!=' ' && j<(int)sizeof(c_month)) {   // monthes
    c_month[j++] = entry[i++];
    c_month[j] = '\0';
  }
  while(entry[i] == ' ') i++;                   // spaces
  j = 0;
  while(entry[i]!=' ' && entry[i] && j<(int)sizeof(c_weekday)) { // weekdays
    c_weekday[j++] = entry[i++];
    c_weekday[j] = '\0';
  }
  time_t t = time(0);
  struct tm res;
  struct tm *tt = localtime_r(&t, &res);
  // tt->tm_mday, tt->tm_mon, tt->tm_year, tt->tm_wday,
  // tt->tm_hour, tt->tm_min, tt->tm_tm_sec
  if(cron_parse(c_min, tt->tm_min) &&
      cron_parse(c_hour, tt->tm_hour) &&
        cron_parse(c_day, tt->tm_mday) &&
          cron_parse(c_month, tt->tm_mon+1) &&
            cron_parse(c_weekday, tt->tm_wday)) {
    return true;
  } 
  return false;
}

// get digit from IP address (ipv4 address in char* format)
int
tacpp::utl::getDigitFromIp(const char *str,const int n) {
  if(!str) return 0;
  if(strlen(str) == 0) return 0;
  char d1[4],d2[4],d3[4],d4[4];
  //int ret;
  bzero(d1, sizeof(d1));
  bzero(d2, sizeof(d2));
  bzero(d3, sizeof(d3));
  bzero(d4, sizeof(d4));

  //int j = 0;
  int k=0;
  int i=0;

  for(i=0; str[i+k] && str[i+k]!='.' && i<3; i++) {
    d1[i] = str[i+k];
  }
  if(str[i+k]=='.') k++;
  k += i;
  for(i=0; str[i+k] && str[i+k]!='.' && i<3; i++) {
    d2[i] = str[i+k];
  }
  if(str[i+k]=='.') k++;
  k += i;
  for(i=0; str[i+k] && str[i+k]!='.' && i<3; i++) {
    d3[i] = str[i+k];
  }
  if(str[i+k]=='.') k++;
  k += i;
  for(i=0; str[i+k] && str[i+k]!='.' && i<3; i++) {
    d4[i] = str[i+k];
  }
  if(n == 1) return atoi(d1);
  if(n == 2) return atoi(d2);
  if(n == 3) return atoi(d3);
  if(n == 4) return atoi(d4);
  return 0;
}

// check IP addresses (IPv4)
bool
tacpp::utl::isIp(const char *str) {
  if(!str) return false;
  int d1=0,d2=0,d3=0,d4=0;
  char s[4][4];
  int k = 0,i = 0,j = 0;

  if(strlen(str) < 7) return false;
  while(str[i] && j<4) {
    k = 0;
    while(str[i] && str[i]!='.')  {
      if(!isdigit(str[i])) return false;
      s[j][k++] = str[i++];
    }
    s[j][k] = '\0';
    j++;   i++;
  }
  if(strlen(s[0]) > 0) d1 = atoi(s[0]);
  if(strlen(s[1]) > 0) d2 = atoi(s[1]);
  if(strlen(s[2]) > 0) d3 = atoi(s[2]);
  if(strlen(s[3]) > 0) d4 = atoi(s[3]);
  if((d1 > 0 && d1 <= 255) &&
    (d2 >= 0 && d2 <= 255) &&
      (d3 >= 0 && d3 <= 255) &&
        (d4 > 0 && d4 < 255)) return true;
  return false;
}

////// Configuration Utilities

// compare two strings
// mask - mask, b - value
bool
tacpp::utl::match(const char *b,const char *mask) {
  int i = 0;
  if(!b || !mask) return false;
  if(strlen(b) == 0) return false;
  if(strlen(mask) == 0) return false;
  while(mask[i]!=0 && b[i]!=0 && isupper(mask[i])>0) {
    if((char)tolower(mask[i]) != (char)tolower(b[i])) return false;
    i++;
  }
  while(mask[i] && b[i]) {
    if(mask[i] != (char)tolower(b[i])) return false;
    i++;
  }
  if(!mask[i] && b[i] && b[i]!=' ') return false;
  return true;
}

bool
tacpp::utl::matchNstrict(const char *b,const char *mask) {
  int i = 0;
  if(strlen(b) == 0) return false;
  while(mask[i] && b[i] && isupper(mask[i]) > 0) {
    if((char)tolower(mask[i]) != (char)tolower(b[i])) return false;
    i++;
  }
  if(b[i]==0 && (isupper(mask[i]) > 0))return false;
  while(mask[i] && b[i] && b[i] != ' ') {
    if(mask[i] != tolower(b[i])) return false;
    i++;
  }
  return true;
}

/*
{int:-2--10}
{ str : 2 -15 }
{ip :}
{choice:Yes|No}
*/
bool //is correspond with param description
tacpp::utl::isCorrespondW(const char * pDesc,const char * str) {
  int dP = 0; //start of param descr
  int eP = 0; //end param descr
  int pL = 0; //start param limits description
  int eL = 0; //end param limits desc
  int len = (int)strlen(str);
  int min = -1;
  unsigned int max = 1;

//  cerr<<"pDesc="<<pDesc<<endl;
//  cerr<<"str="<<str<<endl;
  //[]{ str : 2 -15 }
  while (pDesc[dP]!='\0' && ( pDesc[dP]=='{'|| pDesc[dP]==' ')) dP++;
//  cerr<<"strip spaces dP="<<dP<<" pDesc+dP="<<pDesc+dP<<endl;
  //{ [s]tr : 2 -15 }
  if(pDesc[dP] == '\0')return false;

  eP = dP;
  while (pDesc[eP]!='\0' &&  pDesc[eP]!=':' && pDesc[eP]!=' ') eP++;
  //{ str[:] 2 -15 }
  if(pDesc[eP]=='\0')return false;
//  cerr<<"get param name eP="<<eP<<" pDecs+eP="<<pDesc+eP<<endl;

  pL=eP;
  while (pDesc[pL]!='\0' &&  (pDesc[pL]==':' || pDesc[pL]==' ')) pL++;
//  pL++;
  //{ str :[ ]2 -15 }
//  cerr<<"get param descRiption pL="<<pL<<" pDecs+pL="<<pDesc+pL<<endl;
  eL=strlen(pDesc);
//  cerr<<"eL="<<eL<<endl;
  while (pL<=eL && ( pDesc[eL]=='}' || pDesc[eL]==' ')) eL--;
  eL--;
//  cerr<<"eL="<<eL<<endl;
  //{ str : 2 -1[5] }
  if (!strncasecmp(pDesc+dP,"ip",eP-dP)) {
//    cerr<<"Compare IP="<<str<<endl;
    return utl::isIp(str);
  }
  char * tPar=new char[eL-pL+2];
  strncpy(tPar,pDesc+pL,eL-pL);
  tPar[eL-pL]='\0';
  int delimetr=0;// '|'  or  '-'
  int lenPd=strlen(tPar);
//cerr<<"tPar="<<tPar<<endl;

  if (!strncasecmp(pDesc+dP,"choice",eP-dP)) {
    while(delimetr<lenPd-1) {
            for(int ii=0;ii<=lenPd;ii++) {
              if(tPar[ii]=='|' || tPar[ii]=='\0' ) {
                 //tPar[ii]=='\0';
//cerr<<"choice="<<tPar+delimetr<<endl;
                 if(matchNstrict(str,tPar+delimetr)) {delete [] tPar; return true;}
                 delimetr=ii+1;
              }
            }
     }
     delete [] tPar;
     return false;
  }

  delimetr=0;
  char * tmp;
  bool open=false;
  while(tPar[delimetr]!='\0' &&tPar[delimetr]==' ')delimetr++;
  tmp=tPar+delimetr;
//  cerr<<"delimetr="<<delimetr<<" tPar+delimetr="<<tPar+delimetr<<endl;
  while(tPar[delimetr]!='\0' && tPar[delimetr]!='-'&&tmp[delimetr]!=' ')delimetr++;
  //FIXME I do not process signed integers
  tPar[delimetr]='\0';
  min=atoi(tmp);
//  cerr<<"min="<<min<<endl;
  delimetr++;
  if(lenPd>delimetr) {
    while(tPar[delimetr]!='\0'&&(tPar[delimetr]=='-'||tmp[delimetr]==' '))delimetr++;
    //FIXME I do not process signed integers
    if(lenPd>delimetr) {
      tmp=tPar+delimetr;
      while(tPar[delimetr]!='\0' &&tPar[delimetr]!=' ')delimetr++;
      tPar[delimetr+1]='\0';
      if(strlen(tmp)>0) {
//cerr<<"tmp="<<tmp<<endl;
        max=(unsigned int)atoi(tmp);
      }
      else open=true;
    }
    else     open=true;
  }
  else   open=true;
//  cerr<<"max="<<max<<endl;
//  cerr<<"open="<<open<<endl;


  delete [] tPar;

  if(!strncasecmp(pDesc+dP,"str",eP-dP)) {
//       cerr<<" Compare str="<<str<<endl;
    if(len>=min) {
      if(open)return true;
      if(len <= (int)max) return true;
      return false;
    };
  }
  else if (!strncasecmp(pDesc+dP,"int",eP-dP)) {
//    cerr<<" Compare int="<<atoi(str)<<endl;
    for(int ii=0;ii<len;ii++)if(!isdigit(str[ii]))  return false;
    if((int)atoi(str)>=min) {
      if(open)return true;
      if(atoi(str) <= (int)max) return true;
      return false;
    };
  }

  return false;
}

// compare 2 strings for mask coordination
// b - this is value string
// mask - mask string
// in mask you can set:
//  * - any enter
//  (int) - integer numeric
//  (regex) - regular expression
//  (ip) - ip addr
//  Yes|No - one of several possibilities
// returns additional parameter in case of compare success
// returns 0, if don't success
string
tacpp::utl::vmatch(const char *in,const char *mask) {
  if(!in) return "";
  if(strlen(in) == 0) return "";
  char str[100];
  bzero(str,sizeof(str));
  string sout;
  if(!mask) return sout;

  char b[200];
  bzero(b,sizeof(b));
  snprintf(b,sizeof(b)-1,"%s",in);

  if(scomp(mask,"*")) { // if *, permit
    //snprintf(str,sizeof(str),"%s",b);
    sout = b;
    return sout;
  }
  // if (int), check for digits
  if(scomp(mask,"(int)")) {
    for(int i=0; b[i]; i++) {
      if(!isdigit(b[i])) return sout;
    }
    //snprintf(str,sizeof(str),"%s",b);
    sout = b;
    return sout;
  }
  // if (regex), check regular expression
  if(scomp(mask,"(regex)")) {
    regex_t preg;
    if(regcomp(&preg,b,REG_EXTENDED)) return sout;
    regfree(&preg);
    //snprintf(str,sizeof(str),"%s",b);
    sout = b;
    return sout;
  }
  // if (ip)
  if(scomp(mask,"(ip)")) {
    if(!isIp(b)) return sout;
    //snprintf(str,sizeof(str),"%s",b);
    sout = b;
    return sout;
  }
  // if several selectables Yes|No|Know|How
  // a) split mask
  // s - splited mask
  char s[11][100];  // maximum components 10
  for(int i=0; i<11; i++)
    bzero(s[i],sizeof(s[i]));
  int j = 0;
  int k = 0;
  for(int i=0; mask[i] && j<10 && k<100; i++) {
    if(mask[i] == '|') { j++; k = 0; continue; }
    s[j][k++] = mask[i];
    s[j][k] = 0;
  }
  // b) compare all elements
  bool r;
  int m;
  for(int i=0; i <= j; i++) {
    r = true;
    m = 0;
    // check PERFECT equal
    for(m=0; isupper(s[i][m]); m++) {
      if(!b[m]) r = false;
      else {
        if(tolower(s[i][m]) != tolower(b[m])) r = false;
      }
    }
    // if anything else in b[m]
    for(; b[m] && s[i][m]; m++) {
      if(s[i][m] != tolower(b[m])) r = false;
    }
    if(r) {
      bzero(str,sizeof(str));
      for(m = 0; s[i][m] && m<(int)sizeof(str); m++) {
        str[m]=tolower(s[i][m]);
      }
      sout = str;
      return sout;
    }
  }
  return sout;
}

string
tacpp::utl::splitId(const char *s,const int c) {
  if(!s) return "";
  if(strlen(s) == 0) return "";
  char str[256];
  string ostr;
  bzero(str,sizeof(str));
  //1. find if @ present
  int i = 0;
  int k = 0;
  for(i = 0; i<(int)strlen(s) && i<(int)sizeof(str) && s[i] != '@'; i++) {
    str[i] = s[i];
  }
  if(s[i] != '@' && c==0) return "";    // if we require name, but it no
  if(s[i] == '@' && c==0) return str;   // if we require name, and it presents
  if(s[i] != '@' && c==1) {             // if require host and no name presents
    bzero(str,sizeof(str));
    for(i = 0; i<(int)strlen(s) && i<(int)sizeof(str) && s[i]!=':'; i++) str[i]=s[i];
    if(s[i]==':') {
      ostr = str;
      return ostr;
    }
    return ostr;
  }
  if(s[i]=='@' && c==1) {     // if name present and host required
    i++;
    bzero(str,sizeof(str));
//for(i = 0; i<strlen(s) && i<sizeof(str) && s[i] != '@'; i++);
//if(s[i] != '@')
    k=0;
    for(; i<(int)strlen(s) && i<(int)sizeof(str) && s[i]!=':'; i++) str[k++]=s[i];
    if(s[i]==':') {
      ostr = str;
      return ostr;
    }
    return ostr;
  }
  if(c==2) {
    bzero(str,sizeof(str));
    for(i = 0; i<(int)strlen(s) && i<(int)sizeof(str) && s[i]!=':'; i++) str[i]=s[i];
    if(s[i]!=':') return ostr;
    i++;
    bzero(str,sizeof(str));
    k = 0;
    for(; i<(int)strlen(s) && i<(int)sizeof(str) && s[i]; i++) str[k++]=s[i];
    ostr = str;
    return ostr;
  }
  return ostr;
}

string
tacpp::utl::split(const char *s,const int c) {
  if(!s) return "";
  if(strlen(s) == 0) return "";
  string ostr;
  int j = 0;
  for(int i=0; i<c; i++) {
    while(s[j]!=0 && s[j]!=' ') j++;
    while(s[j] == ' ') j++;
  }
  if(s[j] == 0) return ostr;
  int k = 0;
  char str[500];
  bzero(str,sizeof(str));
  while(s[j]!=0 && s[j]!=' ' && k<500) {
    str[k++] = s[j++];
  }
  ostr = str;
  return ostr;
}

// returns attribute name from entry
string
tacpp::utl::getAvpName(const char *av) {
  if(!av) return "";
  if(strlen(av) == 0) return "";
  string ostr;
  char str[200];
  bzero(str,sizeof(str));
  int i = 0;
  for(i=0; i < (int)strlen(av) && i < (int)sizeof(str) && av[i] != '='; i++) {
    str[i] = av[i];
  }
  if(av[i] != '=') return ostr;
  ostr = str;
  return ostr;
}

// return attribute value from entry type:
// attribute_name=value
string
tacpp::utl::getAvpValue(const char *av) {
  if(!av) return "";
  if(strlen(av) == 0) return "";
  char str[300];
  bzero(str,sizeof(str));
  int i = 0;
  for(i = 0; i < (int)strlen(av) && av[i] != '='; i++);
  i++;
  for(int k = 0; i < (int)strlen(av) && av[i] != 0 && k < (int)sizeof(str); i++, k++) {
    str[k] = av[i];
  }
  string ostr(str);
  return ostr;
}

// returns all after ':'
string
tacpp::utl::getAvpMod(const char *av) {
  if(!av) return "";
  if(strlen(av) == 0) return "";
  char str[300];
  string ostr;
  //bool res = false;
  bzero(str,sizeof(str));
  int i = 0;
  for(i = 0; i < (int)strlen(av) && av[i] != ':'; i++);
  if(av[i] != ':') return ostr;
  i++;
  if(i == (int)strlen(av)) return ostr;
  for(int k = 0; i < (int)strlen(av) &&
      av[i] != 0 && k < (int)sizeof(str); i++, k++) {
    str[k] = av[i];
  }
  ostr = str;
  return ostr;
}

// insert symbol to position
/*
char*
tacpp::utl::inssym(const char *s,const char c,int pos) {
  static char str[100];
  bzero(str,sizeof(str));
  int i=0;
  int j=0;
  for(;i<pos && s[j]; i++,j++) str[i] = s[j];
  str[i++] = c;
  for(;i<(sizeof(str)-1) && s[j]; i++,j++) str[i] = s[j];
  return str;
}
*/

// remove symbol from position
/*
char*
tacpp::utl::delsym(const char *s,int pos) {
  static char str[100];
  bzero(str,sizeof(str));
  int i=0;
  int j=0;
  for(;i<pos && s[j]; i++,j++) str[i] = s[j];
  j++;
  for(;i<(sizeof(str)-1) && s[j]; i++,j++) str[i] = s[j];
  return str;
}
*/

// encode to base64
string
tacpp::utl::base64Encode(const char *in) {
  if(!in) return "";
  if(strlen(in) == 0) return "";
  char s[64];
  bzero(s,sizeof(s));
  snprintf(s,sizeof(s)-1,"%s",in);

  char d[64];
  bzero(d,sizeof(d));
  int dlen = sizeof(d)-1;

  unsigned char tbl[257];
  bzero(tbl,sizeof(tbl));
  //for(int i=0;i<=256;i++) tbl[i] = '\0';
  int i;
  // fill dtable with character encodings
  for(i = 0; i < 26; i++) {
    tbl[i] = 'A' + i;
    tbl[26 + i] = 'a' + i;
  }
  for(i = 0; i < 10; i++) {
    tbl[52 + i] = '0' + i;
  }
  tbl[62] = '+';
  tbl[63] = '/';

  bool hiteof = false;
  int k = 0, m = 0;

  unsigned char igroup[3], ogroup[4];

  igroup[0] = 0;
  igroup[1] = 0;
  igroup[2] = 0;
  ogroup[0] = 0;
  ogroup[1] = 0;
  ogroup[2] = 0;
  ogroup[3] = 0;
  int ogn1,ogn2,ogn3,ogn4;

  while(!hiteof && k<64) {
    unsigned char c = '\0';
    int n = 0;
    igroup[0] = 0;
    igroup[1] = 0;
    igroup[2] = 0;
    for(n = 0; n < 3; n++) {
      c = s[k++];
      if(c == '\0') {
        hiteof = true;
        break;
      }
      igroup[n] = c;
    }
    if(n > 0) {
      ogn1 = 0;
      ogn1 = (int)(igroup[0] >> 2);
      if(ogn1 > 255 || ogn1 < 0) ogn1 = 255; // !!!
      ogroup[0] = tbl[ogn1];
      ogn2 = 0;
      ogn2 = (int)(((igroup[0] & 3) << 4) | (igroup[1] >> 4));
      if(ogn2 > 255 || ogn2 < 0) ogn2 = 255; // !!!
      ogroup[1] = tbl[ogn2];
      ogn3 = 0;
      ogn3 = (int)(((igroup[1] & 0xF) << 2) | (igroup[2] >> 6));
      if(ogn3 > 255 || ogn3 < 0) ogn3 = 255; // !!!
      ogroup[2] = tbl[ogn3];
      ogn4 = 0;
      ogn4 = (int)(igroup[2] & 0x3F);
      if(ogn4 > 255 || ogn4 < 0) ogn4 = 255; // !!!
      ogroup[3] = tbl[ogn4];

      // Replace characters in output stream with "=" pad
      //  characters if fewer than three characters were
      //  read from the end of the input stream. */
      if(n < 3) {
        ogroup[3] = '=';
        if(n < 2) {
          ogroup[2] = '=';
        }
      }
      for(i = 0; i<4 && m<dlen; i++) {
        d[m++] = ogroup[i];
      }
    }
  }
  string sout = d;
  return sout;
}

string
tacpp::utl::base64Decode(const char *in) {
  if(!in) return "";
  if(strlen(in) == 0) return "";
  bool errcheck = false;
  unsigned char dtable[256];
  int k = 0, m = 0;

  char s[64];
  snprintf(s,sizeof(s)-1,"%s",in);

  char d[64];
  bzero(d,sizeof(d));
  int dlen = sizeof(d)-1;

  string sout;

  int i;
  for(i = 0; i < 255; i++) {
    dtable[i] = 0x80;
  }
  for(i = 'A'; i <= 'Z'; i++) {
    dtable[i] = 0 + (i - 'A');
  }
  for(i = 'a'; i <= 'z'; i++) {
    dtable[i] = 26 + (i - 'a');
  }
  for(i = '0'; i <= '9'; i++) {
    dtable[i] = 52 + (i - '0');
  }
  dtable[(int)'+'] = 62;
  dtable[(int)'/'] = 63;
  dtable[(int)'='] = 0;

  while(true) {
    unsigned char a[4], b[4], o[3];
    int c = 0;
    for(i = 0; i<4 && m<(dlen-4) && k<128; i++) {
      c = (int)s[k++];
      if(c == 0) {
        if(errcheck && (i > 0)) {
          sout = d;
          return sout;
        }
        sout = d;
        return sout;
      }
      if(dtable[c] & 0x80) {
        if(errcheck) {
          //fprintf(stderr, "Illegal character '%c' in input file.\n", c);
          return sout;
        }
        // Ignoring errors: discard invalid character
        i--;
        continue;
      }
      a[i] = (unsigned char) c;
      b[i] = (unsigned char) dtable[c];
    }
    o[0] = (b[0] << 2) | (b[1] >> 4);
    o[1] = (b[1] << 4) | (b[2] >> 2);
    o[2] = (b[2] << 6) | b[3];
    i = a[2] == '=' ? 1 : (a[3] == '=' ? 2 : 3);
    //if(fwrite(o, i, 1, stdout) == EOF) {
    //  exit(1);
    //}
    if(i > 0) d[m++] = o[0];
    if(i > 1) d[m++] = o[1];
    if(i > 2) d[m++] = o[2];
    if(i < 3) {
      sout = d;
      return sout;
    }
  }
  sout = d;
  return sout;
}

// convert int digit to string form
string
tacpp::utl::toString(const int a) {
  char aaa[100];
  snprintf(aaa,sizeof(aaa)-1,"%d",a);
  string out(aaa);
  return out;
}

// convert unsigned int to string form
string
tacpp::utl::toString(const unsigned int a) {
  char aaa[100];
  snprintf(aaa,sizeof(aaa)-1,"%u",a);
  string out(aaa);
  return out;
}

string
tacpp::utl::toString(const unsigned long a) {
  char aaa[100];
  snprintf(aaa,sizeof(aaa)-1,"%lu",a);
  string out(aaa);
  return out;
}

string
tacpp::utl::toString(const float a) {
  char aaa[100];
  snprintf(aaa,sizeof(aaa)-1,"%f",a);
  string out(aaa);
  return out;
}

string
tacpp::utl::toString(const double a) {
  char aaa[100];
  snprintf(aaa,sizeof(aaa)-1,"%f",a);
  string out(aaa);
  return out;
}

// int b - for differentiate this method from unsigned int
string
tacpp::utl::toString(time_t a,int b) {
  char aaa[100];
  snprintf(aaa,sizeof(aaa)-1,"%lu",a);
  string out(aaa);
  return out;
}

string
tacpp::utl::toSQL(const string s) {
  char ret[1000];
  char str[900];
  bzero(str,sizeof(str));
  bzero(ret,sizeof(ret));
  snprintf(str,sizeof(str)-1,"%s",s.c_str());
  for(int i=0,j=0; i<(int)strlen(str) && j<(int)sizeof(ret)-3; i++,j++) {
    ret[j] = str[i];
    // symbol '\' should be changed to '\\'
    if(ret[j] == '\\') ret[++j] = '\\';
    // symbol ''' should be changed to '\''
    if(ret[j] == '\'') { ret[j] = '\\'; ret[++j] = '\''; }
    // symbol '"' should be changed to '\"'
    if(ret[j] == '\"') { ret[j] = '\\'; ret[++j] = '\"'; }
  }
  string out = ret;
  return out;
}

string
tacpp::utl::sip(const long unsigned int i) {
  struct in_addr a;
  string str;
  a.s_addr = (in_addr_t)i;
  str = inet_ntoa(a);
  return str;
}

//////////////////// that's all ///////////////////////
