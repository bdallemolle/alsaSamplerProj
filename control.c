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
#include "device.h"
#include "audio.h"


/* -------------------------------------------------------------------------- */

int setPollTable(struct pollfd fdset[], int* nfds) {
  int i = 0;

  fprintf(stderr, " - control.c: setting poll fd table...\n");

  // zero out table
  memset((void*)fdset, 0, sizeof(fdset));

  // set, by default, stdin to be fd.et[0] to fdset[0].fd = STDIN_FILENO;
  fdset[0].fd = STDIN_FILENO;
  fdset[0].events = POLLIN;

  // go through device and listen to all open fd's
  for (i = 0; i < controlDev.numReadPorts; i++) {
    fdset[i+1].fd = controlDev.readPorts[i];
    fdset[i+1].events = POLLIN;
  }

  *nfds = controlDev.numReadPorts + 1;

  return 1;
}

/* -------------------------------------------------------------------------- */

int run(CONFIG* c) {
  struct pollfd fdset[MAX_IO+1];      // poll file descriptor set
  int nfds = 0;                       // 
  int timeout = POLL_TIMEOUT;         // poll timeout time                  
  char buf[MAX_NAME];                 // stdin input buffer
  bool isDone = FALSE;                // boolean loop toggle
  int retval = -1;                    // 
  char val;                            //
  int i = 0;                          // loop index variables

	fprintf(stderr, "*** RUN CALLED! DO \"CONTROL\" STUFF ***\n");

  // set file desciptor tables for polling
  setPollTable(fdset, &nfds);

  // a bad hack for avoiding initial overruns
  fprintf(stderr, "Waiting...\n");
  sleep(1);
  fprintf(stderr, "DONE!\n");

  // start audio thread!
  startAudio();

  while (!isDone) {
    // poll for i/o
    retval = poll(fdset, nfds, timeout);

    // check select values
    if (retval == -1) {
      fprintf(stderr, "*** POLL ERROR! ***\n");
      isDone = 1;
    }
    else if (retval) {
      // fprintf(stderr, "retval for select %d\n", retval);
      if (fdset[0].revents & POLLIN) {
        fprintf(stderr, "INPUT FRON STDIN!\n");
        fgets(buf, 80, stdin);
        // quick, hacky parse
        if (strcmp("-q\n", buf) == 0) {
          isDone = 1;
          // should kill sound here
          fprintf(stdout, "Exiting main loop...\n");
        }
        else {
          fprintf(stdout, "Error reading STDIN input...\n");
        }
      }
      for (i = 1; i < nfds; i++) {
        if (fdset[i].revents & POLLIN) {
          controlDev.digitalRead(fdset[i].fd, &val);
          // controlDev.behavior( )
          // controlDev.event( )
          fprintf(stderr, "READ %d\n", val);
        }
      }
    }
    else {
      fprintf(stderr, " - POLL TIMEOUT...\n");
    }
  }

  // cleanup
  killAudio();                   // kills playback loop/thread
	clearAudioTable();             // clears audio table
  clearSampleTable();            // clears the sample table

	return 1;
}