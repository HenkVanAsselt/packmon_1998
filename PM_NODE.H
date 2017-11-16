//////////////////////////////////////////////////////////////////////////////
// FILE: PM_NODE.H
// HIST: 980614 V0.1 - Derived from PM_HEARD.H
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _PM_NODE_H
#define _PM_NODE_H

#define MAXNODES 50
#define MAX_AGE  5

typedef struct _nodelistentry  {
  char   station[15];
  char   node[15];
  char   alias[15];
  char   via[15];
  int    quality;
  time_t timestamp;			// Time of entry
};

class NODElist {
  public:
    // functions
		NODElist(int size = MAXNODES);    // constructor
    int analyze(char *instr,time_t timestamp);
    int add(char *call, char *node, char *alias, char *via, int quality,time_t timestamp);
    int find(char *reporting_station, char *node,char *alias, char *via);
    void display(void);
    // variables
    int no_nodes;			// number of nodes in the list
    int max_nodes;
    _nodelistentry *table;
};

#endif		// _PM_NODE_H
