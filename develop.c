/*
The AGWPacketEngine program manages the traffic between the TNCs
and the packet applications. That way it is possible to have
different packet applications sharing the same TNCs.
It is also more easy to develop a packet application because
you don’t have to deal direct with tncs, serial ports, protocol
messages (connect, disconnect)etc.
To build a new packet application you must use the DDEML.
Lets see how with code samples in C++ (Borland).
First you must use 2 conversations, 2 topics and 5 items.
*/

/*
FIRST OF ALL
*************
set up the DDEML call back function that is used by the
DDE Management Library to carry out data transfers between applications.
this procedure depends upon your compiler
*/

#define STRICT

#include <windows.h>
#pragma hdrstop
#include <ddeml.h>
#include <dde.h>
#include <windowsx.h>

#include <stdio.h>
#include <string.h>

/*
			The DDE variables
*/

DWORD          idInst = 0L;            /*  Instance of app for DDEML       */
FARPROC        lpDdeProc;              /*  DDE callback function           */
HSZ            hszService;
HSZ            hszTopic[2];
HSZ            hszItem[6];
HCONV          hConv = (HCONV)NULL;    /*Handle of established conversation*/
HDDEDATA       hData;
DWORD          dwResult;
WORD           wFmt = CF_TEXT;         /*  Clipboard format                */
char           szDDEString[80];        /*  Local allocation of data buffer */
char           szDDEData[80];          /*  Local receive data buffer       */
int            iClientCount = 0;       /*  Client to Server message counter*/
char           tbuf[5];                /*  Temporary, to hold count        */

char szAppName[] = "DDEClientApplication";

//*******************************************************

/* HvA: Implemented Callback */
HDDEDATA FAR PASCAL _export
 TAGWFWDBBS::CallBack( WORD wType, WORD, HCONV hConv, HSZ hsz1, HSZ hsz2, HDDEDATA hData, DWORD, DWORD )
{
if (pStaticThis==NULL) return NULL;
/*
pStaticThis is a pointer to a window which receives all the DDEML transactions.
We need this to access the C++ class for that window
*/
	 switch( wType )
	 {
	case XTYP_ADVDATA :
				//FOR MONITOR and AX25 DATA
				pStaticThis->ReceivedData( hData );//function to receive
				return (HDDEDATA)DDE_FACK;
			case XTYP_XACT_COMPLETE :
					pStaticThis->ReceivedData( hData );
				break;
			case XTYP_DISCONNECT :
		 {
		 ::MessageBox( 0, "Disconnected From Packet Engine.\n Program Enters Idle Mode !", TTitle, MB_ICONINFORMATION );
	strcpy(pStaticThis->BbsStrStatus,"Idle Mode! No Server Connection !");
		 pStaticThis->BBSOK=0;
		 }
		 if (hConv==pStaticThis->hConv[0])   pStaticThis->hConv[0] = 0;
		 else if (hConv==pStaticThis->hConv[1])   pStaticThis->hConv[1] = 0;
		 break;
			case XTYP_ERROR :
				::MessageBox(0, "A critical DDE error has occured.", TTitle, MB_ICONINFORMATION );
				break;
			default :
				break;
	 }
	 return NULL;
}

//*******************
// HvA: implemented this routine in MainWndProc
void TAGWFWDBBS::InitInstance()
{
//initialize the callbach function
 if (DdeInitialize(&idInst, (PFNCALLBACK)(FARPROC)CallBackProc, APPCMD_CLIENTONLY, 0) != DMLERR_NO_ERROR) {
		::MessageBox(0,"Initialization failed.", "DDEML Client", MB_ICONSTOP|MB_TASKMODAL);
		PostQuitMessage(0);
	 }
TApplication::InitInstance();
}

//***************
// HvA: This function is implemented in WinMain
int TAGWFWDBBS::TermInstance(int status)
{
//when application finishes execution
if (idInst) DdeUninitialize(idInst);
sndPlaySound("bye.wav",SND_SYNC);
return TApplication::TermInstance(status);
}
//**************
DWORD FWDWIN::idInst()
{
//return the application instance
return((TAGWFWDBBS*)GetApplication())->idInst;
}
//*********


