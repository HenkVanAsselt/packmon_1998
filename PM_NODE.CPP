//----------------------------------------------------------------------------
// HvA Packet monitor - Heard window.
// Based on development code of SV2AGW
//
// FILE: PM_NODE.CPP
// AUTH: Henk van Asselt
// SUBJ: DDE client for Packet Monitor
// HIST: 980112 V0.1
//----------------------------------------------------------------------------

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
#include "pm_node.h"
#include "pm_log.h"

#define GCL_STYLE (-26)

// Response table
DEFINE_RESPONSE_TABLE1(PACKMONWnd, TFrameWindow)
	EV_WM_PAINT,
  EV_WM_INITMENU,
	EV_MESSAGE(WM_SETFONT, WMFONTS),
  EV_COMMAND(CM_DDECONNECT, CmConnect),
  EV_COMMAND(CM_OPENLOGFILE, CmFileOpen),
  EV_COMMAND(CM_HELPABOUT, CmHelpAbout),
END_RESPONSE_TABLE;

// Global variables
char MyCall[] = "HE1PND-3";
char Window_Title[] = "PM : NODE Information";
// static PACKMONWnd* This = 0;
pmBuff Buffer(6000);

NODElist nodetable(50);

PM_LOG logfile;


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

//////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////
void
PACKMONWnd::CmFileOpen()
{
  char *s;
  time_t timestamp;

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
        logfile.OpenLogFile(data.FileName);
        s = logfile.ReadLogData();
        timestamp = logfile.TimeStamp();
        if (!s)			// Break from loop if end of data is detected.
          break;
        nodetable.analyze(s,timestamp);
        nodetable.display();
        EvPaint();
      }

  }
}


//////////////////////////////////////////////////////////////////////
// cf_nodes()
// analyze  pid CF (netrom) nodes broadcast
// returns:  0 if all OK
//				  -1 if error detected
//
// Example of such a broadcast (as given by PEngine of SV2AGW):
//  	1:Fm PURMER To NODES <UI pid CF> [14:06:14]
//  	FF NL9PTW
//    NL3PND NL3PTW     PURMER 255
//    NL9PTR PTRNOD     NL9PTR 192
//////////////////////////////////////////////////////////////////////
int NODElist::analyze(char *instr, time_t timestamp) {
  char orig[20];
  char call1[20];
  char call2[20];
  char call3[20];
  int  quality;
  char *p = NULL;

  // Set pointer to start of string.

  p = instr;
  // Check if it is a NODES broadcast. If not, return
  if (!strstr(p,"To NODES"))
    return(-1);

  // Set pointer to 2nd line and get originator call
  p = skip_n_lines(p,1);
  sscanf(p,"%*s %s",orig);

  // Get all broadcast entries.
  p = skip_n_lines(p,1);
  while (*p) {
    sscanf(p,"%s %s %s %i",call1,call2,call3,&quality);
    nodetable.add(orig,call1,call2,call3,quality,timestamp);
    p = skip_n_lines(p,1);
  }

  // Display complet table and return
  nodetable.display();
  return(0);
}


//
// This function is called when the callback function is notified of
// available data.
//
void
PACKMONWnd::ReceivedData(HDDEDATA hData)
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
  char *strptr;
	DWORD Result;
  time_t timestamp;

	if( hData != NULL ) {
		Result=DdeGetData( hData,(LPSTR) &szData2, sizeof( szData2 ), 0 );
		if (Result<1)
			return;
		szData2[800]=NULL; //we copy the data to another string
		memmove(szData,szData2,(size_t)Result+1);
		memmove(&size,szData+2,2); //take the data size (only the valaible data)

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
 				strptr = szData+4;
			  if (strstr(strptr,"To NODES")) {
    			sndPlaySound("camera.wav",SND_SYNC);
          timestamp = time(NULL);
    			nodetable.analyze(szData+4,timestamp);
  			}
			break;

			case 'H'://MHeardList
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
}