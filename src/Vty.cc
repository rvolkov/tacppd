// This is part of tacppd project (extfended tacacs++ daemon)
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: Vty.cc
// description: terminal access support

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// modifications:
//

#include "global.h"

#define CONTROL(X)  ((X) - '@')

void
Vty::vty_cleanup() {
  bzero(vty_line,sizeof(vty_line));
  vtylinepos = 0;
  isChanged = true;
}

int
Vty::vty_get_linepos() {
  return vtylinepos;
}

string
Vty::vty_get_line() {
  return string(vty_line);
}

void
Vty::vty_set_line(const char *l) {
  bzero(vty_line,sizeof(vty_line));
  strncpy(vty_line,l,maxVtyInput);
  vtylinepos = strlen(l);
  isChanged = true;
}

bool
Vty::twriteErr(const char*s){ return true; };

bool
Vty::twrite(const char*s){ return true; };

//--------------------- Vty Stream
VtyStream::VtyStream(const char *  file) throw(VtyErr){
  char   fileName[512];

  bzero(vty_line,sizeof(vty_line));
  vtylinepos = 0;
  isChanged = true;

  snprintf(fileName,sizeof(fileName)-2,"%s/%s",::MAINDIR,file);
  ifstream *fcfstr=new ifstream();
  fcfstr->open(fileName,std::ios::in);
  if(!fcfstr) throw VtyErr(CMD_ERR_OPEN_FSTREAM,fileName);
  cfstr=fcfstr;

  vtSplit = new VtyLineSplitter(this);
}

VtyStream::VtyStream(const  string str)  {
  cfstr=new istringstream(str);
  isChanged = true;
  bzero(vty_line,sizeof(vty_line));
  vtylinepos = 0;
  vtSplit = new VtyLineSplitter(this);
}

VtyStream::~VtyStream() {
  delete cfstr;
  delete vtSplit;
}

int
VtyStream::treadstr() throw (VtyErr) {
  string s;
  //int ii=0;
  if(cfstr->eof()||!cfstr->good())throw VtyErr(CMD_END_OF_STREAM,"Unexpected end of stream in VtyStream::treadstr");
  std::getline(*cfstr,s,'\n');
  vty_set_line(s.c_str());
  return T_KEY_EXECUTE_COMMAND;
}

char*
VtyStream::readstr() throw (VtyErr) {
  string s;
  //int ii=0;
  if(cfstr->eof()||!cfstr->good()) throw VtyErr(CMD_END_OF_STREAM,"Unexpected end of stream in VtyStream::readstr");
  std::getline(*cfstr,s,'\n');
  vty_set_line(s.c_str());
  return vty_line;
}
bool
VtyStream::twriteErr(const char*s) throw (VtyErr) {
  throw VtyErr( CMD_FAILED,s);
return true;
};

//--------------------- Vty Term
VtyTerm::VtyTerm(int fh, struct in_addr addr) : Packet(fh) {
  snprintf(address,sizeof(address)-1,"%s",inet_ntoa(addr));
  in_address = addr;
  handle = fh;
  bzero(vty_line,sizeof(vty_line));
  vtylinepos = 0;

  local_echo = true;
  isChanged = true;
  vtSplit = new VtyLineSplitter(this);

  vty_height = 24; // setup default values
  vty_width = 80;
}

VtyTerm::~VtyTerm() {
  delete vtSplit;
}