//First step Register strings.
// HvA: Done in MainWndProc
//****************************
hszService = DdeCreateStringHandle( idInst, "SV2AGW Packet Engine Server", CP_WINANSI );
hszTopic[0] = DdeCreateStringHandle( idInst, "SV2AGW HUMAN", CP_WINANSI );
hszTopic[1] = DdeCreateStringHandle( idInst, "AGWFWD DATA", CP_WINANSI );//here enter your program name instead of AGWFWD
hszItem[0] = DdeCreateStringHandle( idInst, "MONDATA", CP_WINANSI );
hszItem[1] = DdeCreateStringHandle( idInst, "INFODATA", CP_WINANSI );
hszItem[2] = DdeCreateStringHandle( idInst, "PORTS", CP_WINANSI );
hszItem[3] = DdeCreateStringHandle( idInst, "HEARD", CP_WINANSI );
hszItem[4] = DdeCreateStringHandle( idInst, "USERS", CP_WINANSI );
hszItem[5] = DdeCreateStringHandle( idInst, "STATUSDATA", CP_WINANSI );
if( (hszService == NULL) || (hszTopic[0] == NULL)||(hszTopic[1] == NULL) || (hszItem[0] == NULL)|| (hszItem[1] == NULL)
		|| (hszItem[2] == NULL)|| (hszItem[3] == NULL)|| (hszItem[4] == NULL)|| (hszItem[5] == NULL))
				{
					 MessageBox(  "Creation of strings failed.", TTitle, MB_ICONSTOP );
					 PostQuitMessage( 0 );
				}

//SECOND STEP Establish communication with AGW PACKET ENGINE
//***********************************************************
//CONNECT For MONITOR
// HvA: Done in MainWndProc
hConv[0] = DdeConnect( idInst(), hszService, hszTopic[0], NULL );
if( hConv[0] == 0 )
{
	MessageBox(  "Can't start Monitor Conversation.\nTry Running First Packet Engine (AGWPE.EXE).\nProgram Enters Idle Mode !", TTitle, MB_ICONSTOP );
	BBSStatusTextG->SetText("BBS OFF:No RadioPorts");//text in status bar
	BBSOK=0;//variable to show if connection with PE is OK.
}

DWORD dwTempResult;
HDDEDATA Result;

//START ADVISE LOOP
// HvA: Done in MainWndProc
if (Speed==0)//SPEED i use it for debbuging only
Result=DdeClientTransaction(NULL,0,hConv[0],hszItem[0],CF_TEXT,XTYP_ADVSTART|XTYPF_ACKREQ,1000,&dwTempResult);
else
Result=DdeClientTransaction(NULL,0,hConv[0],hszItem[0],CF_TEXT,XTYP_ADVSTART,1000,&dwTempResult);
	if (!Result)
	{MessageBox("Can't start Monitor Advise Loop.\nTry Running First Packet Engine (AGWPE.EXE).", TTitle, MB_ICONSTOP );
	BBSStatusTextG->SetText("BBS OFF:No RadioPorts");
	BBSOK=0;
		  }

//REGISTER PACKETENGINE THIS APPLICATION STRING FOR DATA ADVISE LOOP
// HvA: Done in MainWndProc
char tt[50];strcpy(tt,"AGWFWD DATA");//here enter the same string as in hsztopi[1]
char str[100];
short count=strlen(tt);//short int is 2bytes long
str[1]=0;
str[0]='X';
memmove(str+22,&count,2);
memmove(str+24,tt,count+1);
if (!DdeClientTransaction( (LPBYTE)str,count+1+24,hConv[0],hszItem[0],CF_TEXT,XTYP_POKE,1000,NULL ))
	{MessageBox(  "Can't Register Application.\nTry Running First Packet Engine (AGWPE.EXE).", TTitle, MB_ICONSTOP );
	BBSStatusTextG->SetText("BBS OFF:No RadioPorts");
	BBSOK=0;
		 }
//CONNECT to receive connected DATA
// HvA: Done in MainWndProc
	 hConv[1] = DdeConnect( idInst(), hszService, hszTopic[1], NULL );
	 if( hConv[1] == 0 )
	{MessageBox( "Can't start AX25 Conversation.\nTry Running First Packet Engine (AGWPE.EXE).", TTitle, MB_ICONSTOP );
	 BBSStatusTextG->SetText("BBS OFF:No RadioPorts");
	BBSOK=0;
	}
