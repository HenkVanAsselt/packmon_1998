//----------------------------------------------------------------------------
// HvA Packet monitor - Heard window.
// Based on development code of SV2AGW
//
// FILE: PM_TCPIP.CPP
// SUBJ: This file contains all stuff which has to do with IP decoding
// AUTH: Henk van Asselt
// HIST: 980112 V0.1
//----------------------------------------------------------------------------

#include "pm_glob.h"
#include <stdio.h>
#include <string.h>

#include "pm_tcpip.h"

//////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////
PM_TCPIP::PM_TCPIP()
{
  ip_address[0] = '\0';
  hw_address[0] = '\0';
}

//////////////////////////////////////////////////////////////////////
// get_hwip()
// Retrieve from string the Senders HW address and IP address
// Example of an input string, as decoded by SV2AGV packet engine:
// "sender IPaddr 27.137.16.1 hwaddr NL9PND-10"
// Returns < 0 on error
//         > 0 if all OK
//////////////////////////////////////////////////////////////////////
PM_TCPIP::get_hwip(char *instr)
{
  // Make copy of input string
  sscanf(instr,"%*s %*s %s %*s %s",ip_address,hw_address);
  return(1);				// All OK
}




//===============================================================
// Example of a IP TCP SYN MESSAGE
//
// 1:Fm NL9PND-10 To NL1MRK-1 <UI pid=CC Len=44 [22:32:49]
// IP: len 44 27.137.16.1->27.137.18.21 ihl 20 ttl 24 prot TCP
// TCP: 1037->25 Seq x38d0e000 SYN Wnd 2048 MSS 216
//===============================================================

//===============================================================
// Example of a IP TCP RST Frame:
//
//   1:Fm NL9PND-10 To NL1MRK-1 <UI pid=CC Len=40 [22:57:18]
//  IP: len 40 27.137.16.1->27.137.18.21 ihl 20 ttl 24 prot TCP
//  TCP: 1037->25 Seq x38d0e001 RST Wnd 0
//===============================================================

//===============================================================
// Example of a IP TCP DATA Frame:
//
//  1:Fm NL9PND-10 To NL1MRK-1 <UI pid=CC Len=150 [21:53:28]
//  IP: len 150 27.137.16.1->27.137.18.21 ihl 20 ttl 24 prot TCP
//  TCP: 1036->25 Seq xc9e7f3b9 Ack x14de2062 ACK Wnd 2048 Data 110
//  Topics covered in this issue include:
//    1) new gateway
//	  by Orrin Winton <orrin@mail.redshift.com>
//    2) Re
//===============================================================

//////////////////////////////////////////////////////////////////////
// FUNCTION: 	get_sendport()
// PURPOSE:  	Return portnumber of destination of TCP frame
// EXAMPLE:   TCP: 1036->25 Seq xc9e7f3b9 Ack x14de2062 ACK Wnd 2048 Data 110
// RETURNS:   portnumber (1036 in the example)
//////////////////////////////////////////////////////////////////////
unsigned int
PM_TCPIP::get_sendport(char *s)
{
  unsigned int port=0;

  sscanf(s,"%*s: %d",&port);
  send_portnr = port;
  return(port);		// default
}


//////////////////////////////////////////////////////////////////////
// FUNCTION: 	get_rcvport()
// PURPOSE:  	Return portnumber of source of TCP frame
// EXAMPLE:   TCP: 1036->25 Seq xc9e7f3b9 Ack x14de2062 ACK Wnd 2048 Data 110
// RETURNS:   portnumber (25 in the example)
//////////////////////////////////////////////////////////////////////
unsigned int
PM_TCPIP::get_rcvport(char *s)
{
  unsigned int port=0;

  sscanf(s,"%*s: %*d->%d",&port);
  rcv_portnr = port;
  return(port);		// default
}
//////////////////////////////////////////////////////////////////////
// FUNCTION: 	get_seqnr()
// PURPOSE:  	Return sequence number of a TCP frame
// EXAMPLE:   TCP: 1036->25 Seq xc9e7f3b9 Ack x14de2062 ACK Wnd 2048 Data 110
// RETURNS:   sequencenumber ( xc9e7f3b9 in the example)
//////////////////////////////////////////////////////////////////////
unsigned int
PM_TCPIP::get_seqnr(char *s)
{
  unsigned int i=0;
  unsigned int j=0;
  unsigned long k=0;
  char s1[20];
  char s2[20];
  char s3[20];
  char s4[20];
  char s5[20];

  sscanf(s,"TCP: %x->%x Seq x%lx",&i,&j,&k);
  sscanf(s,"TCP: %*d->%*d Seq %x",&i);
  seq_nr = i;
  return(i);		// default
}




