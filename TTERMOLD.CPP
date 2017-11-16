#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <owl\owlpch.h>
#include <owl\applicat.h>
#include <owl\framewin.h>

#include <owl\window.h>
#include <owl\dc.h>
#include <owl\scroller.h>

#include <windowsx.h>

#define GCL_STYLE (-26)

#include <ddeml.h>
#include "ddecli.rh"
#include "packmon.h"
#include "ttermold.h"

// Other prototypes
char *strsetsz(char *str,int newsize);

DEFINE_RESPONSE_TABLE2(TTerm, PACKMONWnd, TWindow)
	EV_MESSAGE(WM_SETFONT, WMFONTS),
	EV_WM_PAINT,
END_RESPONSE_TABLE;

//*************************************************************************
// Constructor Defintion
//*************************************************************************
TTerm::TTerm(TWindow* AParent1,UINT aBufSize,LPSTR ATITLE,bool CanIClose1):
				PACKMONWnd(AParent1,ATITLE)
{
	// Initializations
	TextHeight=0;

	// Check if size does not exceed the maximum
	if (aBufSize>30000) 
		aBufSize=30000;
	Buffer=new char[aBufSize+2];

	// If no Buffer created, try a smaller buffer.
	if (!Buffer) {
		aBufSize=3000;
		Buffer=new char[aBufSize+2];
	}

	// Initialize variables.
	QueFront=QueBack=0;
	BufferSize=aBufSize;
	Attr.Style |= (WS_VSCROLL | WS_HSCROLL | WS_VISIBLE);
	FlagFullBuffer=false;
	CanIClose=CanIClose1;
	TTerm::SetupWindow();
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void TTerm::SetupWindow()
{
	TWindow::SetupWindow();
	Scroller = new TScroller(this,8,15,80,60);
	SetScrollDim(0);
	Scroller->SetRange(260L,0L);
	Scroller->ScrollTo(0L,0L);
	if (CanIClose) {
		long ClassStyle=GetClassLong(GCL_STYLE);
		ClassStyle|=CS_NOCLOSE;
		SetClassLong(GCL_STYLE,ClassStyle);
	}
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void TTerm::CloseWindow(int RetVal)
{
	if (Buffer) 
		delete(Buffer);
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void TTerm::SetScrollDim(UINT TotalHeight)
{
	HDC hdc=GetDC(HWindow);
	SelectObject(hdc,hfont);   
	SIZE SizeChar;
	LPSTR lpszJustified = "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW";
	// Defined in include\wingdi.h: 
//	GetTextExtentPoint32(hdc, lpszJustified, lstrlen(lpszJustified),&SizeChar);
	GetTextExtentPoint(hdc, lpszJustified, lstrlen(lpszJustified),&SizeChar);
	TextHeight=SizeChar.cy;
	Scroller->SetUnits(SizeChar.cx/lstrlen(lpszJustified),TextHeight);
	ReleaseDC(HWindow,hdc);
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
LRESULT  TTerm::WMFONTS(WPARAM wp,LPARAM)
{
	hfont=(HFONT)wp;
	SetScrollDim(0);
	Scroller->SetPageSize();
	Invalidate(true);
	//InvalidateRect(Scroller->HWindow, NULL, TRUE);
	return (LRESULT)1;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void
TTerm::EvPaint()
{
	TPaintDC paintDC(HWindow);
	HDC hdc=GetDC(HWindow);
	TRect rect;

	GetClientRect(rect);
	Paint(hdc,0,rect);
}



//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void TTerm::Paint(TDC& dc, bool erase, TRect& rect)
{
	char s[300];

	COLORREF Tcol = GetSysColor(COLOR_WINDOWTEXT);
	SetBkColor(dc,GetSysColor(COLOR_WINDOW));
	SelectObject(dc,hfont);
	UINT x;
	int y=0;
	long z=0L;

	//rect.top=0;rect.bottom=0;rect.left=0;rect.right=0;CopyVerScr=0;CopyHorScr=0;

	for(x=QueBack ; x!=QueFront ; x++,y++) {
		if (x<=BufferSize) {
			s[y]=Buffer[x];
			if ((s[y]==13) || (y>=252)) {
				if (Scroller->YPos>z) {
					z++; 
					y=-1; 
					continue;
				}
				s[y]=NULL;
				y=-1;
				// What does this: strsetsz(s,250);
				if (s[3]=='F' && s[4]=='m') 
					SetTextColor(dc,RGB(0,0,255));
				else 
					SetTextColor(dc,Tcol);
				//	 TabbedTextOut(PaintDC,0,z*TextHeight,s,strlen(s),0,0,0);
				TextOut(dc,0,z*TextHeight,s,strlen((char *)s));
				z++;
			}
			if (s[y]==10) 
				y--;
		}//end if queback+x
		else {
			// x=x-BufferSize;
			x=0;
			s[y]=Buffer[x];
			if (x==QueFront) break;
			if ((s[y]==13) ||(y>=252)) {
				if (Scroller->YPos>z) {z++;y=-1;continue;}
				s[y]=NULL;
				y=-1;
				// What does this: strsetsz(s,250);
				if (s[3]=='F' && s[4]=='m') SetTextColor(dc,RGB(0,0,255));
				else SetTextColor(dc,Tcol);
				//	 TabbedTextOut(PaintDC,0,z*TextHeight,s,lstrlen(s),0,0,0);
				TextOut(dc,0,z*TextHeight,s,strlen((char *)s));
				z++;
			}
		}
	}//end fornext

	if (y!=-1) {
		s[y]=NULL;
		y=-1;
		// What does this: strsetsz(s,250);
		if (s[0]=='F' && s[1]=='m') 
			SetTextColor(dc,RGB(0,0,255));
		else 
			SetTextColor(dc,Tcol);
		TextOut(dc,0,z*TextHeight,s,strlen((char *)s));
		z++;
	}

	for (x=0;x<2;x++)           //ADD 2 BLANK LINES
	{
		s[0]=NULL;
		// What does this: strsetsz(s,250);
		SetTextColor(dc,Tcol);
		TextOut(dc,0,z*TextHeight,s,strlen((char *)s));
	}
	z++;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void TTerm::InsertTerm(unsigned char *s,int count)
{
	UINT i;
	//printf("INSERT TERM=%s\n",s);
	//count=lstrlen(s);
	if( QueFront + count >= BufferSize ) {
		i = BufferSize - QueFront;
		memmove( &Buffer[QueFront], s, i );
		memmove( Buffer, &s[i], count - i );
		QueFront = count - i;
		QueBack=QueFront+1;
		if (QueBack>=BufferSize) 
			QueBack=0;
		FlagFullBuffer=TRUE;
	}
	else {
		memmove( &Buffer[QueFront], s, count );
		QueFront += count;
		if (FlagFullBuffer) QueBack=QueFront+1;
		//	if (QueBack>=BufferSize) QueBack=0;
	}

	long xx=CountLines();
	if (xx>Scroller->YRange) Scroller->SetRange(Scroller->XRange,xx);
	Scroller->ScrollTo(0L,xx-Scroller->YPage);
//	Invalidate(false);
	Invalidate(true);
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
long TTerm::CountLines(void)
{
	static UINT x;
	long lines=0L;
	for(x=QueBack;x!=QueFront;x++) {
		if (x<=BufferSize)  {if (Buffer[x]==13) lines++; }
		else {
			x=0;
			if (x==QueFront) break;
			if (Buffer[x]==13) lines++;
		}
	}//end for
	return(lines);
}