//START ADVISE LOOP
// HvA: Done in MainWndProc
if (Speed==0)
Result=DdeClientTransaction(NULL,0,hConv[1],hszItem[1],CF_TEXT,XTYP_ADVSTART|XTYPF_ACKREQ,1000,&dwTempResult);
else
Result=DdeClientTransaction(NULL,0,hConv[1],hszItem[1],CF_TEXT,XTYP_ADVSTART,1000,&dwTempResult);
	if (!Result)
	{MessageBox("Can't start AX25 Advise Loop.\nTry Running First Packet Engine (AGWPE.EXE).", TTitle, MB_ICONSTOP );
	BBSStatusTextG->SetText("BBS OFF:No RadioPorts");
	BBSOK=0;
	}
///register the mycall for that application (each application a defferent
//call
// HvA: Done in MainWndProc
if (!DdeClientTransaction( (LPBYTE)MyCall,strlen(MyCall)+1,hConv[1],hszItem[1],CF_TEXT,XTYP_POKE,1000,NULL ))
	{MessageBox( "Can't Register MyCall.\nTry Running First Packet Engine (AGWPE.EXE).", TTitle, MB_ICONSTOP );
	BBSStatusTextG->SetText("BBS OFF:No RadioPorts");
	BBSOK=0;
	}
if (Speed==0)
Result=DdeClientTransaction(NULL,0,hConv[1],hszItem[5],CF_TEXT,XTYP_ADVSTART|XTYPF_ACKREQ,1000,&dwTempResult);
else
Result=DdeClientTransaction(NULL,0,hConv[1],hszItem[5],CF_TEXT,XTYP_ADVSTART,1000,&dwTempResult);
			if (!Result)
	{MessageBox( "Can't start AX25STATUS Advise Loop.\nTry Running First Packet Engine (AGWPE.EXE).", TTitle, MB_ICONSTOP );
	BBSStatusTextG->SetText("BBS OFF:No RadioPorts");
	BBSOK=0;
	}
//to receive monitor heard list
// HvA: Done in MainWndProc
if (Speed==0)
Result=DdeClientTransaction(NULL,0,hConv[1],hszItem[3],CF_TEXT,XTYP_ADVSTART|XTYPF_ACKREQ,1000,&dwTempResult);
else
Result=DdeClientTransaction(NULL,0,hConv[1],hszItem[3],CF_TEXT,XTYP_ADVSTART,1000,&dwTempResult);

			if (!Result)
	{MessageBox( "Can't start  HeardList Advise Loop.\nTry Running First Packet Engine (AGWPE.EXE).", TTitle, MB_ICONSTOP );
	 BBSStatusTextG->SetText("BBS OFF:No RadioPorts");
	BBSOK=0;
	}

GetPorts(); //ask the Packet engine for available radioports
//check if port>maxports

//Create your windows now
//***********************
//****************************************************************
void FWDWIN::GetPorts(void)
{
/*
it receives the port information from PE.You know that way how many ports are
attached to PacketEngine.
the received string has the follow format
[How Many Ports];[port description];...;...;[port description]
everything is in ASCII format.
eg.
2;Port0 with kam HF;Port1 with Kam VHF;
2 ports and their description.
to access the first port use the number 0 for second the number 1 and so on.
*/
Ports.Num=0;lstrcpy(Ports.Port[0],"No Server Connection !");
if (hConv[0]!=0)
{
HDDEDATA hData;
static char szData[3010];
hData=DdeClientTransaction(NULL,0,hConv[0],hszItem[2],
				CF_TEXT,XTYP_REQUEST,100000L,NULL);
if (hData==NULL)
{
uint z;
z=DdeGetLastError(idInst());
wsprintf(szData,"Error %d",z);
MessageBox(szData,"AGWFWD");
}
if (hData)
{
DdeGetData(hData,&szData,1999,0);
char *token;
token=strtok(szData,";");
Ports.Num=atoi(token);
for (int x=0;x<Ports.Num;x++)
	{
	token=strtok(NULL,";");
	if (token==NULL) return;
	strcpy(Ports.Port[x],token);
	}
 return;
}// hdata

}//hconv
}