// set terminal(telnet) settings
void
VtyTerm::tset() {
  char str[10];

  bzero(str,sizeof(str));
  snprintf(str,sizeof(str),"\xff\xfe\x01");     // Don't Echo
  twrite(str);
  snprintf(str,sizeof(str),"\xff\xfe\x03");     // Don't Supress GA
  twrite(str);
  //snprintf(str,sizeof(str),"\xff\xfd\x18");   // Do Terminal Type
  //twrite(str);
  snprintf(str,sizeof(str),"\xff\xfd\x1f");   // Do Negotiation about win
  twrite(str);
//  snprintf(str,sizeof(str),"\xff\xfd\x0d");   // Do Negotiation about FormFeed
//  twrite(str);
  //snprintf(str,sizeof(str),"\xff\xfe\x20");   // Don't Terminal Speed
  //twrite(str);
  //snprintf(str,sizeof(str),"\xff\xfd\x21");   // Do Remoute Flow Control
  //twrite(str);
  //snprintf(str,sizeof(str),"\xff\xfa\x21\x00\xff\xf0"); // Rem Flow Cont
  //twrite(str);
  snprintf(str,sizeof(str),"\xff\xfe\x22");     // Don't linemode
  twrite(str);
  //snprintf(str,sizeof(str),"\xff\xfe\x27");   // Don't new env option
  //twrite(str);
  //snprintf(str,sizeof(str),"\xff\xfc\x05");   // Won't status
  //twrite(str);
  //snprintf(str,sizeof(str),"\xff\xfe\x23");   // Dont X dispaly locat
  //twrite(str);
}

// clear line on vty
void
VtyTerm::vty_clear_input() {
  vty_move_end_of_line();
  while(vtylinepos--) twrite("\b \b");
  vty_line[0] = '\0';
  isChanged = true;
}

// clear one char on vty
void
VtyTerm::vty_del_char() {
  int length = strlen(vty_line);
  if(length==0 || length==vtylinepos) {
    twrite("\a");
    return;
  }
  twrite(vty_line+vtylinepos+1);
  twrite(' ');
  length -= vtylinepos;
  memmove (&vty_line[vtylinepos], &vty_line[vtylinepos+1], length);
  while(length--) twrite("\b");
  isChanged = true;
}

void
VtyTerm::vty_del_backward_char() {
  if(vtylinepos == 0) {
    twrite("\a");
    return;
  }
  vty_move_backward();
  twrite(vty_line+vtylinepos+1);
  twrite(' ');
  int length = strlen(vty_line) - vtylinepos;
  memmove(&vty_line[vtylinepos], &vty_line[vtylinepos+1], length);
  while(length--) twrite("\b");
  isChanged = true;
}

void 
VtyTerm::vty_move_forward() {
  if(vtylinepos < (int)strlen(vty_line)) {
    twrite(vty_line[vtylinepos]);
    vtylinepos++;
    isChanged = true;
  } else if(vtylinepos < maxVtyInput) {
    vty_insert_char(' ');
    isChanged = true;
  } else {
    twrite("\a");
  }
}

void
VtyTerm::vty_move_backward() {
  if(vtylinepos > 0) {
    twrite("\b");
    vtylinepos--;
    isChanged = true;
  }
}

void
VtyTerm::vty_move_to(int pos) {
  if(vtylinepos >= maxVtyInput) return;
  if(pos < vtylinepos)
    while(pos != vtylinepos) vty_move_backward();
  else while(pos != vtylinepos) vty_move_forward();
  return;
}

void
VtyTerm::vty_move_end_of_line() {
  int len = strlen(vty_line);
  while(vtylinepos < len) {
    twrite(vty_line[vtylinepos]);
    vtylinepos++;
  }
  isChanged = true;
}

void 
VtyTerm::vty_move_beginning_of_line() {
  while(vtylinepos > 0) {
    twrite("\b");
    vtylinepos--;
  }
  isChanged = true;
}

