#ifndef _PM_LOG_H			// Prevent multiple inclusions
#define _PM_LOG_H

#define MAXLOGDATASIZE 1024

typedef struct _LOGHEADER {
  // Following 3 entries may not be changed in size and/or order
  unsigned char version;        // Header revision (currently verion 2)
  unsigned int  headerlen;			// Length of this header (version 1)
  unsigned int  datalen;				// Length of data after this header (Version 1)
  // Other log items
  time_t timestamp;				   	  // time Structure (version 2)
}
LOGHEADER;

class PM_LOG{

  // Variables
	public:
  char filename[30];				// Filename
  FILE *logfile;						// Pointer to open file
  LOGHEADER logheader;			// Area for writing and saving logheader in

  // Functions
  public:
  PM_LOG(void);																		// Constructor
  FILE *OpenLogFile(char *s);       							// Open log file
  FILE *CloseLogFile(void);												// Close log file
	void WriteLogData(char *dataptr, int size);			// Write data to file
	char *ReadLogData(void); 												// Read data from file
  time_t TimeStamp(void);													// Return timestamp of header
};


#endif