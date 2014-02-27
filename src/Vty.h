// This is part of tacppd project (extended tacacs++ daemon)
// (c) Copyright in 2000-2007 by Roman Volkov and contributors
// see http://tacppd.org for more information
//
// file: Vty.h
// description:

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
#ifndef __VTY_H__
#define __VTY_H__

#include "global.h"

namespace tacpp {

#define TERMINAL_TIMEOUT	10000
enum command {
  T_KEY_INSERT_CHAR=0,
  T_KEY_BEGINNING_OF_LINE,	
  T_KEY_BACKWARD_CHAR,	
  T_KEY_DELETE_CHAR,
  T_KEY_END_OF_LINE,
  T_KEY_FORWARD_CHAR,
  T_KEY_DELETE_BACKWARD_CHAR,
  T_KEY_CLEAR_LINE,
  T_KEY_NEXT_COMMAND,
  T_KEY_PREVIOUS_COMMAND,
  T_KEY_CLEAR_LINE_FROM_BEGINNING,
  T_KEY_KILL_WORD,
  T_KEY_EXECUTE_COMMAND,
  T_KEY_COMPLETE_COMMAND,
  T_KEY_GET_HELP,

  T_KEY_UP,
  T_KEY_DOWN,
  T_KEY_LEFT,
  T_KEY_RIGHT,

  T_KEY_EOF
};

enum telnetOpt {
  TELOPT_BINARY=0,
  TELOPT_ECHO=1,
  TELOPT_SGA=3,
  TELOPT_STATUS=5,
  TELOPT_NAOFFD=13,
  TELOPT_NAWS=31,
};
enum telnetCmd {
  SE=240,
  NOP,
  DMARK,
  BRK,
  IP,
  AO,
  AYT,
  EC,
  EL,
  GA,
  SB,
  WILL,
  WONT,
  DO,
  DONT,
  IAC
};

const int maxVtyInput=512;
/**
@short class Vty is used for virtual terminal comand line maintaince
@author Igor Lyapin
*/
class VtyLineSplitter;
class Vty {
 protected:
  /**
   */
  char vty_line[maxVtyInput+1];
  /**
   */
  int vtylinepos;
 public:
  /**
  @param isChanged is used to really know, is during perion precede last setting this variable to false,  vty_line was change.
  By changes I mean the changes in vty_line or vtylinepos. This variable is used while split the line into command parts. I want
  to do this split only one time when vtyline was really changes.
  */
  bool isChanged;
  /**
   */
  VtyLineSplitter *vtSplit;
  /**
   */
  virtual string vty_get_line();
  /**
   */
  virtual int vty_get_linepos();
  /**
  @see vty_cleanup zero vty_line and vty_pos, but do not make clear char from screen
  */
  virtual void vty_cleanup();
  /**
   */
  virtual void vty_set_line(const char *);
  /**
   */
  virtual bool twriteErr(const char*);
  /**
   */
  virtual bool twrite(const char*);
  /**
   */
  virtual int treadstr()=0;
  /**
   */
  virtual char * readstr()=0;
  /**
  */
  virtual ~Vty() {};
};

/**
*/
class VtyStream : public Vty {
 private:
  /**
   */
  std::istream * cfstr;
 public:
  /**
   */
  VtyStream(const char * fileName)throw(VtyErr);
  /**
   */
  VtyStream(const  string strAsBuffer);
  /**
   */
  virtual ~VtyStream();
  /**
   */
  virtual int treadstr()throw(VtyErr);
  /**
   */
  virtual bool twriteErr(const char*)throw(VtyErr);
  /**
   */
  virtual char * readstr()throw(VtyErr) ;
};

/**
*/
class VtyTerm : public Packet,public Vty {
 private:
  /**
   */
  int handle;
  /**
   */
  struct in_addr in_address;
  /**
   */
  char address[100];
  /**
   */
  int vty_height;
  /**
   */
  int vty_width;

 public:
  /**
  @param local_echo is used in @see vty_insert to control software echo in @see vty_insert_chars
  */
  bool local_echo;
  /**
   */
  void vty_clear_input();
  /**
   */
  void vty_insert_char(unsigned char);
  /**
   */
  void vty_del_char();
  /**
   */
  void vty_del_backward_char();
  /**
   */
  void vty_clear_line_from_beg();
  /**
   */
  void vty_redraw_line();
  /**
   */
  void vty_move_forward();
  /**
   */
  void vty_move_backward();
  /**
   */
  void vty_move_beginning_of_line();
  /**
   */
  void vty_move_end_of_line();
  /**
   */
  void vty_move_to(int pos);
  /**
   */
  virtual int treadstr();
  /**
   */
  char * readstr();
  /**
   */
  virtual bool twriteErr(const char*);
  /**
   */
  bool twrite(const char*);
  /**
   */
  bool twrite(const string);
  /**
   */
  bool twrite(unsigned char );
  /** @see twrite_marker_at used in error reporting*/
  void twrite_marker_at(int pos,char a='^');
  /**
   */
  unsigned char tsymbol(int &);
  /**
   */
  void echo(bool);
  /**
   */
  void linemode(bool);
  /**
   */
  void toptions();
  /**
  telnet settings */
  void tset();
  /**
  per-page output */
  void pager(const char*sourcePage,int height=0);
  /**
   */
  int get_handle();
  /**
   */
  VtyTerm(int,struct in_addr);
  /**
   */
  virtual ~VtyTerm();
};

};

#endif //__VTY_H__
