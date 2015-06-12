// c-libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// alsa-proj headers
#include "sampler.h"
#include "config.h"
#include "device.h"
// device interfaces
#include "raspiGPIO.h"

/* -------------------------------------------------------------------------- */

void clearControlDevice() {
	controlDev.id = 0;										// later, for when this is a list
	controlDev.isActive = 0;								// not sure why i need this?
	controlDev.numReadPorts = 0;							// number of active listening ports
	memset(controlDev.readPorts, 0, MAX_IO*sizeof(int));	// fd's of input/output ports	
	controlDev.numWritePorts = 0;							// write ports
	memset(controlDev.writePorts, 0, MAX_IO*sizeof(int));	// fd's of input/output ports				
	controlDev.digitalRead = NULL;							// digital read
	controlDev.digitalWrite = NULL;							// digital write 
	return;
}

/* -------------------------------------------------------------------------- */

// function to init a device passed as an argument
int initDevice(CONFIG* c) {
	fprintf(stderr, " - device.c: init device called!\n");
	// intialize control device structure will nulls
	clearControlDevice();

	// go through list of devices
	switch (c->controlDevID) {
		case COMMANDLINE:
			fprintf(stderr, " - DEVICE.c: calling init commandline (NOT IMPLEMENTED)\n");
			// initCommandlineDevice(c);
			break;
		case RASPI_GPIO:
			fprintf(stderr, " - DEVICE.c: calling init raspi GPIO \n");
			initRaspiGPIO(c);
			break;
		case RASPI_SERIAL:
			fprintf(stderr, " - DEVICE.c: calling init raspi serial (NOT IMPLEMENTED)\n");
			// initRaspiSerial(c);
			break;
		default: 
			fprintf(stderr, "*** ERROR: Unknown device to initiate ***\n");
			break;
	}

	// set up sampler device!!!
	// fprintf(stderr, "*** SHOULD SET UP SAMPLER LISTENING PORTS HERE! ***\n");

	return;
}
