//////////////////////////////////////////////////////////////////////////////
// FILE: PMHEARD.H
// HIST: 980120 V0.1
//////////////////////////////////////////////////////////////////////////////

#ifndef _PM_HEARD_H
#define _PM_HEARD_H

#define MAXHEARD 50

typedef struct _heardentry {
  char call[15];
  char tocall[15];
  char path[50];
  time_t first_time;
  time_t last_time;
  unsigned int pid;
  int  port;
  FRAME frame;
  char *idstr;
  char *mailstr;
  char ip_address[20];
  // Counters
  unsigned int  no_frames;
  unsigned int  UI_cntr;
  unsigned int  UA_cntr;
  unsigned int  RR_cntr;
  unsigned int  REJ_cntr;
  unsigned int  DM_cntr;
  unsigned int  DISC_cntr;
  unsigned int  I_cntr;
  unsigned int  SABM_cntr;
};

class HEARDlist : PM_GLOBAL {
	public:
  	// functions
		HEARDlist(int size = MAXHEARD);    	// constructor
	  //	virtual ~PACKMONWnd();
    int find_connection(char *from, char *to);
    int is_mail(char *instr);
    int add(char *call,char *tocall, char *path, time_t timeStamp, char *pid, FRAME frame,int ns, int nr);
    int update(int index,char *path, time_t timeStamp, char *pid, FRAME frame, int ns, int nr);
    void display(void);
    int find_call(char *call);
    int add_idstr(char *call, char *s);
    int add_mailstr(char *call, char *s);

    int analyze(char *instr, time_t timeStamp);
    int analyze_cf(char *instr);
    int analyze_cd(char *instr);
    int analyze_arp(char *instr);
    int analyze_mail(char *instr);
    int analyze_fbb(char *instr);
    int analyze_tcpip(char *instr);

    // variables
    int no_stations;			// Number of stations heard
    int max;							// Maximum number of stations in heard list
    _heardentry *heardtable;		// Table of heard stations
};



typedef struct _fbbentry {
  char call[15];
  int  lastmsg;
  char last_time[10];
};

class FBBlist {
	public:
  	// functions
		FBBlist(int size = MAXHEARD);    	// constructor
	  //	virtual ~PACKMONWnd();
    int add(char *call, int lastmsg, char *timestr);
    int update(int index, int lastmsg, char *timestr);
    void display(void);
    int find_call(char *call);
    int analyze(char *instr);

    // variables
    int no_stations;			// Number of stations heard
    int max;							// Maximum number of stations in heard list
    _fbbentry *fbbtable;		// Table of heard stations
};

#endif   // #ifdef _PM_HEARD_H

