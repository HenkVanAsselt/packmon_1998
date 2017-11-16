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

extern pmBuff Buffer;

//////////////////////////////////////////////////////////////////////
// EvPaint()
//////////////////////////////////////////////////////////////////////
void
PACKMONWnd::EvPaint()
{
	TPaintDC paintDC(HWindow);
	TRect rect;
	HDC hdc=GetDC(HWindow);

 // Establish fixed font in display context.
 SelectObject(hdc, hnewsfont);


	GetClientRect(rect);
	pmWinPaint(hdc,0,rect);
	ReleaseDC(HWindow,hdc);
}

//////////////////////////////////////////////////////////////////////
// pmWinPaint()
//////////////////////////////////////////////////////////////////////
void PACKMONWnd::pmWinPaint(TDC& dc, bool erase, TRect& rect)
{
	char *s;

	COLORREF Tcol = GetSysColor(COLOR_WINDOWTEXT);
	SetBkColor(dc,GetSysColor(COLOR_WINDOW));

	SelectObject(dc,hfont);

	UINT x;
	int y=0;
	long linenr=0L;
  int i = 0;

	//rect.top=0;rect.bottom=0;rect.left=0;rect.right=0;CopyVerScr=0;CopyHorScr=0;

  s = Buffer.GetLine(1);
  while (s) {
    if (linenr >= Scroller->YPos) {
			if (s[3]=='F' && s[4]=='m')
				SetTextColor(dc,RGB(0,0,255));
			else
				SetTextColor(dc,Tcol);
//			TextOut(dc,0,(int)linenr*TextHeight,s,strlen((char *)s));
			TextOut(dc,0,(int)i*TextHeight,s,strlen((char *)s));
      i++;
    }
		linenr++;
    s = Buffer.GetLine(0);
	}


  // Add 2 blank lines
  for (x=0 ; x<5 ; x++) {
    s = Buffer.GetLine(-1);
		if (s[3]=='F' && s[4]=='m')
 			SetTextColor(dc,RGB(0,0,255));
 		else
 			SetTextColor(dc,Tcol);
 		TextOut(dc,0,(int)i*TextHeight,s,strlen((char *)s));
    i++;
	}
}


//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void PACKMONWnd::SetScrollDim()
{
	HDC hdc=GetDC(HWindow);

  // Build fixed screen font. Needed to display formated output.
  cursfont.lfHeight         =  6;              	// Height of text
  cursfont.lfWidth          =  4;								// Average width of characters
  cursfont.lfEscapement     =  0;								// Angle (for italics, etc.)
  cursfont.lfOrientation    =  0;								// currently not used
  cursfont.lfWeight         =  FW_NORMAL;				// light, normal, bold etc.
  cursfont.lfItalic         =  FALSE;						// italic text
  cursfont.lfUnderline      =  FALSE;						// underline text
  cursfont.lfStrikeOut      =  FALSE;						// strikeout (line through text)
  cursfont.lfCharSet        =  ANSI_CHARSET;		// ANSI, OEM, etc
  cursfont.lfOutPrecision   =  OUT_DEFAULT_PRECIS;	// font slection rules
  cursfont.lfClipPrecision  =  CLIP_DEFAULT_PRECIS;	// clipping rules
  cursfont.lfQuality        =  DEFAULT_QUALITY;			// quality rules
  cursfont.lfPitchAndFamily =  FIXED_PITCH | FF_DONTCARE;	// pitch, font family (script, etc.)
  strcpy((char *)cursfont.lfFaceName, "courier");					// Font name (Times New Roman, etc).
  hnewsfont = CreateFontIndirect((LPLOGFONT) &cursfont);
  // Install the font in the current display context.
  SelectObject(hdc, hnewsfont);

	SIZE SizeChar;
	LPSTR lpszJustified = "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW";

	GetTextExtentPoint(hdc, lpszJustified, lstrlen(lpszJustified),&SizeChar);
	TextHeight=SizeChar.cy;
	Scroller->SetUnits(SizeChar.cx/lstrlen(lpszJustified),TextHeight);
	ReleaseDC(HWindow,hdc);
}