void FWDWIN::ReceivedData( HDDEDATA hData )
{
/*
This function is called when the callback function is notified of
available data.and receives the data.
*/
//Term[x] is the program window (each for radioport)
static short int size;//be carefull here for WIN95 an INT variable is 32 bit long
											//we need only 16 bit so Short INT
long Result;
		if( hData != NULL )
		{
	Result=DdeGetData( hData,(LPSTR) &szData2, sizeof( szData2 ), 0 );
	if (Result<1) return;
	szData2[800]=NULL; //we copy the data to another string
	memmove(szData,szData2,Result+1);
	memmove(&size,szData+2,2); //take the data size (only the valaible data)
	int port=szData[0];        //take the port number starting from 0 for first port
	if (Term[0]==NULL) return;//if we dont have a valid window
	switch(szData[1])         //check the kind of data we have
	{
				case 'D'://DATA from AX25 CONNECT Channel
				//do here anything you wish
				 break;
	case 'U'://MONITOR DATA UNPROTO
				 if ((MonitorKind & MU)==MU) //MonitorKind is a variable holds the kind of
																			//monitor frames we wish to monitor
							 {
				 Term[port]->InsertTerm(szData+4,size);
				 }
				 break;
	case 'T'://TXDATA MONITOR our data
				 if ((MonitorKind & MTX)==MTX)
				 Term[port]->InsertTerm(szData+4,size);
			 break;
	case 'S'://MONITOR HEADER
				 if ((MonitorKind & MS)==MS)
			 Term[port]->InsertTerm(szData+4,size);
				 break;
	case 'I'://MONITOR  HEADER+DATA CONNECT OTHER STATIONS
				 if ((MonitorKind & MI)==MI)
			 Term[port]->InsertTerm(szData+4,size);
				 break;
	case 'C'://CONNECT DISCONNECT messages  //status message (*** Connect)
			 Term[port]->InsertTerm(szData+4,size);
		 Connected(szData+4);
				 break;
	case 'M'://MAXFRAME (How many packets in tx que)
		 MaxFrame=size;
		 break;
	case 'H'://MHeardList
//	      Term->InsertTerm(szData+2,15);
				if (szData[2]==' ') break;
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
			 Term[port]->InsertTerm(temp,strlen(temp));
		 break;
	default:
				 Term[0]->InsertTerm("ERROR RECEIVED\r",15);
				}//END SWITCH

		}
}
/*
DATA FORMAT General form
			1                    2                3                       4
[Port number (BYTE)][Data Kind (BYTE)][Size of data(short int)][DATA length Size]
1..Port nuber in ASCII format 0,1,2,3,4,5.....
2..Data kind in ASCII format
a)'D'=Data apo connect channel.Information transfer
	The DATA portion (4) is as follow.
			1              2             3
	[OriginCALL ][DestinationCall][ACTUALL DATA]
	1,2. Each call is 11 bytes long Null terminated (10 bytes data 11 byte NULL). eg. SV2AGW/0
	3.The data. The length size is in 3 field from general form
b) monitor data
	'U' Unproto data
	'T' our tx data
	'S' Only monitor Header (RR,REJ,RNR etc)
	'I' Monitor Header+ data from inof transfer from other stations
	The DATA portion (4) is As follow
	[DATA ] all the info in ascii format. The size is from field 3
c)Connect Disconnect status messages for us
	'C'.
	[*** Disconnected From Station [station call]]
	 [*** DISCONNECTED RETRYOUT With [staion call]]
	 [*** CONNECTED With Station [station call]] When we ask to connect with a staion
	 [*** CONNECTED To Station [station call]] Someone connected to us

d) MAxframe info for a port
	'M'
	We ask for it and we get in the size (3) of the general form

e) MHEARD LIST FOR a port
	 1                 2                            3
	[Call][First time heard (sizeof long)][Last time Heard(sizeof long)]
	1.Call null terminated
	2,3 time as long in UNIX format.
*/
//**********************************
void FWDWIN::Connected(char *str)
{
//*** Connected With SV2DXC-1
char *token;
char bbcall[15];
if (strstr(str,"*** DISC"))
{
//ConnDisStatus=DISCONNECT;drawStatusLine(HWindow);
token=strtok(str," "); //***
token=strtok(NULL," ");//DISConnecetd
token=strtok(NULL," ");//From
token=strtok(NULL," ");//station
token=strtok(NULL," \r");//SV2BBO klp
if (strcmp(token,bbcall)!=0) return;
if (token==NULL) return;
ConnBit->SelectImage(DISCONNECT,true);
if (bbs_status!=IDLE)
   if (strstr(str,"RETRYOUT")) bbs_rec((unsigned char *)str,(ushort)5,RETRYOUT);
      else bbs_rec((unsigned char *)str,(ushort)5,DISCONNECT);
return;
}

//it is a CONNECT from a station disconnect him. We are just a BBS FWD program 
if (strstr(str,"To"))
{
char *token;
token=strtok(str," "); //***
token=strtok(NULL," ");//Connecetd
token=strtok(NULL," ");//to
token=strtok(NULL," ");//station
token=strtok(NULL," \r");//SV2BBO klp
if (token==NULL) return;
SendPacket(token,"No Terminal Service!\r",21,'D');
//WaitSilent(2);
DisConnect(token);
return;
}

//ConnDisStatus=CONNECT;drawStatusLine(HWindow);
//(answer to a previous connect request from us)
ConnBit->SelectImage(CONNECT,true);
//inform the program for the connect
bbs_rec((unsigned char *)str,(ushort)5,CONNECTFROM);
}

