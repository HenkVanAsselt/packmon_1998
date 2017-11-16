//----------------------------------------------------------------------------
// HvA Packet monitor.
// Based on development code of SV2AGW
//
// FILE: PACKMON.CPP
// AUTH: Henk van Asselt
// SUBJ: DDE client for Packet Monitor
// HIST: 971205 V1.0
//----------------------------------------------------------------------------

#define __TRACE
#define __WARN

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
#include "buffer.h"
#include "packmon.h"
#include "pm_log.h"
// #include "ttermold.h"

#define GCL_STYLE (-26)

DEFINE_RESPONSE_TABLE1(PACKMONWnd, TFrameWindow)
  EV_WM_PAINT,
  EV_WM_INITMENU,
	EV_MESSAGE(WM_SETFONT, WMFONTS),
  EV_MESSAGE(WM_KEYDOWN,Ev_Msg_Char),
  EV_COMMAND(CM_DDECONNECT, CmConnect),
  EV_COMMAND(CM_OPENLOGFILE, CmFileOpen),
  EV_COMMAND(CM_SOUNDEFFECTS, CmToggleSoundEffects),
  EV_COMMAND(CM_OPTIONSDATA_ONLY,CmToggleDataOnly),
//  EV_COMMAND(CM_DDEDISCONNECT, CmDisConnect),
//  EV_COMMAND(CM_REQUEST, CmRequest),
//  EV_COMMAND(CM_POKE, CmPoke),
//  EV_COMMAND_AND_ID(CM_ADVISE, CmAdvise),
  EV_COMMAND(CM_HELPABOUT, CmHelpAbout),
END_RESPONSE_TABLE;

char MyCall[] = "HE1PND-2";
char Window_Title[] = "PM : Monitor & Logging";
static PACKMONWnd* This = 0;
pmBuff Buffer(7000);

PM_LOG logfile;

//////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////
LRESULT PACKMONWnd::Ev_Msg_Char(WPARAM wp,LPARAM)
{
  char *s;
  int i;

//  MessageBox("Key pressed","PACKMON",MB_OK | MB_ICONINFORMATION);

  switch(wp) {
    case VK_F1:
      // Read next 5 lines from logfile.
      for (i=0 ; i<5 ; i++) {
	      s = logfile.ReadLogData();
  	    Buffer.insert(s,strlen(s));
    	  EvPaint();
      }
      break;
    default:
      break;
  }
  return(0);
}

//////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////
void
PACKMONWnd::CmFileOpen()
{
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
    if (CanClose()) {
		  // If input file not opened yet, do it now
      logfile.OpenLogFile(data.FileName);
      s = logfile.ReadLogData();
      Buffer.insert(s,strlen(s));
      EvPaint();
    }
  }
}

void PACKMONWnd::CmToggleDataOnly()
{
  TMenu menu(GetMenu());

	if (ShowDataOnly == TRUE) {
    ShowDataOnly = FALSE;
    menu.CheckMenuItem(CM_OPTIONSDATA_ONLY, MF_UNCHECKED);
  }
  else {
    ShowDataOnly = TRUE;
    menu.CheckMenuItem(CM_OPTIONSDATA_ONLY, MF_CHECKED);
  }
  DrawMenuBar();
}

//
// This function is called when the callback function is notified of
// available data.
//
void
PACKMONWnd::ReceivedData(HDDEDATA hData)
{
	static short int size;//be carefull here for WIN95 an INT variable is 32 bit long
												//we need only 16 bit so Short INT
	static char szData2[3010];
	static char szData[3010];
	DWORD Result;

  memset(szData2,'\0',3010);
  memset(szData,'\0',3010);

	if( hData != NULL ) {
		Result=DdeGetData( hData,(LPSTR) &szData2, sizeof( szData2 ), 0 );
		if (Result<1)
			return;
		szData2[800]=NULL; //we copy the data to another string
		memmove(szData,szData2,Result+1);
		memmove(&size,szData+2,2); //take the data size (only the valaible data)
//		int port=szData[0];        //take the port number starting from 0 for first port

		switch(szData[1])         //check the kind of data we have
		{
			case 'U'://MONITOR DATA UNPROTO
			case 'T'://TXDATA MONITOR our data
			case 'S'://MONITOR HEADER
			case 'I'://MONITOR  HEADER+DATA CONNECT OTHER STATIONS
			case 'C'://CONNECT DISCONNECT messages  //status message (*** Connect)
        Buffer.insert(szData+4,size);  	// Put data in circular buffer
			  EvPaint();											// Display the data
        logfile.WriteLogData(szData+4,size);      		// Save data in log file
				break;

			default:
				  Buffer.insert("ERROR RECEIVED\r",15);
          EvPaint();
          break;

		}//END SWITCH

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
