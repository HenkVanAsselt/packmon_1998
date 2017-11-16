//----------------------------------------------------------------------------
// HvA Packet monitor - General routines
// Based on development code of SV2AGW
//
// FILE: PM_GEN.CPP
// AUTH: Henk van Asselt
// SUBJ: General routinges for Packet Monitor
// HIST: 980123 V0.1
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
#include "buffer.h"
#include "packmon.h"
#include "pmheard.h"

#define GCL_STYLE (-26)

static PACKMONWnd* This = 0;

//////////////////////////////////////////////////////////////////
// FUNCTION: CmConnect()
// PURPOSE:  This function is called after the Menu 'Connect; is selected
//					 If there is NO DDE connection to the SV2AGW PE, it will be made
//					 If there is a connections, it will be closed
// HIST:     980501 V0.1
//////////////////////////////////////////////////////////////////
void
PACKMONWnd::CmConnect()
{
  TMenu menu(GetMenu());

	if (DDE_Connected == TRUE) {
    PM_Exit_DDE();
    menu.CheckMenuItem(CM_DDECONNECT, MF_UNCHECKED);
  }
  else {
    PM_Init_DDE();
    menu.CheckMenuItem(CM_DDECONNECT, MF_CHECKED);
  }
  DrawMenuBar();
}

//////////////////////////////////////////////////////////////////
// FUNCTION: CmTollgeSoundEffects()
//  PURPOSE: Called by menu. Toggle Sound effects ON/OFF
//  HISTORY: 980501 V0.1
//////////////////////////////////////////////////////////////////
void
PACKMONWnd::CmToggleSoundEffects()
{
  TMenu menu(GetMenu());

	if (SoundEffects == TRUE) {
    SoundEffects = FALSE;
    menu.CheckMenuItem(CM_SOUNDEFFECTS, MF_UNCHECKED);
  }
  else {
    SoundEffects = TRUE;
    menu.CheckMenuItem(CM_SOUNDEFFECTS, MF_CHECKED);
  }
  DrawMenuBar();
}

//////////////////////////////////////////////////////////////////
// FUNCTION:
//  PURPOSE:
//  HISTORY:
//////////////////////////////////////////////////////////////////
void
PACKMONWnd::CmDisConnect()
{
  ;
}

#ifdef xxx
//////////////////////////////////////////////////////////////////
// FUNCTION:
//  PURPOSE:
//  HISTORY:
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
      inputfile = fopen(data.FileName,"r+b");

      while (TRUE) {
        if (feof(inputfile)) {
          fclose(inputfile); return;
        }
        memset(s,'\0',MAXSIZE);
        // Read header, then read the data
        // No special case for differnt types of headers build in yet.
        fread(&logheader,sizeof(LOGHEADER1),1,inputfile);
        fread(s,logheader.datalen,1,inputfile);
        heardtableanalyze(s);
        heardtable.display();
        EvPaint();
      }
//      Invalidate();
    }
  }
}
#endif

// Dummy function
// FUNCTION:
//  PURPOSE:
//  HISTORY:
void
PACKMONWnd::CmRequest()
{
  ;
}

// FUNCTION:
//  PURPOSE: Dummy function
//  HISTORY:
void
PACKMONWnd::CmPoke()
{
  ;
}

// FUNCTION:
//  PURPOSE: Dummy function
//  HISTORY:
void
PACKMONWnd::CmAdvise()
{
  ;
}

//////////////////////////////////////////////////////////////////////
// FUNCTION:	EvInitMenu
//  PURPOSE:
//  HISTORY:
//////////////////////////////////////////////////////////////////////
void
PACKMONWnd::EvInitMenu(HMENU menuHandle)
{
  // This technique is used to automatically update the status of the various
  // menu choices just before the menu is displayed.
  //
  TMenu menu(menuHandle);
  menu.EnableMenuItem(CM_DDECONNECT, !hConv[0] ? MF_ENABLED : MF_GRAYED);
//  menu.EnableMenuItem(CM_REQUEST, HConv ? MF_ENABLED : MF_GRAYED);
//  menu.EnableMenuItem(CM_POKE, HConv ? MF_ENABLED : MF_GRAYED);
//  menu.EnableMenuItem(CM_ADVISE, HConv ? MF_ENABLED : MF_GRAYED);
//  menu.CheckMenuItem(CM_ADVISE, MF_BYCOMMAND | Loop ? MF_CHECKED : MF_UNCHECKED);
  DrawMenuBar();
}



