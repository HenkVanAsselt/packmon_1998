//----------------------------------------------------------------------------
// HvA Packet monitor - Heard window.
// Based on development code of SV2AGW
//
// FILE: PMHEARD.CPP
// AUTH: Henk van Asselt
// SUBJ: DDE client for Packet Monitor
// HIST: 980112 V0.1
//----------------------------------------------------------------------------


#include "pm_glob.h"

#include <owl\owlpch.h>
#include <owl\applicat.h>
#include <owl\framewin.h>
#include <owl\dc.h>
#include <owl\menu.h>
#include <owl\inputdia.h>
#include "packmon.rh"
#include <ddeml.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mmsystem.h>			// For sound play

#include "buffer.h"
#include "packmon.h"
#include "pmheard.h"
#include "pm_node.h"
#include "pm_log.h"
#include "pm_tcpip.h"

#define GCL_STYLE (-26)


// Response table
DEFINE_RESPONSE_TABLE1(PACKMONWnd, TFrameWindow)
  EV_WM_PAINT,
  EV_WM_INITMENU,
	EV_MESSAGE(WM_SETFONT, WMFONTS),
  EV_COMMAND(CM_DDECONNECT, CmConnect),
  EV_COMMAND(CM_OPENLOGFILE, CmFileOpen),
  EV_COMMAND(CM_SOUNDEFFECTS, CmToggleSoundEffects),
//  EV_COMMAND(CM_DDEDISCONNECT, CmDisConnect),
//  EV_COMMAND(CM_REQUEST, CmRequest),
//  EV_COMMAND(CM_POKE, CmPoke),
//  EV_COMMAND_AND_ID(CM_ADVISE, CmAdvise),
  EV_COMMAND(CM_HELPABOUT, CmHelpAbout),
END_RESPONSE_TABLE;



// Global variables
char MyCall[] = "HE1PND6";
char Window_Title[] = "PM : Heard Stations";
// static PACKMONWnd* This = 0;
pmBuff Buffer(6000);
HEARDlist heardtable(50);
NODElist nodetable(50);

PM_LOG logfile;


//////////////////////////////////////////////////////////////////
// FUNCTION: IsMultiCall()
//  PURPOSE: Check if parameter 'call' is a unique call or a
//					 multi-call (broadcast) address
//  HISTORY: 980605 V0.1
//////////////////////////////////////////////////////////////////
bool
IsMultiCall(char *call)
{
  char *multiCallList[] = {
    "ID",
    "K40",
    "FBB",
    "BEACON",
    "ID",
    "CQ",
    "NODES",
    "QST",
    NULL,
  };
  int i;

  for (i=0 ; multiCallList[i] ; i++) {
    if (!strcmp(call,multiCallList[i]))
      return(TRUE);
  }
  return(FALSE);
}




