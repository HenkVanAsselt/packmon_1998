#include "pm_glob.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <checks.h>                 
#include "buffer.h"

#define FALSE 0
#define TRUE 1

//////////////////////////////////////////////////////////////////////
// Buffer constructor code
//////////////////////////////////////////////////////////////////////
pmBuff::pmBuff(int aBufSize = 2000)
{
	Buffer=new char[aBufSize+2];
	if (!Buffer) {             // If no Buffer created, try a smaller buffer.
		aBufSize=1000+2;
		Buffer=new char[aBufSize+2];
	}
	BufferSize=aBufSize;
 	QueFront=0;    	// Set index of of front pointer
  QueBack=0;			// Set index of back pointer
 	FlagFullBuffer=FALSE;		// Indicate buffer is not full
}

//////////////////////////////////////////////////////////////////////
// reset()
// reset pointers to default
//////////////////////////////////////////////////////////////////////
void pmBuff::reset()
{
 	QueFront=0;    	// Set index of of front pointer
  QueBack=0;			// Set index of back pointer
 	FlagFullBuffer=FALSE;		// Indicate buffer is not full
}

//////////////////////////////////////////////////////////////////////
// insert()
//////////////////////////////////////////////////////////////////////
void pmBuff::insert(char *s, int count)
{
	unsigned int i;

  // Check if string stills fits in the buffer.
	if( QueFront + count >= BufferSize ) {	// It does not fit in buffer
		i = BufferSize - QueFront;  					// Calculate buffer space left
		memmove( &Buffer[QueFront], s, i );		// Fill buffer with first part
		memmove( Buffer, &s[i], count - i );  // FIll buffer with remainder
		QueFront = count - i;									// Set new Quefront
		QueBack=QueFront+1;										// Set queback.
		if (QueBack>=BufferSize)							// If past end of buffer
			QueBack=0;													// Set pointer to correct position.
		FlagFullBuffer=TRUE;    							// Buffer is FULL. Set flag
	}
	else {																			// It fit's.
		memmove( &Buffer[QueFront], s, count );		// Move data in buffer
		QueFront += count;												// Adjust front pointer
		if (FlagFullBuffer)												// If buffer is full
      QueBack=QueFront+1;											// Set back pointer just before the front pointer
	}

}

//////////////////////////////////////////////////////////////////////
//  input parameter start:    1 = get first line from buffer
//                            0 = get next line from buffer
//                           -1 = return a blank line
//////////////////////////////////////////////////////////////////////
char *pmBuff::GetLine(int start)
{
  static char s[300];
  static int x;
  int c;
  int y = 0;

  if (start == -1) {			// Return a blank line
 	  s[0] = '\0';
  	strsetsz(s,250);
  	return(s);
  }

  if (start==1) {      // If first line, set x
    x = QueBack;
	}

  s[y] = '\0';					// Initialization of the string.
  while (1) {
    if (x == QueFront)
      break;
	  if (x>BufferSize)		// Wrap around if needed
  	  x = 0;

    c = Buffer[x++];	// Get character from circular buffer
    if (c == '\0') {		// Check if CR found
      s[y++] = ' ';
      break;
    }
    if (c == 13) {		// Check if CR found
      s[y++] = ' ';
      break;
    }
    if (c == 10) {		// Check if LF found
      s[y++] = ' ';
      break;
    }
    if (y >= 252)			// Check if we are not at the end of our string
      break;
    s[y++] = c;
  }

  s[y] = '\0';
  if (strlen(s) == 0)
    return(NULL);

  strsetsz(s,250);
  return(s);

}


//////////////////////////////////////////////////////////////////////
// pmBuff CountLines
//////////////////////////////////////////////////////////////////////
long pmBuff::CountLines(void)
{
	static unsigned int x;
	long lines=0L;

	for(x=QueBack ; x!=QueFront ; x++) {
		if (x<=BufferSize) {
      if (Buffer[x]==13 || Buffer[x]==10)
        lines++;
    }
		else {
			x=0;
			if (x==QueFront)
        break;
			if (Buffer[x]==13 || Buffer[x]==10)
        lines++;
		}
	}//end for
	return(lines);
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void pmBuff::strsetsz(char *s, int m)
{
	int len;
	int i;

	len = strlen(s);
	for (i=len ; i<m ; i++)
		s[i] = ' ';
	s[i] = '\0';
}