void
VtyTerm::vty_redraw_line() {
  twrite(vty_line);
  int back = strlen(vty_line) - vtylinepos;
  while(back > 0) {
    twrite("\b");
    back--;
  }
}
void
VtyTerm::vty_clear_line_from_beg() {
  if(vtylinepos == 0) {
    twrite("\a");
    return;
  }
  int cur;
  int length=strlen(vty_line);

  memmove(&vty_line[0], &vty_line[vtylinepos], length - vtylinepos );
  cur=length-vtylinepos;
  while(cur<length)vty_line[cur++]=' ';
  cur=vtylinepos;
  vty_move_beginning_of_line();
  twrite(vty_line);
  vty_line[length-cur]='\0';
  vtylinepos=length;
  vty_move_beginning_of_line();

  isChanged = true;
}
void
VtyTerm::vty_insert_char(unsigned char c) {
  if(local_echo) {
    twrite(c);
//std::cout<<"XXX c="<<c<<std::endl;
  }
  if(vtylinepos == (int)strlen(vty_line)) {
    vty_line[vtylinepos] = c;
    vty_line[vtylinepos+1] = '\0';
  } else if(vtylinepos < maxVtyInput) {
    if(local_echo) twrite(vty_line+(vtylinepos));
    int length = strlen(vty_line) - vtylinepos;
    memmove(&vty_line[vtylinepos + 1], &vty_line[vtylinepos], length);
    vty_line[vtylinepos] = c;
    while(length--) twrite("\b");
  } else {
    twrite("\a");
    return;
  }
  vtylinepos++;
  isChanged = true;
}

void
VtyTerm::toptions(){
  unsigned char s[2];
  bool inSub=false;
  bool inCmd=false;

//  cerr<<"\nIAC "<<endl;

  do {
    bzero(s,sizeof(s));
    sockread(s,1,TERMINAL_TIMEOUT);
    //==-1)throw VtyErr(CMD_TIMEOUT,"Unexpected error or timeout while sockread.");
//     cerr<<" Option="<<int(s[0]);
    switch (s[0]) {
    case IAC:
//      cerr<<"IAC ";
      break;
    case WILL:
//      cerr<<"WILL ";
      inCmd=true;
      break;
    case WONT:
//      cerr<<"WONT ";
      inCmd=true;
      break;
    case DO:
//      cerr<<"DO ";
      inCmd=true;
      break;
    case DONT:
//      cerr<< "DONT ";
      inCmd=true;
      break;
    case SB:
      inSub=true;
//      cerr<<"SB ";
      break;
    case SE:
//      cerr<<"SE ";
      return;
      break;
    case TELOPT_ECHO:
//    cerr<<"TELOPT_ECHO ";
      if(inCmd)return;
      break;
    case TELOPT_NAOFFD:
//    cerr<<"TELOPT_NAOFFD";
      if(inCmd)return;
      break;
    case TELOPT_SGA:
//    cerr<<"TELOPT_SGA ";
      if(inCmd)return;
      break;
    case TELOPT_NAWS:
//      cerr<< "TELOPT_NAWS ";
      if(inCmd) return;
      bzero(s,sizeof(s));
      sockread(s,1,TERMINAL_TIMEOUT);
      bzero(s,sizeof(s));
      sockread(s,1,TERMINAL_TIMEOUT);
      vty_width=s[0];
      bzero(s,sizeof(s));
      sockread(s,1,TERMINAL_TIMEOUT);
      bzero(s,sizeof(s));
      sockread(s,1,TERMINAL_TIMEOUT);
      vty_height=s[0];
      break;
    default:
//      cerr<<(int)s[0]<<endl;
      break;
    }
//    cerr<<endl;
  } while(inCmd||inSub);
}


