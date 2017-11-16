//----------------------------------------------------------------------------
// FILE: PM_NODE.CPP
// AUTH: Henk van Asselt
// SUBJ: Node functions for Packet Monitor
// HIST: 980125 V0.1
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
#include <mmsystem.h>			// For sound play

#include "buffer.h"
#include "packmon.h"
#include "pm_node.h"

//////////////////////////////////////////////////////////////////////
// NODElist() Constructor
//////////////////////////////////////////////////////////////////////
NODElist::NODElist(int size)
{
  max_nodes = size;											// Remember the size of the table
  table = new _nodelistentry[size];	// Allocate memory for table
  no_nodes = 0;													// Set actual number of stations
}

//////////////////////////////////////////////////////////////////////
// FUNCTION: NODElist::find()
// PURPOSE:  find if a node is already in the list
// RETURNS:  -1 if node not found
//					 >= 0 index to node entry in the table.
//////////////////////////////////////////////////////////////////////
int NODElist::find(char *reporting_station, char *node,char *alias, char *via)
{
  int i;

  // Check if node information is not already in the list
  for (i=0 ; i<no_nodes ; i++) {
    if (!strcmp(table[i].station,reporting_station) &&
        !strcmp(table[i].node,node) &&
        !strcmp(table[i].alias,alias) &&
        !strcmp(table[i].via,via)) {
          return(i);
        }
  }
  return(-1);	// No match found
}


//////////////////////////////////////////////////////////////////////
// NODElist::add
// add information to the node list of a reporting station:
// node,alias,via,quality
// returns: total number of nodes in the list
//////////////////////////////////////////////////////////////////////
int NODElist::add(char *reporting_station, char *node,char *alias, char *via, int quality, time_t timestamp)
{
  int i;
  time_t currentTime;
  time_t diffTime;
  struct tm *age;

  // Check if still room in table
  if (no_nodes >= max_nodes)
    return(0);

  currentTime = time(NULL);

  // Check if node information is not already in the list
  // If so, only update the quality
  i = find(reporting_station,node,alias,via);
  if (i >= 0) {
    // update
    table[i].quality = quality;
    table[i].timestamp = timestamp;
  }

  // Calculate the age of the entry
  diffTime = currentTime-timestamp;
  age = localtime(&diffTime);
  // If the entry is already 4 day's old, don't add it to the table
  if (age->tm_mday > MAX_AGE)
    return(no_nodes);

  if (i < 0) {
    strcpy(table[no_nodes].station,reporting_station);
    strcpy(table[no_nodes].node,node);
    strcpy(table[no_nodes].alias,alias);
    strcpy(table[no_nodes].via,via);
    table[no_nodes].quality = quality;
    table[no_nodes].timestamp = timestamp;
    // Increment number of nodes in the list
    no_nodes++;
  }
  // Return number of nodes in the list
  return(no_nodes);
}

//////////////////////////////////////////////////////////////////////
// display()
// display node table.
//////////////////////////////////////////////////////////////////////
void NODElist::display()
{
  int i;
  char s[256];
  extern pmBuff Buffer;
  struct tm *tb;					// Structure timeblock
  time_t currtime;
  time_t difftime;
  struct tm *age;


  Buffer.reset();			// Reset buffer pointers as if it is empty

  sprintf(s,"## STATION    NODE       ALIAS      VIA        QUALITY    AGE\n\n");
  Buffer.insert(s,strlen(s));

  // Print station data of the table
	for (i=0 ; i<no_nodes ; i++) {
    currtime = time(NULL);
    difftime = currtime-table[i].timestamp;

    age = localtime(&difftime);
    if (age->tm_mday > MAX_AGE)
      break;
    else {
      // Print general information
      sprintf(s,"%2d %-10.10s %-10.10s %-10.10s %-10.10s %3d ",
        i+1,
        table[i].station,
        table[i].node,
        table[i].alias,
        table[i].via,
        table[i].quality);
      Buffer.insert(s,strlen(s));

      // Print age information
      age = localtime(&difftime);
      sprintf(s,"[%2d days %2d hours %2dmin] ",
        age->tm_mday,
        age->tm_hour,
        age->tm_min);
      Buffer.insert(s,strlen(s));

      // Calculate local time for timestamp
      tb = localtime(&table[i].timestamp);
      sprintf(s,"[%02d%02d%02d %02d:%02d:%02d]\n",
        tb->tm_year,
        tb->tm_mon+1,				// As month is returned as 0...11
        tb->tm_mday,
        tb->tm_hour,
        tb->tm_min,
        tb->tm_sec
      );
      Buffer.insert(s,strlen(s));
    }
  }
}






