
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


//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
#ifdef xxx
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
z=DdeGetLastError(InstId());
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
#endif

#ifdef xxx
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
#endif

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

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
#ifdef xxx
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
#endif


//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
#ifdef xxx
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
#endif

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
#ifdef xxx
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
#endif

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
#ifdef xxx
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
#endif


//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
#ifdef xxx
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
#endif

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
#ifdef xxx
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
#endif

