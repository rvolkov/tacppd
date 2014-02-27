// icmp-based device availability poller
// (c) Copyright in 2000-2008 by Roman Volkov and contributors
// Visit http://tacppd.org for more information
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted, see COPYING file

// ICMP ping code fragments used here getted from netfu project
// with next COPYRIGHT notice:

/*
Copyright (c) 2002 Regents of The University of Michigan.
All Rights Reserved.

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of The University
of Michigan not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission. This software is supplied as is without expressed or
implied warranties of any kind.

Research Systems Unix Group
The University of Michigan
c/o Wesley Craig
4251 Plymouth Road B1F2, #2600
Ann Arbor, MI 48105-2785
nefu@umich.edu
http://rsug.itd.umich.edu/software/nefu/
*/

#include "global.h"

#define ATTEMPTS                3                // three tries default
#define TIME                    5                // one sec plenty for default
#define ICMP_SIZE               8                // rfc 1885 page 14
#define SIZEOF_TIMEVAL          8                // as per sizeof call
#define SIZEOF_IP_HEAD          20               // as per sizeof call
#define MESSAGE                 "tacppd test"    // message
#define MESSAGE_LEN             strlen( MESSAGE ) + 1
#define SEND_MESSAGE_OFFSET     ICMP_SIZE + SIZEOF_TIMEVAL
#define RECV_MESSAGE_OFFSET     SEND_MESSAGE_OFFSET + SIZEOF_IP_HEAD
#define SEND_PACKET_SIZE        ICMP_SIZE + SIZEOF_TIMEVAL + MESSAGE_LEN
#define RECV_PACKET_SIZE        SEND_PACKET_SIZE + SIZEOF_IP_HEAD

// make icmp socket
int
Icmp::make_socket() {
  sock = 0;
  sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if(sock < 0) {
    startlog(LOG_ERROR)<<"Cant create ICMP socket"<<endlog;
  }
  return sock;
}

// check sum
int
Icmp::in_cksum(unsigned short *addr, int len) {
  int nleft = len;
  unsigned short *w = addr;
  unsigned short answer;
  register int sum = 0;
  // Our algorithm is simple, using a 32 bit accumulator (sum),
  // we add sequential 16 bit words to it, and at the end, fold
  // back all the carry bits from the top 16 bits into the lower
  // 16 bits.
  while(nleft > 1)  {
    sum += *w++;
    nleft -= 2;
  }
  // mop up an odd byte, if necessary
  if(nleft == 1) {
    *(unsigned char*)(&answer) = *(unsigned char*)w;
    sum += answer;
    //sum += htons(*(u_char *)w << 8);
  }
  // add back carry outs from top 16 bits to low 16 bits
  sum = (sum >> 16) + (sum & 0xffff);   // add hi 16 to low 16
  sum += (sum >> 16);                   // add carry
  answer = ~sum;                        // truncate to 16 bits
  return (answer);
}

int
Icmp::time_start(struct timeval *tv) {
  return (gettimeofday(tv, NULL));
}

int
Icmp::time_end(struct timeval *tv) {
  struct timeval now;
  if(gettimeofday(&now, NULL) == -1) return -1;
  if(now.tv_usec < tv->tv_usec) {
    now.tv_usec += 1000000;
    now.tv_sec--;
  }
  tv->tv_sec = (now.tv_sec - tv->tv_sec);
  tv->tv_usec = (now.tv_usec - tv->tv_usec);
  return 0;
}

int
Icmp::time_minus(struct timeval *total, struct timeval *minus) {
  if(total->tv_usec < minus->tv_usec) {
    total->tv_usec += 1000000;
    total->tv_sec--;
  }
  total->tv_sec -= minus->tv_sec;
  total->tv_usec -= minus->tv_usec;
  if((total->tv_sec < 0) || (total->tv_usec < 0)) return -1;
  return 0;
}

