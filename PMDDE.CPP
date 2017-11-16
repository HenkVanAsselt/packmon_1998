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
#include "packmon.h"

#ifdef PE_ENGINE

HSZ            hszService;
HSZ            hszTopic[2];
HSZ            hszItem[6];

char TTitle[] = "HvA's Packmon";
char BBSCall[] = "HE1PND";

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
int
PACKMONWnd::PM_Init_DDE()
{
	int Speed = 0;
	DWORD dwTempResult;
	HDDEDATA Result;
  extern char MyCall[];
  int error = FALSE;

	hszService = DdeCreateStringHandle( InstId(), "SV2AGW Packet Engine Server", CP_WINANSI );
	hszTopic[0] = DdeCreateStringHandle( InstId(), "SV2AGW HUMAN", CP_WINANSI );
	hszTopic[1] = DdeCreateStringHandle( InstId(), "HVAPACKMON", CP_WINANSI );//here enter your program name instead of AGWFWD
	hszItem[0] = DdeCreateStringHandle( InstId(), "MONDATA", CP_WINANSI );
	hszItem[1] = DdeCreateStringHandle( InstId(), "INFODATA", CP_WINANSI );
	hszItem[2] = DdeCreateStringHandle( InstId(), "PORTS", CP_WINANSI );
	hszItem[3] = DdeCreateStringHandle( InstId(), "HEARD", CP_WINANSI );
	hszItem[4] = DdeCreateStringHandle( InstId(), "USERS", CP_WINANSI );
	hszItem[5] = DdeCreateStringHandle( InstId(), "STATUSDATA", CP_WINANSI );
	if( (hszService == NULL) || (hszTopic[0] == NULL)||(hszTopic[1] == NULL) || (hszItem[0] == NULL)|| (hszItem[1] == NULL)
		|| (hszItem[2] == NULL)|| (hszItem[3] == NULL)|| (hszItem[4] == NULL)|| (hszItem[5] == NULL)) {
			 MessageBox(  "Creation of strings failed.", TTitle, MB_ICONSTOP );
       error = TRUE;
	}

  if (!error) {
  	hConv[0] = DdeConnect( InstId(), hszService, hszTopic[0], NULL );
	  if (hConv[0])
		  ;
  	else {
	  	MessageBox(  "Can't start Monitor Conversation.\nTry Running First Packet Engine (AGWPE.EXE).\nProgram Enters Idle Mode !", TTitle, MB_ICONSTOP );
      error = TRUE;
    }
	}

  if (!error) {
  	if (Speed==0)//SPEED i use it for debbuging only
	  	Result=DdeClientTransaction(NULL,0,hConv[0],hszItem[0],CF_TEXT,XTYP_ADVSTART|XTYPF_ACKREQ,1000,&dwTempResult);
	  else
	   	Result=DdeClientTransaction(NULL,0,hConv[0],hszItem[0],CF_TEXT,XTYP_ADVSTART,1000,&dwTempResult);
	  if (!Result) {
		  MessageBox("Can't start Monitor Advise Loop.\nTry Running First Packet Engine (AGWPE.EXE).", TTitle, MB_ICONSTOP );
      error = TRUE;
	  }
  }

	//REGISTER PACKETENGINE THIS APPLICATION STRING FOR DATA ADVISE LOOP
	// HvA: Done in MainWndProc
  if (!error) {
  	char tt[50];strcpy(tt,"HVAPACKMON");//here enter the same string as in hsztopi[1]
	  char str[100];
	  short count=strlen(tt);//short int is 2bytes long
  	str[1]=0;
	  str[0]='X';
		memmove(str+22,&count,2);
		memmove(str+24,tt,count+1);
		if (!DdeClientTransaction( (LPBYTE)str,count+1+24,hConv[0],hszItem[0],CF_TEXT,XTYP_POKE,1000,NULL )) {
			MessageBox(  "Can't Register Application.\nTry Running First Packet Engine (AGWPE.EXE).", TTitle, MB_ICONSTOP );
    	error = TRUE;
    }
	}

	//CONNECT to receive connected DATA
  if (!error) {
		hConv[1] = DdeConnect( InstId(), hszService, hszTopic[1], NULL );
		if( hConv[1] == 0 ) {
			MessageBox( "Can't start AX25 Conversation.\nTry Running First Packet Engine (AGWPE.EXE).", TTitle, MB_ICONSTOP );
			error = TRUE;
    }
	}

	//START ADVISE LOOP
	// HvA: Done in MainWndProc
  if (!error) {
 		if (Speed==0)
			Result=DdeClientTransaction(NULL,0,hConv[1],hszItem[1],CF_TEXT,XTYP_ADVSTART|XTYPF_ACKREQ,1000,&dwTempResult);
		else
			Result=DdeClientTransaction(NULL,0,hConv[1],hszItem[1],CF_TEXT,XTYP_ADVSTART,1000,&dwTempResult);
		if (!Result) {
			MessageBox("Can't start AX25 Advise Loop.\nTry Running First Packet Engine (AGWPE.EXE).", TTitle, MB_ICONSTOP );
    	error = TRUE;
    }
	}

	///register the mycall for that application (each application a dfferent call
	// HvA: Done in MainWndProc
  if (!error) {
		if (!DdeClientTransaction( (LPBYTE)MyCall,strlen(MyCall)+1,hConv[1],hszItem[1],CF_TEXT,XTYP_POKE,1000,NULL )) {
			MessageBox( "Can't Register MyCall.\nTry Running First Packet Engine (AGWPE.EXE).", TTitle, MB_ICONSTOP );
	    error = TRUE;
    }
	}

  if (!error) {
		if (Speed==0)
			Result=DdeClientTransaction(NULL,0,hConv[1],hszItem[5],CF_TEXT,XTYP_ADVSTART|XTYPF_ACKREQ,1000,&dwTempResult);
		else
			Result=DdeClientTransaction(NULL,0,hConv[1],hszItem[5],CF_TEXT,XTYP_ADVSTART,1000,&dwTempResult);
		if (!Result) {
			MessageBox( "Can't start AX25STATUS Advise Loop.\nTry Running First Packet Engine (AGWPE.EXE).", TTitle, MB_ICONSTOP );
      error = TRUE;
    }
	}

	//to receive monitor heard list
  if (!error) {
		if (Speed==0)
			Result=DdeClientTransaction(NULL,0,hConv[1],hszItem[3],CF_TEXT,XTYP_ADVSTART|XTYPF_ACKREQ,1000,&dwTempResult);
		else
			Result=DdeClientTransaction(NULL,0,hConv[1],hszItem[3],CF_TEXT,XTYP_ADVSTART,1000,&dwTempResult);
  	if (!Result) {
			MessageBox( "Can't start  HeardList Advise Loop.\nTry Running First Packet Engine (AGWPE.EXE).", TTitle, MB_ICONSTOP );
    	error = TRUE;
    }
	}

  // Adapt menu
  if (!error) {
 	  TMenu menu(GetMenu());
  	menu.CheckMenuItem(CM_DDECONNECT, MF_CHECKED);
  	DDE_Connected = TRUE;
  	return(0);
  }
  else {
    DDE_Connected = FALSE;
    PostQuitMessage( 0 );
    return(-1);
  }
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
PACKMONWnd::PM_Exit_DDE()
{
	// Clean up resource of DDEML conversations
	if (hConv[0])
		DdeDisconnect(hConv[0]);     // Let the other party know we are leaving

	if (hConv[1])
		DdeDisconnect(hConv[1]);     // Let the other party know we are leaving

	if (InstId()) {
		DdeFreeStringHandle(InstId(), Service);
		DdeFreeStringHandle(InstId(), Topic);
		DdeFreeStringHandle(InstId(), Item);
	}
  DDE_Connected = FALSE;
	return(0);
}
#endif

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
#ifdef xxx
void PACKMONWnd::TXDATA(char *str,int count)
{
#ifdef PE_ENGINE
  if (hConv[0]!=0)
		for (int x=0;x<3;x++)
	    if (DdeClientTransaction( (LPBYTE)str,count+1,hConv[0],hszItem[3],CF_TEXT,XTYP_POKE,1000,NULL ))
        return;
		  else continue;
#endif
}
#endif


