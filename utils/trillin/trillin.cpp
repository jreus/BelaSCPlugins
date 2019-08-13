
#include "Bela.h"
#include "Trill.h"
#include "Trill.h"

/* TODO: Needs a deconstructor?
void cleanup(BelaContext *context, void *userData)
{
	unit->sensor.cleanup();
}
*/

// See the Server Plugin API for more info
// http://doc.sccode.org/Reference/ServerPluginAPI.html
#include "SC_PlugIn.h"

// number of trill sensors in raw data
#define NUM_SENSORS 26

// These functions are provided by Xenomai
int rt_printf(const char *format, ...);
int rt_fprintf(FILE *stream, const char *format, ...);

// InterfaceTable contains pointers to functions in the host (scserver).
static InterfaceTable *ft;

// Holds UGen state variables
struct TrillIn : public Unit {
  Trill sensor;
  AuxiliaryTask i2cReadTask;
  unsigned int readInterval;
  unsigned int readIntervalSamples;
  unsigned int readCount;
};

static void TrillIn_Ctor(TrillIn* unit); // constructor
static void TrillIn_next_k(TrillIn* unit, int inNumSamples); // audio callback



/*
 ____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\

The platform for ultra-low latency audio and sensor processing

http://bela.io

A project of the Augmented Instruments Laboratory within the
Centre for Digital Music at Queen Mary University of London.
http://www.eecs.qmul.ac.uk/~andrewm

(c) 2016 Augmented Instruments Laboratory: Andrew McPherson,
  Astrid Bin, Liam Donovan, Christian Heinrichs, Robert Jack,
  Giulio Moro, Laurel Pardue, Victor Zappi. All rights reserved.

The Bela software is distributed under the GNU Lesser General Public License
(LGPL 3.0), available here: https://www.gnu.org/licenses/lgpl-3.0.txt
*/


#include <Bela.h>
#include <libraries/OSCSender/OSCSender.h>
#include <iostream>

OSCSender oscSender;
int remotePort = 57120;
const char* remoteIp = "127.0.0.1";

// parse messages received by OSC Server
// msg is Message class of oscpkt: http://gruntthepeon.free.fr/oscpkt/
bool handshakeReceived;

int main() {
  setup();
}

int main(int argc, char *argv[]){
   std::cout << "Hello OSC" << std::endl;
   return 0;
}

void on_receive(oscpkt::Message* msg)
{
	if(msg->match("/osc-setup-reply"))
		handshakeReceived = true;
	else if(msg->match("/osc-test")){
		int intArg;
		float floatArg;
		int count = msg->match("/osc-test").popInt32(intArg).popFloat(floatArg).isOkNoMoreArgs();
		printf("received a message with int %i and float %f\n", intArg, floatArg);
		oscSender.newMessage("/osc-acknowledge").add(count).add(4.2f).add(std::string("OSC message received")).send();
	}
}

bool setup()
{
	oscSender.setup(remotePort, remoteIp);

	// the following code sends an OSC message to address /osc-setup
	// then waits 1 second for a reply on /osc-setup-reply
	oscSender.newMessage("/osc-setup").send();
	int count = 0;
	int timeoutCount = 10;
	printf("Waiting for handshake ....\n");
	while(!handshakeReceived && ++count != timeoutCount)
	{
		usleep(100000);
	}
	if (handshakeReceived) {
		printf("handshake received!\n");
	} else {
		printf("timeout! : did you start the node server? `node /root/Bela/resources/osc/osc.js\n");
		return false;
	}
	return true;
}


/**
\example OSC/render.cpp

Open Sound Control
------------------

This example shows an implementation of OSC (Open Sound Control) which was
developed at UC Berkeley Center for New Music and Audio Technology (CNMAT).

It is designed to be run alongside resources/osc/osc.js.
For the example to work, run in a terminal on the board
```
node /root/Bela/resources/osc/osc.js
```

The OSC receiver port on which to receive is set in `setup()`
via `oscReceiver.setup()`. Likewise the OSC client port on which to
send is set in `oscSender.setup()`.

In `setup()` an OSC message to address `/osc-setup`, it then waits
1 second for a reply on `/osc-setup-reply`.

in `render()` the code receives OSC messages, parses them, and sends
back an acknowledgment.
*/