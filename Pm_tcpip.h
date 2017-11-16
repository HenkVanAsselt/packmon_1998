#ifndef _PM_TCPIP_H
#define _PM_TCPIP_H

class PM_TCPIP {
  public:
  // variables
    char ip_address[22];
    char hw_address[14];
    char from_ip[22];
    char to_ip[22];
    unsigned int send_portnr;
    unsigned int rcv_portnr;
    unsigned int seq_nr;

  public:
	// functions
    PM_TCPIP(void);		// Constructor
    int  get_hwip(char *instr);
    unsigned int get_sendport(char *s);
    unsigned int get_rcvport(char *s);
    unsigned int get_seqnr(char *s);
};

#endif