// read symbol from terminal
unsigned char
VtyTerm::tsymbol(int &control) {
  char out;
  unsigned char s[2];
  bool nextChar = true;

  int escStat = 0; //{0} normal, {1} pre escape, {2} escape

  while(nextChar) {
    bzero(s,sizeof(s));
    sockread(s,1,TERMINAL_TIMEOUT);//==-1);throw VtyErr(CMD_TIMEOUT,"Unexpected error or timeout while sockread.");

    out = s[0];
    switch (out) {
    case '\xff':
      toptions();
      break;
    case '\x1b':
      escStat=1;
      break;
    case '[':
      if(escStat==1) escStat=2;
      else goto insertChar;
    case '\n':
       break;
    case 'A':
      if(escStat!=2) goto insertChar;
      else control=T_KEY_UP;
      return '\0';
      break;
    case 'B':
      if(escStat!=2) goto insertChar;
      else control=T_KEY_DOWN;
      return '\0';
      break;
    case 'C':
      if(escStat!=2) goto insertChar;
      else control=T_KEY_RIGHT;
      return '\0';
      break;
    case 'D':
      if(escStat!=2) goto insertChar;
      else control=T_KEY_LEFT;
      return '\0';
      break;
    case CONTROL('A'):
      control=T_KEY_BEGINNING_OF_LINE;	
      return '\0';
      break;
    case CONTROL('B'):
      control=T_KEY_BACKWARD_CHAR;	
      return '\0';
      break;
    case CONTROL('D'):
      control=T_KEY_DELETE_CHAR;
      return '\0';
      break;
    case CONTROL('E'):
      control=T_KEY_END_OF_LINE;
      return '\0';
      break;
    case CONTROL('F'):
      control=T_KEY_FORWARD_CHAR;
      return '\0';
      break;
    case CONTROL('H'):
      case 0x7f:
      control=T_KEY_DELETE_BACKWARD_CHAR;
      return '\0';
      break;
    case CONTROL('K'):
      control=T_KEY_CLEAR_LINE;
      return '\0';
      break;
    case CONTROL('N'):
      control=T_KEY_NEXT_COMMAND;
      return '\0';
      break;
    case CONTROL('P'):
      control=T_KEY_PREVIOUS_COMMAND;
      return '\0';
      break;
    case CONTROL('U'):
      control=T_KEY_CLEAR_LINE_FROM_BEGINNING;
      return '\0';
      break;
    case CONTROL('W'):
      control=T_KEY_KILL_WORD;
      return '\0';
      break;
    case '\r':
      control=T_KEY_EXECUTE_COMMAND;
      return '\0';
      break;
    case '\t':
      control=T_KEY_COMPLETE_COMMAND;
      return '\0';
      break;
    case '?':
      control=T_KEY_GET_HELP;
      twrite("?");
      return '\0';
      break;
    default:
insertChar:
      control=T_KEY_INSERT_CHAR;
      return out;
    }
  }
  return '\0';
}

// read string from terminal
int
VtyTerm::treadstr() {
  unsigned char c = '\0';
  int control=0;
  while(true) {
    c = tsymbol(control);
    switch(control) {
      case T_KEY_INSERT_CHAR:
      if(c != '\0') vty_insert_char(c);
      break;
      case T_KEY_BEGINNING_OF_LINE:
      vty_move_beginning_of_line();
      break;
      case T_KEY_END_OF_LINE:
      vty_move_end_of_line();
      break;
      case T_KEY_LEFT:
      case T_KEY_BACKWARD_CHAR:
      vty_move_backward();
      break;
     case T_KEY_RIGHT:
     case T_KEY_FORWARD_CHAR:
      vty_move_forward();
      break;
      case T_KEY_DELETE_CHAR:
      vty_del_char();
      break;
      case T_KEY_DELETE_BACKWARD_CHAR:
      vty_del_backward_char();
      break;
      case T_KEY_CLEAR_LINE:
      vty_clear_input();
      break;
      case T_KEY_CLEAR_LINE_FROM_BEGINNING:
      vty_clear_line_from_beg();
      break;
     case T_KEY_EXECUTE_COMMAND:
      return T_KEY_EXECUTE_COMMAND;
      break;
     case T_KEY_COMPLETE_COMMAND:
      return T_KEY_COMPLETE_COMMAND;
      break;
     case T_KEY_DOWN:
     case T_KEY_NEXT_COMMAND:
      return T_KEY_NEXT_COMMAND;
      break;
     case T_KEY_UP:
     case T_KEY_PREVIOUS_COMMAND:
      return T_KEY_PREVIOUS_COMMAND;
      break;
     case T_KEY_GET_HELP:
      return T_KEY_GET_HELP;
      break;
     case T_KEY_KILL_WORD:
      default:
      break;
    }
  }
  return 0;
}

char*
VtyTerm::readstr() {
  bzero(vty_line,sizeof(vty_line));
  vtylinepos = 0;
  while(treadstr() != T_KEY_EXECUTE_COMMAND);
  return vty_line;
}