//////////////////////////////////////////////////////////////////////
// PACKMONWnd() Constructor
// FUNCTION:
//  PURPOSE:
//  HISTORY:
// Here the default values for a number of variables are set.
//////////////////////////////////////////////////////////////////////
PACKMONWnd::PACKMONWnd(TWindow* parent, const char* title)
	: TFrameWindow(parent, title),
		TWindow(parent, title)
{
  // Initialize DDE variables.
	Data[0] = 0;
	hConv[0] = 0;
	hConv[1] = 0;

  // Initialize various variables
  SoundEffects = TRUE;
  ShowDataOnly = FALSE;
}

//////////////////////////////////////////////////////////////////////
// FUNCTION:
//  PURPOSE:
//  HISTORY:
// PACKMONWnd Destructor
//////////////////////////////////////////////////////////////////////
PACKMONWnd::~PACKMONWnd()
{
#ifdef PE_ENGINE
 	PM_Exit_DDE();
#endif
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void
PACKMONWnd::SetupWindow()
{
	This = this;

	// Initialize variables.
	TextHeight=0;
	Attr.Style |= (WS_VSCROLL | WS_HSCROLL | WS_VISIBLE);

	TFrameWindow::SetupWindow();
  AssignMenu(PACKMONWnd_MENU);
//	TWindow::SetupWindow();

	// Set scrollbar stuff
	Scroller = new TScroller(this,8,15,80,60);
	SetScrollDim();
	Scroller->SetRange(260L,260L);
	Scroller->ScrollTo(0L,0L);

  // Set close button stuff
	CanIClose = 0;   // Yes, show close button on main window
	if (CanIClose) {
		long ClassStyle=GetClassLong(GCL_STYLE);
		ClassStyle |= CS_NOCLOSE;
		SetClassLong(GCL_STYLE,ClassStyle);
	}

#ifdef PE_ENGINE
	// Initialize Packmon as DDE client
	PM_Init_DDE();
#endif

}
//////////////////////////////////////////////////////////////////////
// FUNCTION:
//  PURPOSE:
//  HISTORY:
//////////////////////////////////////////////////////////////////////
void
PACKMONWnd::CmHelpAbout()
{
  MessageBox("PACKMON\nWritten by Henk van Asselt - PA3BTL\n"
             "Copyright (c) 1997, 1998 by HvA",
             "About PACKMON", MB_ICONINFORMATION);
}

//////////////////////////////////////////////////////////////////////
// FUNCTION:
//  PURPOSE:
//  HISTORY:
//////////////////////////////////////////////////////////////////////
LRESULT PACKMONWnd::WMFONTS(WPARAM wp,LPARAM)
{
	hfont=(HFONT)wp;
	SetScrollDim();
	Scroller->SetPageSize();
	Invalidate(true);
	//InvalidateRect(Scroller->HWindow, NULL, TRUE);
	return (LRESULT)1;
}

#ifdef PE_ENGINE
//////////////////////////////////////////////////////////////////////////////
// FUNCTION:
//  PURPOSE:
//  HISTORY:
// This call back function is the heart of interaction between this program
// and DDEML.  Because Windows doesn't pass C++ 'this' pointers to call
// back functions, a static 'this' pointer was used.  If you wanted to
// create a Client that would allow for more than one conversation, using a
// List of conversations and their associated 'this' pointers would be one
// possible method to try.  The XTYP_ constants are described in detail in
// the online help.
//////////////////////////////////////////////////////////////////////////////
HDDEDATA FAR PASCAL _export
PM_APPL::CallBack(WORD type, WORD, HCONV hConv, HSZ, HSZ, HDDEDATA hData,
										DWORD, DWORD)
{
// if (pStaticThis==NULL) return NULL;
/*
pStaticThis is a pointer to a window which receives all the DDEML transactions.
We need this to access the C++ class for that window
*/

	switch (type) {

		case XTYP_ADVDATA:
        // DDE server informs the client that the value of the data item has changed.
				This->ReceivedData(hData);
			return (HDDEDATA)DDE_FACK;

		case XTYP_XACT_COMPLETE:
        // asynchronous transaction, initiated by a call to the DdeClientTransaction function, has concluded.
				This->ReceivedData(hData);
			break;

		case XTYP_DISCONNECT:
			{
  			This->MessageBox("Disconnected from Packet Engine. \n Program Enters Idle Mode !",
          This->Title, MB_ICONINFORMATION);
			}
			if (hConv==This->hConv[0])
				This->hConv[0] = 0;
			else if (hConv==This->hConv[1])
				This->hConv[1] = 0;
     //      This->PostMessage(WM_INITMENU, WPARAM(This->GetMenu()));
  		break;

		case XTYP_ERROR:
			This->MessageBox("A critical DDE error has occured.", This->Title, MB_ICONINFORMATION);
			break;

		case WM_PAINT:
			This->EvPaint();
			break;

		default:
			break;
	}
	return 0;
}
#endif

//////////////////////////////////////////////////////////////////////
// FUNCTION:
//  PURPOSE:
//  HISTORY:
// By default, InitMainWindow constructs a generic TFrameWindow object with
// the name of the application as its caption. You can redefine InitMainWindow
// to construct a useful main window object of TFrameWindow (or a class derived
// from TFrameWindow) and store it in MainWindow. The main window must be a
// top-level window; that is, it must be derived from TFrameWindow.
//////////////////////////////////////////////////////////////////////
void
PM_APPL::InitMainWindow()
{
  extern char Window_Title[];
  MainWindow = new PACKMONWnd(0, Window_Title);
}

//////////////////////////////////////////////////////////////////////
// InitInstance()
// FUNCTION:
//  PURPOSE:
//  HISTORY:
// This function Performs each instance initialization necessary for the application.
// Unlike InitApplication, which is called for the first instance of an application,
// InitInstance is called whether or not there are other executing instances of
// the application. InitInstance calls InitMainWindow, and then creates and shows
// the main window element by TWindow::Create and TWindow::Show.
// If the main window cannot be created, a TXInvalidMainWindow exception is thrown.
//
// This function has been redefined here, since we want to initialize
// the DDE part first. Then we have explicitely to call the
// TApplication::InitInstance() function.
//////////////////////////////////////////////////////////////////////
void
PM_APPL::InitInstance()
{
#ifdef PE_ENGINE
	// The code below sets up the DDEML call back function that is used by the
	// DDE Management Library to carry out data transfers between applications.
	if (DdeInitialize(&InstId, (PFNCALLBACK)(FARPROC)CallBackProc, APPCMD_CLIENTONLY, 0) != DMLERR_NO_ERROR) {
			::MessageBox(0,"Initialization failed.", "DDEML Client", MB_ICONSTOP|MB_TASKMODAL);
			PostQuitMessage(0);
		}
#endif

	// Since InitInstance is redefined, explictitely call TApplication::InitInstance
	TApplication::InitInstance();
}

//////////////////////////////////////////////////////////////////////
// FUNCTION:
//  PURPOSE:
//  HISTORY:
// TermInstance()
// Handles the termination of each executing instance of an
// ObjectWindows application.
//////////////////////////////////////////////////////////////////////
int
PM_APPL::TermInstance(int status)
{
	if (InstId) {
#ifdef PE_ENGINE
		DdeUninitialize(InstId);
#endif
	}
	return TApplication::TermInstance(status);
}




