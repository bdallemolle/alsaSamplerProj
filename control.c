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
void hackyCommandLineParse(bool *isDone)
{
  char buf[MAX_NAME];         // stdin input buffer

  fgets(buf, 80, stdin);
  // quick, hacky parse for exiting only
  if (strcmp("-q\n", buf) == 0) 
  {
    *isDone = 1;
    // should kill sound here
    fprintf(stdout, "Exiting program...\n");
  }
  else 
  {
    fprintf(stdout, "Error reading STDIN input...\n");
  }

  return;
}

// -------------------------------------------------------------------------- //

int setPollTable(struct pollfd fdset[], int* nfds) 
{
  int i = 0;

  fprintf(stderr, " - control.c: setting poll fd table...\n");

  // zero out table
  memset((void*)fdset, 0, sizeof(fdset));

  // set, by default, stdin to be fd.et[0] to fdset[0].fd = STDIN_FILENO;
  fdset[0].fd = STDIN_FILENO;
  fdset[0].events = POLLIN;

  // go through device and listen to all open fd's
  for (i = 0; i < controlDev.numReadPorts; i++) {
    fdset[i + 1].fd = controlDev.readPorts[i];
    fdset[i + 1].events = POLLIN;
  }

  *nfds = controlDev.numReadPorts + 1;

  return 1;
}

// -------------------------------------------------------------------------- //

int run(CONFIG* c) 
{
  struct pollfd fdset[MAX_IO+1];      // poll file descriptor set
  int nfds = 0;                       // 
  int timeout = POLL_TIMEOUT;         // poll timeout time                  
  bool isDone = FALSE;                // boolean loop toggle
  int retval = -1;                    // 
  char val;                           //
  int i = 0;                          // loop index variables

	fprintf(stderr, "*** SAMPLER RUNNING! ***\n");

  // set file desciptor tables for polling
  setPollTable(fdset, &nfds);

  // a bad hack for avoiding initial overruns
  fprintf(stderr, "Waiting...\n");
  sleep(1);
  fprintf(stderr, "...\n");
  sleep(1);
  fprintf(stderr, "DONE!\n");

  // start audio thread!
  startAudio();

  while (!isDone) {
    // poll for i/o
    retval = poll(fdset, nfds, timeout);

    // check select values
    if (retval == -1) {
      fprintf(stderr, "*** ERROR: poll failed! ***\n");
      isDone = 1;
    }
    else if (retval) {
      if (fdset[0].revents & POLLIN) {
        hackyCommandLineParse(&isDone);
      }
      for (i = 1; i < nfds; i++) {
        if (fdset[i].revents & POLLIN) {
          // read device
          controlDev.digitalRead(controlDev.readPorts[i - 1], &val);
          // call behavior for device
          if (b.behavior[i - 1](i, val)) {
            e.event[i - 1](i - 1);
          }
        }
      }
      // hacky way to do light status
      e.event[LIGHT](b.behavior[LIGHT](LIGHT, val));
    }
    else if (GENERAL_DEBUG) {
      fprintf(stderr, " - POLL TIMEOUT...\n");
    }
  }

  // cleanup
  killAudio();                   // kills playback loop/thread
	clearAudioTable();             // clears audio table
  clearSampleTable();            // clears the sample table

	return 1;
}