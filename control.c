// c-lib includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <poll.h>
#include <pthread.h>
#include <ctype.h>
// sample-proj headers
#include "sampler.h"
#include "config.h"
#include "control.h"
#include "behavior.h"
#include "device.h"
#include "event.h"
#include "audio.h"

// -------------------------------------------------------------------------- //

// a function to remove once their are virtual devices
void hackyCommandLineParse(bool *isDone) {
  char buf[MAX_NAME];         // stdin input buffer

  fgets(buf, 80, stdin);
  // quick, hacky parse for exiting only
  if (strcmp("-q\n", buf) == 0) {
    *isDone = 1;
    // should kill sound here
    fprintf(stdout, "Exiting program...\n");
  }
  else {
    if (GENERAL_DEBUG)
      fprintf(stdout, "Error reading STDIN input...\n");
  }

  return;
}

// -------------------------------------------------------------------------- //

void setPollTable(struct pollfd fdset[], int* nfds) {
  if (GENERAL_DEBUG)
    fprintf(stderr, " - control.c: setting poll fd table...\n");

  // set, by default, stdin to be fd.et[0] to fdset[0].fd = STDIN_FD;
  fdset[0].fd = STDIN_FD;
  fdset[0].events = POLLIN;

  // go through device and listen to all open fd's
  // CAREFUL: numReadPorts could be 80...
  for (int i = 0; i < controlDev.numReadPorts; ++i) {
    fdset[i + 1].fd = controlDev.readPorts[i];
    fdset[i + 1].events = POLLPRI;
  }

   // zero out table
  memset((void*)fdset, 0, sizeof(fdset) * (controlDev.numReadPorts - 1));

  *nfds = controlDev.numReadPorts + 1;
}

// -------------------------------------------------------------------------- //

int run(CONFIG* c) 
{
  struct pollfd fdset[MAX_IO+1];	 // poll file descriptor set
  int nfds = 0;				             // number of fd's to poll 
  int timeout = POLL_TIMEOUT; 		 // ...self explanitory                  
  bool isDone = FALSE;			       // loop toggle
  int retval = -1;			           // poll return value 
  char readVal;				                 // single character for reading (should remove)

  if (GENERAL_DEBUG)
    fprintf(stderr, "*** SAMPLER RUNNING! ***\n");

  startAudio();

  while (!isDone) {
    // set the poll table and wait on interrupts
    setPollTable(fdset, &nfds);
    retval = poll(fdset, nfds, timeout);

    if (retval == -1) {
      fprintf(stderr, "*** ERROR: poll failed! ***\n");
      isDone = 1;
    }
    // there is input to read
    else if (retval) {
      // check for HACKED exit from commandline
      if (fdset[0].revents & POLLIN) 
        hackyCommandLineParse(&isDone);

      // check other file descriptors
      for (int i = 1; i < nfds; ++i) {
        if (fdset[i].revents & POLLPRI) {
          // read device
          controlDev.digitalRead(controlDev.readPorts[i - 1], &readVal);

          // verify behavior to trigger event
          if (b.behavior[i - 1](i, readVal)) {
            // perform event if behavior is triggered
            e.event[i - 1](i - 1);
          }
        }
      }
    }
    // there was a timeout
    else {
      if (GENERAL_DEBUG)
        fprintf(stderr, " - POLL TIMEOUT...\n");

      /**
       * THIS IS WHERE TIMEOUT ACTIONS WOULD BE CHECKED
       * FOR EXAMPLE, THE LIGHT STATE
       */

      // hacky way to do light status
      e.event[LIGHT](b.behavior[LIGHT](LIGHT, readVal));
    }
  }

  killAudio();                   // kills playback loop/thread
	clearAudioTable();             // clears audio table
  clearSampleTable();            // clears the sample table

	return 1;
}