//**********************************************************
//**** DATA TX to PACKET ENGINE                        *****
//**********************************************************
/*
GENERAL FORM
    1          2        3          4                  5                     6
[DATA KIND][Port Num][Call From][Call to][size of data (short int 16bit)][DATA]

1.DATA kind Byte long (char)
a)'D' data for an AX25 Channel
b)'M' Unproto Data
c)'S' PACKET COMMAND CONNECT,DISCONNECT,MAXFRAME,HEARD?
d)'K' pure KISS frame for special applications.No need for conversions TFESC etc
		just put the data the conversion will be done from PE. The frame
      must not be included between FEND.
e)'X' DDEML SECOND CONNECTION FOR DATA TRANSFERS ONLY.We register our MYCALL for
      this application.

2.Char port number to take action starting form 0.the format is as follow
     "char Port=1;" or "char Port=7" etc
3. Call from MYcall Null terminated
4.Call to Null Terminated
5.The DATA size short int
6.The data
*/

void FWDWIN::DisConnect(char *sstr)
{
static char szTemp[50];
char str[2];
str[0]='D';str[1]=NULL;
short count=strlen(str);
		 szTemp[1]=Port;szTemp[0]='S';
		 memmove(szTemp+2,MyCall,strlen(MyCall)+1);
		 if (sstr)  memmove(szTemp+12,sstr,strlen(sstr)+1);
		 else
       if (UNPROTOVIA) memmove(szTemp+12,DigiCall,strlen(DigiCall)+1); else memmove(szTemp+12,BBSCall,strlen(BBSCall)+1);
       memmove(szTemp+22,&count,2);
       memmove(szTemp+24,str,count);
TXDATA(szTemp,count+24);

}
void FWDWIN::AskMaxFrame(void)
{
static char szTemp[50];
char str[2];
str[0]='S';str[1]=NULL;
short count=strlen(str);
       szTemp[1]=Port;szTemp[0]='S';
       memmove(szTemp+2,MyCall,strlen(MyCall)+1);
       memmove(szTemp+12,BBSCall,strlen(BBSCall)+1);
       memmove(szTemp+22,&count,2);
       memmove(szTemp+24,str,count);
TXDATA(szTemp,count+24);
}

void FWDWIN::CMMHeard()
{

//BALE NA DIALEGEIS PORTA GIA MH
int x=0;
if (!MultiMonitor)
{
if (Ports.Num>1)
  {
  x=FINDPORT(this,"FindPort",&Ports,Port).Execute();
  if (x==1000) return;
  }
} else if (Ports.Num>1)
  {
 TMDIChild* curChild;
 curChild = GetActiveMDIChild();
 TTerm* cur ;
 TWindow *Clientw;
 Clientw=curChild->GetClientWindow();
 cur = TYPESAFE_DOWNCAST(Clientw, TTerm);
  for (x=0;x<Ports.Num;x++) if (cur==Term[x]) break;
	if (x==Ports.Num) return;
	 }
static char szTemp[50];
char str[12];
str[0]='H';str[1]=NULL;
short int count=strlen(str);
       szTemp[1]=x;szTemp[0]='S';
       memmove(szTemp+2,MyCall,strlen(MyCall)+1);
		 memmove(szTemp+12,BBSCall,strlen(BBSCall)+1);
		 memmove(szTemp+22,&count,2);
		 memmove(szTemp+24,str,2);                   
TXDATA(szTemp,count+24);
}

