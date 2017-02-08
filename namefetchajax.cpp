#include <iostream>
// Stuff for AJAX
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

//Stuff for pipes
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include "fifo.h"

using namespace std;
using namespace cgicc; // Needed for AJAX functions.

// Possible values returned by results_select
#define LAST  "Last"
#define MALE "Male"
#define FEMALE "Female"

//Indeed, myString needs to be a copy of the original string
std::string StringToUpper(std::string myString)
{
  const int length = myString.length();
  for(int i=0; i!=length ; ++i)
    {
      myString[i] = std::toupper(myString[i]);
    }
  return myString;
}

// fifo for communication
string receive_fifo = "Namereply";
string send_fifo = "Namerequest";

int main() {
  Cgicc cgi;    // Ajax object
  char *cstr;
  // Create AJAX objects to recieve information from web page.
  form_iterator ts = cgi.getElement("type_select");
  form_iterator name = cgi.getElement("name");

  // create the FIFOs for communication
  Fifo recfifo(receive_fifo);
  Fifo sendfifo(send_fifo);
  
  string type = "$LAST*";
  if (**ts == LAST) {
	type = "$LAST*";
  } else if (**ts == MALE) {
	type = "$MALE*";
  } else if (**ts == FEMALE) {
	type = "$FEMALE*";
  }

  // Call server to get results
  string stname = **name;
  stname = StringToUpper(stname);
  string message =  type+stname;
  sendfifo.openwrite();
  sendfifo.send(message);
  
  /* Get a message from a server */
  recfifo.openread();
  string results = recfifo.recv();
  recfifo.fifoclose();
  sendfifo.fifoclose();
  cout << "Content-Type: text/plain\n\n";

  cout << results;
  
return 0;
}