//////////////////////////////////////////////////////////////////////
// Skip n lines
//////////////////////////////////////////////////////////////////////
char *skip_n_lines(char *p, int n)
{
  int i;

  for (i=0 ; i<n ; i++) {
    while(*p && *p != '\n' && *p != '\r')				// Find CR or LF
      *p++;
    while (*p == '\n' || *p == '\r')			// Skip next CR's and LF's too
      *p++;
  }
  return(p);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// PID STUFF
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class PID {
  protected:
  // variables
    int pid;

  public:
	// functions
    int  setpid(char *s);
    int  setpid(int i);
    int  getpid(void);
    char *getstr(char *s);

};

//////////////////////////////////////////////////////////////////////
// setpid(char *s)
// Convert sting, containg PID in 2 hex characters, to an integer
// The integer's hex value is the same as the string, i.e.
// "F0" is returned as 0xF0
//////////////////////////////////////////////////////////////////////
int
PID::setpid(char *s)
{
  sscanf(s,"%x",&pid);
  return(pid);
}

//////////////////////////////////////////////////////////////////////
// setpid(int i)
//////////////////////////////////////////////////////////////////////
int
PID::setpid(int i)
{
  pid = i;
  return(pid);
}

//////////////////////////////////////////////////////////////////////
// getstr(char *s)
// Convert pid to string.
// The string has to be declared outside this function
// If the input pid was invalid, the string will be made empty
//////////////////////////////////////////////////////////////////////
char *
PID::getstr(char *s)
{
  if (pid > 0 && pid < 0xFF)
    sprintf(s,"%02X",pid);
  else
    s[0] = '\0';
  return(s);
}

//////////////////////////////////////////////////////////
// int getpid(void)
//////////////////////////////////////////////////////////
int
PID::getpid()
{
  return(pid);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// FRAMETYPE STUFF
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// str2frame()
// Convert sting, containg frametype, to an integer
//////////////////////////////////////////////////////////////////////
void
FRAME::set(char *s)
{
  // Set default frametype
  frametype = FRAME_UNKNOWN;

  if (!strcmp(s,"UI"))
    frametype = FRAME_UI;
  else if (!strcmp(s,"UA"))
    frametype = FRAME_UA;
  else if (!strcmp(s,"RR"))
    frametype = FRAME_RR;
  else if (!strcmp(s,"REJ"))
     frametype = FRAME_REJ;
  else if (!strcmp(s,"DM"))
    frametype = FRAME_DM;
  else if (!strcmp(s,"DISC"))
    frametype = FRAME_DISC;
  else if (!strcmp(s,"I"))
    frametype = FRAME_I;
  else if (!strcmp(s,"SABM"))
    frametype = FRAME_SABM;
}

//////////////////////////////////////////////////////////////////////
// FUNCTION: frame2str()
// PURPOSE : Convert frametype to string
// RETUrNS : Pointer to the string filled.
// NOTES   : String s has to be defined outside this function
// HISTORY : 980208 V0.1
//////////////////////////////////////////////////////////////////////
char *
FRAME::getstr()
{
  switch(frametype){
    case FRAME_UI  : return("UI");
    case FRAME_UA  : return("UA");
    case FRAME_RR  : return("RR");
    case FRAME_REJ : return("REJ");
    case FRAME_DM  : return("DM");
    case FRAME_DISC: return("DISC");
    case FRAME_I   : return("I");
    case FRAME_SABM: return("SABM");
    default  : return("");
  }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////
void
PACKMONWnd::CmFileOpen()
{
  #define MAXSIZE 1024
  char *s;

  TMenu menu(GetMenu());
	if (DDE_Connected == TRUE) {
    PM_Exit_DDE();
    menu.CheckMenuItem(CM_DDECONNECT, MF_UNCHECKED);
    DrawMenuBar();
  }

  static TOpenSaveDialog::TData data (
    OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
    "PM Log Files (*.LOG)|*.log|",
    0,
    0,
    "LOG"
  );
  if (TFileOpenDialog(this, data).Execute() == IDOK) {
    while (TRUE) {
      // strcpy(ElementName, data.FileName);  // Remember the file to open.
      logfile.OpenLogFile(data.FileName);
      s = logfile.ReadLogData();
      if (!s)			// Break from loop if end of data is detected.
        break;
      heardtable.analyze(s,logfile.TimeStamp());
      heardtable.display();
      EvPaint();
    }
  }
}



//////////////////////////////////////////////////////////////////////
// is_mail()
// Check if received a broadcast to station address MAIL
// Check the mail entries,
// Examples are:
// 1:Fm EU2PTR To MAIL <UI pid F0> [21:31:09]
//   Mail for: PA3FCE GE
// 1:Fm NL3PND To MAIL <UI pid F0> [21:34:00]
//   27-Jan 21:33 <<< Mailbox NL3PND Purmerend >>> 1647 aktive berichten.
//   Post voor:=> ALL BB1PND
//////////////////////////////////////////////////////////////////////
int HEARDlist::is_mail(char *instr)
{
  char *ptr = NULL;
  int  i = 0;
  char *key_string[]= {"Mail for:", "Post voor:", "" };
  char s[256];

  // Check if this was a broadcast to address MAIL
  if (strstr(instr,"To MAIL")) {
    return(1);		// Success
  }

  // Check if the information contains one of the key strings
  strcpy(s,instr);
  for (i=0 ; *key_string[i] ; i++) {
    ptr = strstr(s,key_string[i]);
    if (ptr)
      break;
  }

  if (!ptr)
    return(0);    // String not found
  else
    return(1); 		// String found

}


//////////////////////////////////////////////////////////////////////
// HEARlist() Constructor
//////////////////////////////////////////////////////////////////////
HEARDlist::HEARDlist(int size)
{
  max = size;											// Remember the size of the table
  heardtable = new _heardentry[size];	// Allocate memory for table
  no_stations = 0;								// Set actual number of stations

  // Clear complete heardtable at once.
  memset(heardtable,0,size*sizeof(_heardentry));
}


//////////////////////////////////////////////////////////////////////
// FUNCTION: find_connection()
// PURPOSE : Check if the connection between 'from' to 'to'
//           is already in the heard list
// RETURNS:  index in heard list if the connection is found
//					 -1 if the connection is not found
//////////////////////////////////////////////////////////////////////
int HEARDlist::find_connection(char *from, char *to)
{
  int i;

  for (i=0 ; i<max ; i++) {
    if (!strcmp(heardtable[i].call,from) && !strcmp(heardtable[i].tocall,to))
      return(i);
  }
  return(-1);
}

//////////////////////////////////////////////////////////////////////
// add station to heard list
// returns: 0 if station was already in the list.
// 					1 if station is added to the list.
//////////////////////////////////////////////////////////////////////
int HEARDlist::add(char *call,char *tocall, char *path, time_t timeStamp, char *pidstr, FRAME frame, int ns, int nr)
{
  int index;
  PID pid;

  // Check if still room in table
  if (no_stations >= max)
    return(0);

  // Check if station is not already in the list
  if (find_connection(call,tocall) > -1)
    return(0);

  // Index is no_stations at this moment
  index = no_stations;

  // Store data
  strcpy(heardtable[index].call,call);
  strcpy(heardtable[index].tocall,tocall);
  strcpy(heardtable[index].path,path);
  heardtable[index].first_time = timeStamp ;
  heardtable[index].last_time = timeStamp;

  heardtable[index].pid |= pid.setpid(pidstr);
  heardtable[index].frame = frame;
  heardtable[index].no_frames = 1;									// This is the 1st frame

  /*
  // 980419: I don't know how to resolve this one.....
   switch(frame.frametype) {
    case FRAME_UI: heardtable[index].UI_cntr++;
    							 break;
    case FRAME_RR: heardtable[index].RR_cntr++;
    							 break;
    case FRAME_REJ: heardtable[index].REJ_cntr++;
    							 break;
    case FRAME_DM: heardtable[index].DM_cntr++;
    							 break;
    case FRAME_DISC: heardtable[index].DISC_cntr++;
    							 break;
    case FRAME_I: heardtable[index].I_cntr++;
    							 break;
    case FRAME_SABM: heardtable[index].SABM_cntr++;
    							 break;
	  default: 			 break;
  }
  */

  // Increment station counter
  no_stations++;
  return(no_stations);
}

//////////////////////////////////////////////////////////////////////
// update()
// update station in the heard list
// returns 0 if all OK
//////////////////////////////////////////////////////////////////////
int HEARDlist::update(int index,char *path, time_t timeStamp,char *pidstr, FRAME frame, int ns, int nr)
{
  PID pid;
  // Store variables
  heardtable[index].last_time = timeStamp;
  strcpy(heardtable[index].path,path);
  heardtable[index].pid |= pid.setpid(pidstr);
  heardtable[index].frame = frame;
  heardtable[index].no_frames += 1;

  /*
  // 980419: I don't know yet how to resolve this one
  // Upate frame counters
  switch(frametype) {
    case FRAME_UI: heardtable[index].UI_cntr++;
    							 break;
    case FRAME_RR: heardtable[index].RR_cntr++;
    							 break;
    case FRAME_REJ: heardtable[index].REJ_cntr++;
    							 break;
    case FRAME_DM: heardtable[index].DM_cntr++;
    							 break;
    case FRAME_DISC: heardtable[index].DISC_cntr++;
    							 break;
    case FRAME_I: heardtable[index].I_cntr++;
    							 break;
    case FRAME_SABM: heardtable[index].SABM_cntr++;
    							 break;
	  default: 			 break;
  }
  */

  return(0);
}

//////////////////////////////////////////////////////////////////////
// display()
// display table of heard stations.
//////////////////////////////////////////////////////////////////////
void HEARDlist::display()
{
  int i;
  char s[256];
  int len = 0;
  char pidstr[32];
  PID pid;
  time_t currentTime;
  time_t diffTime;
  unsigned int days,hours,minutes;

  Buffer.reset();			// Reset buffer pointers as if it is empty

  // Print header
  sprintf(s," # FROM       TO         PID TYPE TOT \n");
  Buffer.insert(s,strlen(s));
  sprintf(s,"== ========== ========== === ==== === \n");
  Buffer.insert(s,strlen(s));


  // Print station data of the table
	for (i=0 ; i<no_stations ; i++) {
    pid.setpid(heardtable[i].pid);
    len = 0;
    len += sprintf(s+len,"%2d ",      i+1);
    len += sprintf(s+len,"%-10.10s ", heardtable[i].call);
    if (IsMultiCall(heardtable[i].tocall))
      len += sprintf(s+len,"*");
    else
      len += sprintf(s+len," ");

    len += sprintf(s+len,"%-10.10s ", heardtable[i].tocall);
//    len += sprintf(s+len,"%3.3s ",    frame.getstr());
    len += sprintf(s+len,"%4.4s ",    pid.getstr(pidstr));
    len += sprintf(s+len,"%3d ",      heardtable[i].no_frames);
    Buffer.insert(s,strlen(s));

    // Print path
    /*
    if (strlen(heardtable[i].path) != 0) {
      sprintf(s,"Via %s ",heardtable[i].path);
      Buffer.insert(s,strlen(s));
    }
    */

    // Print age
      currentTime = time(NULL);
      diffTime = difftime(currentTime,heardtable[i].last_time);
      days = diffTime / 20864;
      hours =  (diffTime - (days*20864)) / 3600;
      minutes = (diffTime - ((days*20864) + (hours*60))) / 60;
      sprintf(s,"[%2dd/%02d:%02d] ",
        days,hours,minutes);
      Buffer.insert(s,strlen(s));

    // Print counters. The order is illogical and irrelevant at the moment
    /*
    sprintf(s,"%2d %2d %2d %2d %2d %2d %2d",
      heardtable[i].UI_cntr,
      heardtable[i].I_cntr,
      heardtable[i].RR_cntr,
      heardtable[i].REJ_cntr,
      heardtable[i].DM_cntr,
      heardtable[i].DISC_cntr,
      heardtable[i].SABM_cntr
    );
    Buffer.insert(s,strlen(s));
    */

    // Print the IP address (if any)
    if (heardtable[i].ip_address[0]) {
      sprintf(s,"IP = [%s]  ",heardtable[i].ip_address);
      Buffer.insert(s,strlen(s));
    }

    // Print the ID string (if any)
    if (heardtable[i].idstr) {
      sprintf(s,"\nID = \"%s\"  ",heardtable[i].idstr);
      Buffer.insert(s,strlen(s));
    }

    // Print the mailstring (if any)
    if (heardtable[i].mailstr) {
      sprintf(s,"\n          %s",heardtable[i].mailstr);
      Buffer.insert(s,strlen(s));
    }

    // End with CR/LF
    sprintf(s,"\n");
    Buffer.insert(s,strlen(s));

  }
}

//////////////////////////////////////////////////////////////////////
// analyze_cd()
// analyze pid=CD (ARP) transmission
// returns: > 0 if all OK
//				  < 0 if error detected
//
// ARP Example:
// 1:Fm NL9PND-10 To NL1MRK-1 <UI pid=CD Len=30 > [20:21:03]
// ARP: len 30 hwtype AX25 prot IP op REQUEST
// sender IPaddr 27.137.16.1 hwaddr NL9PND-10
// target IPaddr 27.137.18.21
//////////////////////////////////////////////////////////////////////
int HEARDlist::analyze_cd(char *instr) {
  char *p;
  int i;
  int ret = 0;

  // Check if this is really an ARP message
  if (!strstr(instr,"pid=CD") || !strstr(instr,"ARP"))
    return(-1);
  // Check if this is really an request or an reply
  if (!strstr(instr,"REQUEST") && !strstr(instr,"REPLY"))
    return(-1);

  if (SoundEffects)
    sndPlaySound("bye.wav",SND_SYNC);

  // Get pointer to start of 3rd line (info on request)
  p = instr;
  for (i=0 ; i<2 ; i++) {
    while(*p != '\n' && *p != '\r')				// Find CR or LF
      *p++;
    while (*p == '\n' || *p == '\r')			// Skip next CR's and LF's too
      *p++;
  }
  ret = analyze_arp(p);

  if (strstr(instr,"REQUEST"))
    return(ret);

  // if this is an REPLY, goto the 4th line and get the reply data too
  while(*p != '\n' && *p != '\r')				// Find CR or LF
    *p++;
  while (*p == '\n' || *p == '\r')			// Skip next CR's and LF's too
    *p++;
  ret = analyze_arp(p);

  return(ret);
}

//////////////////////////////////////////////////////////////////////
// FUNCTION: analyze_tcpip()
//
// Example:
//  1:Fm NL9PND-10 To NL1MRK-1 <UI pid=CC Len=150 [21:53:28]
//  IP: len 150 27.137.16.1->27.137.18.21 ihl 20 ttl 24 prot TCP
//  TCP: 1036->25 Seq xc9e7f3b9 Ack x14de2062 ACK Wnd 2048 Data 110
//  Topics covered in this issue include:
//    1) new gateway
//	  by Orrin Winton <orrin@mail.redshift.com>
//////////////////////////////////////////////////////////////////////
int HEARDlist::analyze_tcpip(char *instr)
{
   char *p = NULL;
   PM_TCPIP data;


   // Set pointer to start of input string
   p = instr;
   // Search for start of 2nd line
   while (*p != '\n' && *p !='\r')
     p++;
   while (*p == '\n' || *p == '\r')
     p++;
   // Search for start of 3rd line
   while (*p != '\n' && *p !='\r')
     p++;
   while (*p == '\n' || *p == '\r')
     p++;
	 data.get_sendport(p);
   data.get_rcvport(p);
   data.get_seqnr(p);


   return(0);
}

//////////////////////////////////////////////////////////////////////
// analyze_arp()
// analyze pid CD (TCPIP) - ARP transmission
// returns: > 0 if all OK
//				  < 0 if error detected
//
// ARP Example:
//
// "1:Fm NL9PND-10 To NL1MRK-1 <UI pid=CD Len=30 > [20:21:03]"
// "ARP: len 30 hwtype AX25 prot IP op REQUEST               "
// "sender IPaddr 27.137.16.1 hwaddr NL9PND-10               "
// "target IPaddr 27.137.18.21                               "
////////////////////////////////////////////////////////////////////////
int HEARDlist::analyze_arp(char *instr) {
  char s[512];
  PM_TCPIP data;
  int ret = 0;
  int i;

  // We are already on the 3rd line of the string

  // Copy the input string
  strcpy(s,instr);

  // Get 3rd token of the string
  ret = data.get_hwip(s);
  if (ret > 0) {
     i = find_call(data.hw_address);
     if (i > 0) {
       strcpy(heardtable[i].ip_address,data.ip_address);
     }
  }

  return(ret);
}


//////////////////////////////////////////////////////////////////////
// analyze_cf()
// analyze pid CF (netrom) transmission
// returns:  0 if all OK
//				  -1 if error detected
//
//////////////////////////////////////////////////////////////////////
int HEARDlist::analyze_cf(char *instr) {
  char *tok = NULL;
  char s[512];
  char from_call[20];
  char to_call[20];
  char *p;

  if (SoundEffects)
    sndPlaySound("bye.wav",SND_SYNC);

  // We play it very save to get FROM call
  strcpy(s,instr);					// Make an copy of the input string
  p = strstr(s,"Fm");				// Find the "Fm" identifier
  if (!p)
    return(-1);							// No From call detected.
  tok = strtok(p+3," ");  	// Fm
  if (tok)
    strcpy(from_call,tok);

  // We play it very save to get TO call
  strcpy(s,instr);					// Make an copy of the input string
  p = strstr(s,"To");				// Find the "To" identifier
  if (!p)
    return(-1);							// No From call detected.
  tok = strtok(p+3," ");  	// Fm
  if (tok)
    strcpy(to_call,tok);



  return(0);									// Return ALL OK.
}



//////////////////////////////////////////////////////////////////////
// find_call()
// Find call in heardlist.
// Returns: > 0 index in heard list
//          < 0 if call is not found
//////////////////////////////////////////////////////////////////////
int HEARDlist::find_call(char *call)
{
  int i = -1;

  for (i=0 ; i<max ; i++) {
    if (strlen(heardtable[i].call) == 0)
      return(-1);
    if (!strcmp(call,heardtable[i].call)) {
      return(i);
    }
  }
  return(-1);		// Not found
}

//////////////////////////////////////////////////////////////////////
// add_idstr()
// Add id string to call in heard list.
// returns: -1 if no success
//////////////////////////////////////////////////////////////////////
int HEARDlist::add_idstr(char *call, char *s)
{
  int i = -1;
  char *tok;

  i = find_call(call);
  if (i > -1) {			// If found
    tok = strtok(s,"\r");					// Skip first line
    if (!tok)
      return(-1);
    tok = strtok(NULL,"\r");
    if (!tok)
      return(-1);
    if (heardtable[i].idstr) {
      free(heardtable[i].idstr);
    }
    heardtable[i].idstr = strdup(tok);
  }
  return(i);
}

//////////////////////////////////////////////////////////////////////
// add_mailstr()
// Add "mail for" string to call in heard list.
// Returns index in station list if found
//         -1 in case of general error.
//////////////////////////////////////////////////////////////////////
int HEARDlist::add_mailstr(char *call, char *instr)
{
  int i = -1;
  char *tok;
  char *s;
  char *p;

  s = strdup(instr);		// Make copy of input string

  i = find_call(call);
  if (i == -1)
    return(-1);			// Return if call not found in list

  tok = strtok(s,"\r");
  while(tok) {
    p = strstr(tok,"Mail for");
    if (p) {
    	if (heardtable[i].mailstr) {
     	 free(heardtable[i].mailstr);
    	}
    	heardtable[i].mailstr = strdup(p);
      free(s);
      return(i);
  	}
    tok= strtok(NULL,"\r");
  }
  return(-1);		// Error
}

//////////////////////////////////////////////////////////////////////
// analyze()
// analyze data by received data string from PE engine
// returnfs:  0 if all OK
//				  -1 if error detected
//////////////////////////////////////////////////////////////////////
int HEARDlist::analyze(char *instr, time_t timeStamp)
{
  #define MAXPATHLEN 100

  char s[512];
  char from_call[20];
  char to_call[20];
  FRAME frame;
  char pidstr[20];
  int  msglen=0;
  int  index = -1;
  char path[MAXPATHLEN+1];
  char *p = NULL;
  int  port;
  int ns = -1 , nr = -1;

  // Example input strings
  // 1:Fm EU2PTR To ID Via TCP <UI pid=F0 Len=64 > [21:31:37]
  // 1:Fm NL1PTP To CQ <UI pid=F0 [DAMA] Len=144 > [21:30:25]
  // 1:Fm NL3PND To FBB <UI pid=F0 Len=10 > [21:20:44]
  // 1:Fm NL9PTP To NODES <UI pid=CF Len=70 > [21:15:55]

  // Initialize strings
  from_call[0] = '\0';
  to_call[0] = '\0';
  pidstr[0] = '\0';
  path[0]='\0';

  // Get first 3 parameters, which is the same for every frame
  sscanf(instr,"%d:%*s %s %*s %s",&port,from_call,to_call);


  // If the keyword Via is found, extract the path
  p = strstr(instr,"Via");			// Find the "Via" identifier
  if (p) {
    p+=4;
    sscanf(p,"%s,",path);
  }


  // Determine the frame type (UI, I, RR etc.)
  p = strstr(instr,"<");
  if (p) {
    sscanf(p,"<%s",s);
    frame.set(s);
  }

  if (frame.frametype == FRAME_I) {
		sscanf(p,"<%*s R%d S%d",&nr,&ns);
  }

  // Get PID
  p = strstr(instr,"pid=");
  if (p) {
    sscanf(p,"pid=%s",pidstr);
  }

  // Get message length
  p = strstr(instr,"Len=");
  if (p) {
    sscanf(p,"Len=%d",&msglen);
  }

  // If station already in the heardlist, update data,
  // else add the station to the heardlist
  index = find_connection(from_call,to_call);
  if (index > -1)
    update(index,path,timeStamp,pidstr,frame,ns,nr);
  else
    add(from_call,to_call,path,timeStamp,pidstr,frame,ns,nr);

  // Analyze pid=CF connection or broadcast
  if (!strcmp(pidstr,"CF"))
    analyze_cf(instr);

  // Analyze pid=CD connections or broadcasts
  if (!strcmp(pidstr,"CD"))
    analyze_cd(instr);

  // Analyze pid=CC (TCPIP) connections or broadcasts
  if (!strcmp(pidstr,"CC"))
    analyze_tcpip(instr);

  // Check Special TO CALLS and perform action.
  if (!strcmp(to_call,"NODES")) {
    if (SoundEffects)
      sndPlaySound("camera.wav",SND_SYNC);
  }
  else if (!strcmp(to_call,"MAIL")) {
    if (SoundEffects)
      sndPlaySound("meepmeep.wav",SND_SYNC);
    add_mailstr(from_call,instr);
  }
  else if (!strcmp(to_call,"FBB")) {
    if (SoundEffects)
      sndPlaySound("whoosh.wav",SND_SYNC);
   }
   else if (!strcmp(to_call,"ID") || !strcmp(to_call,"BEACON")) {
     add_idstr(from_call,instr);
  }

  return(0);
}

//
// This function is called when the callback function is notified of
// available data.
//
#ifdef PE_ENGINE
void
PACKMONWnd::ReceivedData(HDDEDATA hData)
#else
PACKMON::ReceivedData(char *s)
#endif
{
	/*
	This function is called when the callback function is notified of
	available data.and receives the data.
	*/
	//Term[x] is the program window (each for radioport)
	static short int size;//be carefull here for WIN95 an INT variable is 32 bit long
												//we need only 16 bit so Short INT
	static char szData2[3010];
	static char szData[3010];
	DWORD Result;
  time_t currentTime;

	if( hData != NULL ) {

#ifdef PE_ENGINE
		Result=DdeGetData( hData,(LPSTR) &szData2, sizeof( szData2 ), 0 );
		if (Result<1)
			return;
		szData2[800]='\0';					//we copy the data to another string
		memmove(szData,szData2,(size_t)Result+1);
		memmove(&size,szData+2,2); //take the data size (only the valaible data)
		int port=szData[0];        //take the port number starting from 0 for first port
#endif

		switch(szData[1])         //check the kind of data we have
		{
			case 'D'://DATA from AX25 CONNECT Channel
				//do here anything you wish
				break;

			case 'T'://TXDATA MONITOR our data
			case 'U'://MONITOR DATA UNPROTO
			case 'S'://MONITOR HEADER
			case 'I'://MONITOR  HEADER+DATA CONNECT OTHER STATIONS
        if (SoundEffects)
          sndPlaySound("type.wav",SND_SYNC);
        currentTime = time(NULL);
        heardtable.analyze(szData+4,currentTime);
        heardtable.display();
			break;

			case 'H'://MHeardList
					char temp[200];
					long FirstTime,LastTime;
					memmove(&FirstTime,szData+13,sizeof(long));
					memmove(&LastTime,szData+13+sizeof(long),sizeof(long));
					char *t1, t2[30];
					time_t t;;
					t=(time_t) FirstTime;t1=ctime(&t);
					strcpy(t2,t1);
					t=(time_t) LastTime;t1=ctime(&t);
					wsprintf(temp,"Port%d>%9.9s %24.24s  %24.24s\r",
					port+1,szData+2,t2,t1);
          Buffer.insert(temp,strlen(temp));
				break;

			default:
				  Buffer.insert("ERROR RECEIVED\r",15);
          break;

		}//END SWITCH

    EvPaint();

	}
}


///////////////////////////////////////////////////////
// get_PEheard()
// Get heard list from the SV2AGW packet engine.
// HIST: 980120 V0.2
///////////////////////////////////////////////////////
void PACKMONWnd::get_PEheard()
{
  static char szTemp[50];
	char str[12];
  short int count;
  extern char MyCall[];

	str[0]='H';
  str[1]='\0';
	count=strlen(str);
  szTemp[0]='S';      // 'S' = Send
  szTemp[1]=0;				// Port number
  memmove(szTemp+2,MyCall,strlen(MyCall)+1);
	memmove(szTemp+12,MyCall,strlen(MyCall)+1);
	memmove(szTemp+22,&count,2);
	memmove(szTemp+24,str,count);
 // TXDATA(szTemp,count+24);
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
int
OwlMain(int /*argc*/, char* /*argv*/ [])
{
	return PM_APPL().Run();
			// Initializes the instance, calling InitApplication for the first executing
      // instance and InitInstance for all instances. If the initialization is
      // successful, Run calls MessageLoop and runs the application.
      // If exceptions are thrown outside the message loop, Run catches these exceptions.
}