void FWDWIN::SendPacket(char *ToCall,char *str,int count,int DataKind)
{
short Count=count;
static char szTemp[300];
if (DataKind=='K')//via
{
//    struct stframe *frame;
//	 char hdr[HDRSIZE];
//	 frame = frmalloc(HDRSIZE +len);
//	 hdr[0] = SerialPort[port]->KissId;
szTemp[0]='K' ;
szTemp[1]=Port;;
    memmove(szTemp+2,MyCall,strlen(MyCall)+1);
    memmove(szTemp+12,BBSCall,strlen(BBSCall)+1);
	 char test[10];
    szTemp[25]=NULL;
    ca2ad(BBSCall,test); memmove(szTemp+25,test,7);
	 ca2ad(MyCall,test);memmove(szTemp+32,test,7);
    ca2ad(ViaCall,test);memmove(szTemp+39,test,7);
	 szTemp[31]  &= 0x1f;		//* C,RR not used
	 szTemp[38] &= 0x1f;
    szTemp[45] &= 0x1f;
	 szTemp[45] |= 0x01;
	 szTemp[46] = 0x03;		// UI
	 szTemp[47] = 0xf0;		// PID = BB
    memmove(szTemp+48,str,count);
    Count+=24;
    memmove(szTemp+22,&Count,2);
    TXDATA(szTemp,count+47);
return;
}
       szTemp[1]=Port;szTemp[0]=DataKind;
       memmove(szTemp+2,MyCall,strlen(MyCall)+1);
       if (ToCall) memmove(szTemp+12,ToCall,strlen(ToCall)+1);
       else if (UNPROTOVIA) memmove(szTemp+12,DigiCall,strlen(DigiCall)+1);else memmove(szTemp+12,BBSCall,strlen(BBSCall)+1);
       memmove(szTemp+22,&Count,2);
		 memmove(szTemp+24,str,count+1);
TXDATA(szTemp,count+24);
}

//***********************************************************
void FWDWIN::TXDATA(char *str,int count)
{
if (hConv[0]!=0)
    for (int x=0;x<3;x++)
	 if (DdeClientTransaction( (LPBYTE)str,count+1,hConv[0],hszItem[1],CF_TEXT,XTYP_POKE,1000,NULL )) return;
    else continue;
}

//*********************
//******************************************************************
void FWDWIN::beacon(void)
{
char test[256];
char ss[2];char sss[3];
ss[0]=13;ss[1]=NULL;sss[0]=13;sss[1]=10;sss[2]=NULL;
beaconTime+=beaconTimeEvery;
HFILE i;
i=_lopen(BeaconFile,OF_READ);
if (i==HFILE_ERROR) return;
for(;;)
{
long x=_hread(i,test,255);
if (x<=0) break;
test[x]=NULL;
lstrschg(test,sss,ss);
SendPacket("BEACON",test,strlen(test),'M');
}
_lclose(i);
}
//*************************************
void FWDWIN::Connect(void)
{
static char szTemp[50];
char str[2];
str[0]='C';str[1]=NULL;
short count=strlen(str);
			 szTemp[1]=Port;szTemp[0]='S';
			 memmove(szTemp+2,MyCall,strlen(MyCall)+1);
if (!UNPROTOVIA)  memmove(szTemp+12,BBSCall,strlen(BBSCall)+1);
			  else memmove(szTemp+12,DigiCall,strlen(DigiCall)+1);
       memmove(szTemp+22,&count,2);
       memmove(szTemp+24,str,count+1);
TXDATA(szTemp,count+24);
}
//***********************************************************
/*
 Don't hesitate to ask for any other info.


@COPYRIGHT 1990-1997
(SV2AGW)ROSSOPOULOS GEORGE

SV2AGW@SV2DXC.TSL.GRC.EU
sv2agw@the.forthnet.gr

*/
