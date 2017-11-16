//----------------------------------------------------------------------------
// HvA Packet monitor - FBB window.
// Based on development code of SV2AGW
//
// FILE: PM_FBB.CPP
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

#define GCL_STYLE (-26)

/*
LOGFONT   cursfont;                           // font structure
HFONT     holdsfont;                          // handle of original font
HFONT     hnewsfont;                          // handle of new fixed font
*/

// Response table
DEFINE_RESPONSE_TABLE1(PACKMONWnd, TFrameWindow)
  EV_WM_PAINT,
  EV_WM_INITMENU,
	EV_MESSAGE(WM_SETFONT, WMFONTS),
  EV_COMMAND(CM_DDECONNECT, CmConnect),
  EV_COMMAND(CM_OPENLOGFILE, CmFileOpen),
//  EV_COMMAND(CM_DDEDISCONNECT, CmDisConnect),
//  EV_COMMAND(CM_REQUEST, CmRequest),
//  EV_COMMAND(CM_POKE, CmPoke),
//  EV_COMMAND_AND_ID(CM_ADVISE, CmAdvise),
  EV_COMMAND(CM_HELPABOUT, CmHelpAbout),
END_RESPONSE_TABLE;



// Global variables
char MyCall[] = "HE1PND-7";
char Window_Title[] = "PM : FBB Stations";
static PACKMONWnd* This = 0;
pmBuff Buffer(1500);
FBBlist fbbtable(50);

//////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////
void
PACKMONWnd::CmFileOpen()
{
  #define MAXSIZE 1024
  char s[MAXSIZE];

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
    if (CanClose()) {
      // strcpy(ElementName, data.FileName);  // Remember the file to open.
      logfile = fopen(data.FileName,"r+b");

      while (TRUE) {
        if (feof(logfile)) {
          fclose(logfile); return;
        }
        memset(s,'\0',MAXSIZE);
        // Read header, then read the data
        // No special case for differnt types of headers build in yet.
        fread(&logheader,sizeof(LOGHEADER1),1,logfile);
        fread(s,logheader.datalen,1,logfile);
        fbbtable.analyze(s);
        fbbtable.display();
        EvPaint();
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////
// analyze()
// Received a broadcast to FBB interested stations
// Check the contents of the broadcast,
// Examples are:
// 1:Fm NL3PTR To FBB Via OBDAM <UI pid F0> [21:08:45]
// 3979   !!
// Returns number of last message
//         -1 if error found.
//		      0 if not last message number, but no errors found
//////////////////////////////////////////////////////////////////////
int FBBlist::analyze(char *instr)
{
  char s[512];
  char *tok = NULL;
  char *p = NULL;
  int lastmsg = 0;
  int index;
  char from_call[20];
  char timestr[20];

  // Check if the string meets the conditions
  if (!(strstr(instr,"To FBB"))) {
    return(-1);
  }

  // Copy the input string
  strcpy(s,instr);

  // get FROM call
  strcpy(s,instr);					// Make an copy of the input string
  p = strstr(s,"Fm");				// Find the "Fm" identifier
  if (!p)
    return(-1);
  tok = strtok(p+3," ");
  if (tok)
    strcpy(from_call,tok);

  // Get time
  strcpy(s,instr);
  p = strstr(s,"[");
  if (!p)
    return(-1);
  tok = strtok(p+1,"]");
  if (tok)
    strcpy(timestr,tok);

  // Get pointer to 2nd line in 'tok'
  strcpy(s,instr);
  tok = strtok(s,"\r");	  	// Skip first line
  if (!tok)
    return(-1);
  tok = strtok(NULL,"\r");	// Get 2nd line
  if (!tok)
    return(-1);

  // Get last message of station
  if (strstr(tok,"!!")) {
    lastmsg = atoi(tok);
  }

  // If station already in the fbblist, update data,
  // else add the station to the fbb list
  index = find_call(from_call);
  if (index > -1)
    update(index,lastmsg,timestr);
  else
    add(from_call,lastmsg,timestr);

  sndPlaySound("whoosh.wav",SND_SYNC);

  // Return all OK
  return(0);
}

//////////////////////////////////////////////////////////////////////
// FBBlist() Constructor
//////////////////////////////////////////////////////////////////////
FBBlist::FBBlist(int size)
{
  max = size;											// Remember the size of the table
  fbbtable = new _fbbentry[size];	// Allocate memory for table
  no_stations = 0;								// Set actual number of stations

  // Clear complete fbb table at once.
  memset(fbbtable,0,size*sizeof(_fbbentry));
}

//////////////////////////////////////////////////////////////////////
// add station to fbb list
// returns: 0 if station was already in the list.
// 					1 if station is added to the list.
//////////////////////////////////////////////////////////////////////
int FBBlist::add(char *call, int lastmsg, char *timestr)
{
  int index;

  // Check if still room in table
  if (no_stations >= max)
    return(0);

  // Check if station is not already in the list
  if (find_call(call) > -1)
    return(0);

  // Index is no_stations at this moment
  index = no_stations;

  // Store data
  strcpy(fbbtable[index].call,call);
  strcpy(fbbtable[index].last_time,timestr);
  fbbtable[index].lastmsg = lastmsg;

  // Increment station counter
  no_stations++;
  return(no_stations);
}

//////////////////////////////////////////////////////////////////////
// update()
// update station in the fbb list
// returns 0 if all OK
//////////////////////////////////////////////////////////////////////
int FBBlist::update(int index, int lastmsg, char *timestr)
{
  // Store variables
  strcpy(fbbtable[index].last_time,timestr);
  fbbtable[index].lastmsg = lastmsg;

  return(0);
}

//////////////////////////////////////////////////////////////////////
// display()
// display table of fbb stations.
//////////////////////////////////////////////////////////////////////
void FBBlist::display()
{
  int i;
  char s[256];

  Buffer.reset();			// Reset buffer pointers as if it is empty

  // Print header
  sprintf(s," # CALL    LASTMSG    time\n\n");
  Buffer.insert(s,strlen(s));

  // Print station data of the table
	for (i=0 ; i<no_stations ; i++) {
    sprintf(s,"%2d %-10.10s %d %-10.10s\n",
      i+1,
      fbbtable[i].call,
      fbbtable[i].lastmsg,
      fbbtable[i].last_time);
    Buffer.insert(s,strlen(s));
  }
}

//////////////////////////////////////////////////////////////////////
// find_call()
// Find call in FBBlist.
// Returns: index in fbb list
//////////////////////////////////////////////////////////////////////
int FBBlist::find_call(char *call)
{
  int i = -1;

  for (i=0 ; i<max ; i++) {
    if (!strcmp(call,fbbtable[i].call)) {
      return(i);
    }
  }
  return(-1);		// Not found
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

	if( hData != NULL ) {

#ifdef PE_ENGINE
		Result=DdeGetData( hData,(LPSTR) &szData2, sizeof( szData2 ), 0 );
		if (Result<1)
			return;
		szData2[800]='\0';					//we copy the data to another string
		memmove(szData,szData2,Result+1);
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
        sndPlaySound("type.wav",SND_SYNC);
        fbbtable.analyze(szData+4);
        fbbtable.display();
			break;

			default:
				  Buffer.insert("ERROR RECEIVED\r",15);
          break;

		}//END SWITCH

    EvPaint();

	}
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
