/***************************************************************************
* nameservercpp  -  Program to serve of last name statistics
 *
* copyright : (C) 2009-2017 by Jim Skon
*
* This program runs as a background server to a CGI program, providinging US Census
* Data on the frequency of names in response to requestes.  The rational for this 
* is to demonstrait how a background process can serve up information to a web program,
* saving the web program from the work of intiallizing large data structures for every
* call.
*  
*
***************************************************************************/
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <map>
#include "fifo.h"

using namespace std;

/* Fifo names */
string receive_fifo = "Namerequest";
string send_fifo = "Namereply";

/* Name data structure */
struct name_record
{
  string name;         // Last name
  string percent;      // Frequency of occurance of a given name
  string cumulative;   // Cummulative freqency of all name up to and including this name
  string rank;         // Rank of this Name in terms of frequency
};

// Maps for holding the name data
map<string,name_record> lname_map;
map<string,name_record> fname_map;
map<string,name_record> mname_map;
map<string,name_record>::iterator it;

/*
 * Read the US Census data file "dist.all.last" and load it into an
 * C++ STL MAP with name as the key.
 *
 * Return 1 if success, 0 if fail
 */
int createnamemap(map<string,name_record> &name_map ,string filename) {
	
  name_record namedata;
  fstream infile(filename.c_str());
  if (infile.good()) {
	while (1)
	  {
		infile >> namedata.name;
		infile >> namedata.percent;
		infile >> namedata.cumulative;
		infile >> namedata.rank;
		if (infile.fail()) break;
		name_map[namedata.name] = namedata;
		//cout << namedata.name + " " <<  namedata.percent << " " 
		//	 << namedata.cumulative << " " << namedata.rank << endl;
	  }
	infile.close();
	return(1);
  } else {
	return(0);
  }
}


/* Server main line: create name MAPs, wait for and serve requests */
int main() {
  
  string inMessage, outMessage,name,percent,rank,type;
  name_record result;
  int pos;

  // Create the map of last name data
  if (createnamemap(lname_map,"dist.all.last") != 1) {
	cout << "Error Loading Database" << endl;
	exit(0);
  }
  // Create the map of male name data
  if (createnamemap(mname_map,"dist.male.first") != 1) {
	cout << "Error Loading Database" << endl;
	exit(0);
  }
  // Create the map of last name data
  if (createnamemap(fname_map,"dist.female.first") != 1) {
	cout << "Error Loading Database" << endl;
	exit(0);
  }
  
  cout << "Name data loaded!" << endl;

  // create the FIFOs for communication
  Fifo recfifo(receive_fifo);
  Fifo sendfifo(send_fifo);
  
  map<string,name_record> *curMap;
  while (1) {

    /* Get a message from a client */
    recfifo.openread();
    inMessage = recfifo.recv();
	/* Parse the incoming message */
	/* Form:  $type*name  */
	pos=inMessage.find_first_of("*");
	if (pos!=string::npos) {
	  type = inMessage.substr(0,pos);
	  pos++;
	} else {
	  type = "$LAST";
	  pos = 0;
	}
	name = inMessage.substr(pos,2000);
	cout << "Message: " << type << " : " << name << endl;

	// Set curMap to be the map requested
	// Set it to be an iterator for the appropriate map
	if (type == "$LAST") {
	  //Get the closest match
	  it = lname_map.lower_bound (name);
	  curMap = &lname_map;

	} else  if (type == "$MALE") {
	  //Get the closest match
	  it = mname_map.lower_bound (name);
	  curMap = &mname_map;

	} else  if (type == "$FEMALE") {
	  //Get the closest match
	  it = fname_map.lower_bound (name);
	  curMap = &fname_map;
	}

	// back up 5 places
	for (int i=0 ; i<5 && (it!=curMap->begin()); i++) {
	  it--;
	}

	outMessage = "";
	// Get 10 results
	for (int i=0 ; i<10 && (it!=curMap->end()); i++) {
	  result = (*it).second;
	  name = result.name;
	  percent = result.percent;
	  rank = result.rank;
	  if (i!=0) outMessage+=",";
	  outMessage += name + "," + percent + "," + rank;
	  it++;

	}
	
	cout << " Results: " << outMessage << endl;

	sendfifo.openwrite();
	sendfifo.send(outMessage);
	sendfifo.fifoclose();
	recfifo.fifoclose();

  }
}
