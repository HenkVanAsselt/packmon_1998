#include <stdio.h>
#include <time.h>
#include <memory.h>
#include "pm_log.h"

//////////////////////////////////////////////////////////////////
// FUNCTION: PM_LOG()
// Purpose:  Constructor of the PM_LOG Class
// RETURNS:
// HISTORY:  980605: Created this function.
//////////////////////////////////////////////////////////////////
PM_LOG::PM_LOG()
{
  // Initializations
  filename[0] = '\0';
  logfile = NULL;
}

//////////////////////////////////////////////////////////////////
// FUNCTION: CloseLogFile()
// Purpose:  Close open logfile
// RETURNS:  NULL Pointer
// HISTORY:  980605: Created this function.
//////////////////////////////////////////////////////////////////
FILE *
PM_LOG::CloseLogFile()
{
	if (logfile) {
    fclose(logfile);
    logfile = NULL;
	}
  return(NULL);
}

//////////////////////////////////////////////////////////////////
// FUNCTION: OpenLogFile()
// Purpose:  Open logfile by name.
//					 Filename will be saved in class, together with the
//					 pointer to the logfile.
// RETURNS:  Pointer to file opened.
// HISTORY:  980605: Created this function.
//////////////////////////////////////////////////////////////////
FILE *
PM_LOG::OpenLogFile(char *s)
{
	if (!logfile) {
		logfile = fopen(s,"r+b");
	}
  return(logfile);
}


//////////////////////////////////////////////////////////////////
// FUNCTION: ReadLogData()
// Purpose:  Read line from logfile
// Returns:  pointer to string of data read in case of success
//					 NULL pointer in case of failure.
// HISTORY:  980530 V0.2 - Changed way file header is read.
//					 980605 V0.3 - Moved this function to PM_LOG.C and
//												 created header file PM_LOG.H
//////////////////////////////////////////////////////////////////
char *PM_LOG::ReadLogData()
{
  static char s[MAXLOGDATASIZE]; 	// String to contain store logdata in.
  static long fp;								// Pointer to start of header
  unsigned char version;        // Header revision
  unsigned int  headerlen;			// Length of this header
  unsigned int  datalen;				// Length of data after this header

  // check if logfile is opened
  if (!logfile)
    return(NULL);

   // If end of input, close file and return
   if (feof(logfile)) {
     CloseLogFile();
     return(NULL);
   }

   memset(s,'\0',MAXLOGDATASIZE);		// Initialize string

   // Read header, then read the data
   // No special case for differnt types of headers build in yet.
   fp = ftell(logfile);		// Get current position
   fread(&version,sizeof(unsigned char),1, logfile);
   fread(&headerlen,sizeof(unsigned int),1, logfile);
   fread(&datalen,sizeof(unsigned int),1, logfile);
   // Go back to first start of header and read complete header now
   fseek(logfile,fp,SEEK_SET);
   fread(&logheader,headerlen,1,logfile);
   // Now read the data part
   fread(s,datalen,1,logfile);
   return(s);
}

///////////////////////////////////////////////////////////////////
// FUNCTION: 	LogData
// PURPOSE:  	Log data in a log file.
// 						This function opens the log file, write the header
//					  then writes the data part and closes the file again.
// HISTORY:		970605 V0.2 - Moved this function to 'pm_log.c'
///////////////////////////////////////////////////////////////////
void PM_LOG::WriteLogData(char *dataptr, int size)
{
  FILE *f_out = NULL;

   // Open the log file
   if (!f_out) {
     f_out = fopen("PM.LOG","a+b");
   }

   // If logfile could not be opened, return
   if (!f_out)
     return;

   // Fill in header of the log record
   logheader.version = 2;							// Version 2
   logheader.headerlen = sizeof(LOGHEADER); // version 2)
   logheader.datalen = size;				// Version 1
   logheader.timestamp = time(NULL);			// Save current time (version: 2)

   // Write log header and data to the log file
   fwrite(&logheader,sizeof(LOGHEADER),1,f_out);
   fwrite(dataptr,sizeof(char),size,f_out);

   // Close the logfile
   CloseLogFile();
}

//////////////////////////////////////////////////////////////////
// FUNCTION: TimeStamp()
// Purpose:  Return timestamp of header
// Returns:  Timestamp of the header
// HISTORY:  980614 V0.1 - Initial version
//////////////////////////////////////////////////////////////////
time_t PM_LOG::TimeStamp()
{
  return(logheader.timestamp);
}


