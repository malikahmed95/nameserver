#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "fifo.h"

using namespace std;
string receive_fifo = "Namereply";
string send_fifo = "Namerequest";

int main() {
  string message, name;
  string reply;

  while (1) {
	cout << "Enter a last name:";
	cin >>  name;
	transform(name.begin(), name.end(), name.begin(), ::toupper);

	
	// create the FIFOs for communication
	Fifo recfifo(receive_fifo);
	Fifo sendfifo(send_fifo);

	message = "$LAST*" + name;
	cout << "Send:" << message << endl;
	sendfifo.openwrite();
	sendfifo.send(message);

	/* Get a message from a server */
	recfifo.openread();
	reply = recfifo.recv();
	
	recfifo.fifoclose();
	sendfifo.fifoclose();
	cout << "Server sent: " << reply << endl;
  }


}