// write
// add \r before \n
// or...
bool
VtyTerm::twrite(const char *str)  {
  char s[600];
  int ss;
  ss = sizeof(s) - 1;
  bzero(s,ss+1);
  int j;
  j = 0;
  for(int i=0; str[i] && j<ss; i++) {
    s[j++] = str[i];
    if(str[i] == '\n') s[j++] = '\r';
  }
//std::cout<<"s="<<s<<std::endl;
  sockwrite((unsigned char *)s, strlen(s), TERMINAL_TIMEOUT);// <= 0)  throw VtyErr(CMD_TIMEOUT,"Unexpected error or timeout while sockwrite.");

  return true;
}

bool
VtyTerm::twrite(const string str) {
  return twrite(str.c_str());
}

bool
VtyTerm::twriteErr(const char*str) {
  twrite(str);
  return true;
}

bool
VtyTerm::twrite(unsigned char c){
  unsigned char s[3];
  s[0] = c;
  s[1] = '\0';
  sockwrite((unsigned char *)s, 1, TERMINAL_TIMEOUT);// > 0); throw VtyErr(CMD_TIMEOUT,"Unexpected error or timeout while sockwrite.");
  return true;
}

void
VtyTerm::twrite_marker_at(int pos,char marker) {
  char line[maxVtyInput+2];
  if(pos > maxVtyInput) return;
  memset(line,' ',pos+1);
  line[0] = '\n';
  line[pos+1] = marker;
  line[pos+2] = '\0';
  twrite(line);
  return;
}

int
VtyTerm::get_handle() {
  return handle;
}

// set/unset terminal echo
void
VtyTerm::echo(bool s) {
  char str[10];
  bzero(str,sizeof(str));
  if(s) {
    snprintf(str,sizeof(str),"\xff\xfc\x01");
  } else {
    snprintf(str,sizeof(str),"\xff\xfb\x01");
  }
  twrite(str);
}

// set/unset linemode
void
VtyTerm::linemode(bool s) {
  char str[10];
  bzero(str,sizeof(str));
  if(s) {
    snprintf(str,sizeof(str),"\xff\xfc\x03");       // Won't supress GA
    twrite(str);
    //snprintf(str,sizeof(str),"\xff\xfe\x22");     // don't linemode
    //twrite(str);
  } else {
    snprintf(str,sizeof(str),"\xff\xfb\x03");       // Will Supress GA
    twrite(str);
    //snprintf(str,sizeof(str),"\xff\xfd\x22");     // do linemode
    //twrite(str);
  }
}

// per page output
void
VtyTerm::pager(const char *spage,int height) {
  if(height == 0) height = vty_height;
  int width = vty_width; // it could be changed during tsymbol

  char *pline = new char[(width+2)];
  bzero(pline,width+2);
  int step = height-1;

  int i = 0;
  int j = 0;
  int cc;
  while(spage[i]) {
    for(int p=0; p<step && spage[i]; p++) {
      j = 0;
      bzero(pline,width+2);
      while(spage[i] && spage[i]!='\n' && spage[i]!='\r' && j<width) pline[j++] = spage[i++];
      if(j == width) pline[width]='\n';
      else pline[j++] = spage[i++];
      twrite(pline);
    }

    char c = 'a';
    cc = 0;
    if(spage[i]) {
      twrite("--(press space for next page, enter for line, q to stop)--");
      while(c!=' ' && c!='q' && c!='\r' && cc!=T_KEY_EXECUTE_COMMAND) c = tsymbol(cc);

      bzero(pline,width+2);
      memset(pline,'\b',width);
      twrite(pline);
      memset(pline,' ',width);
      twrite(pline);
      memset(pline,'\b',width);
      twrite(pline);
      if(cc == T_KEY_EXECUTE_COMMAND) step=1;
      if(c == ' ') step = height-1;
      if(c == 'q') break;
    } else break;
  }
  delete [] pline;
}

///////////////////////////////////