// send test icmp echo packet and receive echo reply
bool
Icmp::test_ping(struct sockaddr_in t_sin) {
  //static
  unsigned int sequence_number = 1;
  struct timeval tv_remain;// time until select time out
  struct timeval tv_remain2;
  struct timeval tv_timeout;
  struct timeval tv_wait;
  fd_set readfds;
  int attempts;
  int send_wrote;// how much did sendto write?
  int recv_read;// how much did recvfrom read?
  int attempt;
  int fromlen;
  int readyfds;
  //int same_host = 0;

  // these are the packets, and pointers in to the packets

  struct icmp *icmp_send;
  struct icmp *icmp_recv;
  unsigned char packet_send[SEND_PACKET_SIZE]; // packet to send
  bzero(packet_send,sizeof(unsigned char)*SEND_PACKET_SIZE);
  unsigned char packet_recv[RECV_PACKET_SIZE]; // packet to recv
  bzero(packet_recv,sizeof(unsigned char)*RECV_PACKET_SIZE);
  struct timeval *tv_send; // area that stores time we sent it
  struct timeval *tv_recv; // area that stores time we sent it
  char *send_message; // message we send
  char *recv_message; // message we recv
  struct sockaddr_in sin_recv; // should be our address
  struct ip *ip_recv;

  attempts = ATTEMPTS;
  tv_timeout.tv_sec = TIME;
  tv_timeout.tv_usec = 0;

  fromlen = sizeof(struct sockaddr_in);
  // set up packets
  ip_recv = (struct ip *)packet_recv;
  icmp_send = (struct icmp *)packet_send;
  icmp_send->icmp_type = ICMP_ECHO;
  int pid = getpid();
  icmp_send->icmp_id = pid; // reasonably unique key
  icmp_recv = (struct icmp*)(packet_recv + SIZEOF_IP_HEAD);
  tv_send = (struct timeval*)(packet_send + ICMP_SIZE);
  tv_recv = (struct timeval*)(packet_recv + ICMP_SIZE + SIZEOF_IP_HEAD);
  send_message = (char*)(packet_send + SEND_MESSAGE_OFFSET);
  recv_message = (char*)(packet_recv + RECV_MESSAGE_OFFSET);
  strcpy(send_message, MESSAGE);
  // try to send the packets

  // send only packet
  attempt = 0;

  icmp_send->icmp_seq = sequence_number;
  sequence_number++;
  tv_remain = tv_timeout;
  // get time of day for packet
  if(time_start(tv_send) < 0) {
    //report_printf( r, "time_start %m" );
//std::cerr<<"Icmp::test_ping point 0"<<std::endl;
    return false;
  }
  icmp_send->icmp_cksum = 0;
  icmp_send->icmp_cksum = in_cksum((unsigned short*)icmp_send,SEND_PACKET_SIZE);

//std::cerr<<"Icmp::test_ping point 1 host="<<inet_ntoa(t_sin.sin_addr)<<std::endl;

// disable SIGPIPE
#ifdef SOLARIS
  void (*old_sigpipe)(int) = 0;
#else
  sig_t old_sigpipe = 0;
#endif
  old_sigpipe = signal (SIGPIPE, SIG_IGN);

//  if((send_wrote = sendto(sock,(char *)icmp_send,
//        SEND_PACKET_SIZE,MSG_NOSIGNAL,(struct sockaddr *)&t_sin,
//          sizeof(struct sockaddr_in))) < 0) {

  if((send_wrote = sendto(sock,(char *)icmp_send,
        SEND_PACKET_SIZE,0x0,(struct sockaddr *)&t_sin,
          sizeof(struct sockaddr_in))) < 0) {

    //report_printf( r, "sendto: %m" );
//std::cerr<<"Icmp::test_ping point 1.1"<<std::endl;

// back the way it was..
    signal (SIGPIPE, old_sigpipe);

    return false;
  } else {

// back the way it was..
    signal (SIGPIPE, old_sigpipe);

  } // fi

//std::cerr<<"Icmp::test_ping point 2"<<std::endl;

  while((tv_remain.tv_sec > 0) || (tv_remain.tv_usec > 0)) {
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    // get start time

//std::cerr<<"Icmp::test_ping point 3"<<std::endl;

    if(time_start(&tv_wait) < 0) {
      //report_printf( r, "time_start %m" );
//std::cerr<<"Icmp::test_ping point 4"<<std::endl;
      return false;
    }

//std::cerr<<"Icmp::test_ping point 5"<<std::endl;
    tv_remain2 = tv_remain;
    if((readyfds = select(sock + 1,&readfds,NULL,NULL,&tv_remain2)) < 0) {
      //report_printf( r, "select: %m" );
//std::cerr<<"Icmp::test_ping point 6"<<std::endl;
      time_end(&tv_wait);
      return false;
    } else if(readyfds == 0) {
      // socket NOT ready to be read, and timed out
//std::cerr<<"Icmp::test_ping point 7"<<std::endl;
      time_end(&tv_wait);
      break;
    } else {
      if(time_end(&tv_wait) < 0) {
        //report_printf( r, "time_end: %m" );
//std::cerr<<"Icmp::test_ping point 8"<<std::endl;
        return false;
      }
      if(time_minus(&tv_remain,&tv_wait) < 0) {
//std::cerr<<"Icmp::test_ping point 9"<<std::endl;
        break;
      }
      if((tv_remain.tv_sec == 0) && (tv_remain.tv_usec == 0)) {
//std::cerr<<"Icmp::test_ping point 10"<<std::endl;
        break;
      }
      // read the damn socket
      if((recv_read = recvfrom(sock,packet_recv,RECV_PACKET_SIZE,
          0,(struct sockaddr*)&sin_recv,(socklen_t*)&fromlen)) < 0) {
        //report_printf( r, "recvfrom: %m" );
//std::cerr<<"Icmp::test_ping point 11"<<std::endl;
        return false;
      }
      if(bcmp(&sin_recv.sin_addr,&t_sin.sin_addr,sizeof(struct in_addr)) == 0) {
//std::cerr<<"Icmp::test_ping point 12"<<std::endl;
        if((icmp_recv->icmp_type == ICMP_ECHOREPLY) &&
              (icmp_recv->icmp_id == icmp_send->icmp_id) &&
                (icmp_recv->icmp_seq >= icmp_send->icmp_seq -
                  attempt) && (strcmp(send_message,
                    recv_message) == 0)) {
//std::cerr<<"Icmp::test_ping point 13"<<std::endl;
          // packet is echo reply packet, from our pid, with our
          // message, in a range of packets that we'll consider.
          if(time_end(tv_recv) < 0) {
            //report_printf( r, "time_end: %m" );
//std::cerr<<"Icmp::test_ping point 14"<<std::endl;
            return false;
          }
          //syslog( LOG_INFO, "%s %s %s %d %ld.%.6ld", t->t_rlist,
          //  t->t_machine->m_name, t->t_name, attempt + 1,
          //    tv_recv->tv_sec, tv_recv->tv_usec );
//std::cerr<<"Icmp::test_ping point 15"<<std::endl;
          return true;
        }
      }
    }
  }
//std::cerr<<"Icmp::test_ping point 16"<<std::endl;
  return false;
}

// constructor - make socket
Icmp::Icmp() {
  sock = -1;
  make_socket();
}

// destr - close socket
Icmp::~Icmp() {
  if(sock >= 0) close(sock);
}

// test
bool
Icmp::test(ipaddr ip) {
  struct sockaddr_in sin;
  //socklen_t size_sin = sizeof(struct sockaddr);
  bzero((char*)&sin,sizeof(sin));
  sin.sin_family = AF_INET;
  //sin.sin_port = htons();
  //inet_aton(ip.get().c_str(),&sin.sin_addr);
  sin.sin_addr.s_addr = inet_addr(ip.get().c_str());
  bool ret = test_ping(sin);
  return ret;
}

////////////// that is all ///////////////
