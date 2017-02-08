/***************************************************************************
* fifo.cpp  -  code to allow interprocess communication via a fifo, or "names pipe"
 *
* copyright : (C) 2009 by Jim Skon
*
* This code permits the creation and use of FIFOs for communication
* between processes.  
* 
* the named piped is created and used in /tmp
*
***************************************************************************/
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <iostream>
#include <fstream>

using namespace std;

#define MaxMess 100

#define PATH "/tmp/"

#define MODE 0666

/* The following is added to the name to make sure it is unique */
/* Returns 0 if failure, 1 if success */
char * signature = "SkonNames_";

// create a named pipe (FIFO)
int createfifo(char *pipename) {

  // build the name string
  char name[100] = PATH;
  strcat(name,signature);
  strcat(name,pipename); 

  umask(0);
  // Create (or open) the fifo
  int result = mkfifo(name,MODE);

  if ((result == -1) && (errno != EEXIST)) {
	cout << "Error creating pipe: " << name << endl;
	return (0);
  }

  return(1);

}

// Receive a message from a FIFO (named pipe)
// Assumes each message ends with a "#"
string recv(char *pipename) {

  int length, i;
  char message[MaxMess];
  bool done;

  // clear message buffer
  memset(message,0,100);

  // build the pipe name string
  char name[100] = PATH;
  strcat(name,signature);
  strcat(name,pipename); 

  // Open the pipe
  ifstream pipe_in(name);

  // Check if open succeeded
  if (!pipe_in.good()) {
	cout << "Error - bad input pipe: " << name << endl;
	return("");
  }
  
  // read until we see an end of message character '#';
  done = false;
  for (i=0 ; i<MaxMess && !done; i++) {
	pipe_in.read(&message[i],1);
	done = message[i] == '#';
  }
  // get rid of message terminator
  message[i-1] = 0;

  return(message);
}

// Send a message to a FIFO (named pipe)
// Return 0 if fails, 1 if succeeds
int send(string& message, char * pipename) {

  // Append end of message terminator
  message = message + "#";

  // build the name string
  char name[100] = PATH;
  strcat(name,signature);
  strcat(name,pipename); 

  ofstream pipe_out(name);

  // Check if open succeeded
  if (!pipe_out.good()) {
	cout << "Error - bad output pipe: " << name << endl;
	return (0);
  }

  pipe_out << message;
  return 1;
}

