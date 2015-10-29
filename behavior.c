// c-libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// alsa-proj headers
#include "sampler.h"
#include "config.h"
#include "audio.h"
#include "behavior.h"

int previousButtonVal[MAX_IO];
int previousButtonState[MAX_IO];

/* -------------------------------------------------------------------------- */

void clearBehaviors() {
	int i = 0;
	b.numBehaviors = 0;							
	for (i = 0; i < MAX_IO; i++) {
		b.behavior[i] = NULL;
		previousButtonVal[i] = 0;
		previousButtonState[i] = 0;
	}
	return;
}

/* -------------------------------------------------------------------------- */

// behavior functions
int downPressButton(int i, int val) {
  // if (GENERAL_DEBUG) {
    fprintf(stderr, " behavior.c - down button press called!\n");
    fprintf(stderr, " behavior.c - IGNORING value of fd %d is %d!\n", i, val);
  // }

  /*
  previousButtonState[i]++;
  if (previousButtonState[i] == 1) {
  	// if (GENERAL_DEBUG) {
	fprintf(stderr, "*** REGISTER DOWN PRESS FOR INPUT %d ***\n", i);
	// }
	return 1;
  }
  else if (previousButtonState[i] == 2) {
	previousButtonState[i] = 0;
  }	
  */

  if (val != previousButtonVal[i]) {
    previousButtonVal[i] = val;
	previousButtonState[i]++;
	  if (previousButtonState[i] == 1) {
		fprintf(stderr, "*** REGISTER DOWN PRESS FOR INPUT %d ***\n", i);
		return 1;
	  }
	else if (previousButtonState[i] == 2) {
	  previousButtonState[i] = 0;
	}
  }
  return 0;

}

/* -------------------------------------------------------------------------- */

int upPressButton(int i, int val) {
  // if (GENERAL_DEBUG) {
    fprintf(stderr, " behavior.c - up button press called!\n");
	fprintf(stderr, " behavior.c - IGNORING value of fd %d is %d!\n", i, val);
  // }

  /*
  previousButtonState[i]++;
  if (previousButtonState[i] == 2) {
	fprintf(stderr, "*** REGISTER UP PRESS FOR INPUT %d ***\n", i);
	previousButtonState[i] = 0;
	return 1;
  }
  else {
    return 0;
  }
  */

  if (val != previousButtonVal[i]) {
    previousButtonVal[i] = val;
	previousButtonState[i]++;
    if (previousButtonState[i] == 2) {
	  fprintf(stderr, "*** REGISTER UP PRESS FOR INPUT %d ***\n", i);
	  previousButtonState[i] = 0;
	  return 1;
	}
  }
  return 0;

}

/* -------------------------------------------------------------------------- */

// sets a light whenever ANY audio is playing
int lightDemo(int j, int val) {
	int i = 0;
	// fprintf(stderr, "*** LIGHT DEMO CALLED ***\n");

	for (i = 0; i < MAX_SAMPLE; i++)
		if (sampleTable[i].playbackState == PLAYING)
			return 1;

	return 0;
}

/* -------------------------------------------------------------------------- */

int initBehavior(CONFIG* c) {
	int i = 0;

	fprintf(stderr, " - behavior.c: intializing behaviors!\n");

	clearBehaviors();

	for (i = 0; i < c->numReadPorts; i++) {
		if (c->behavior[i] == 1) {
			b.behavior[i] = &downPressButton;
			b.numBehaviors++;
		}
		else if (c->behavior[i] == 2) {
			b.behavior[i] = &upPressButton;
			b.numBehaviors++;
		}
	}

	/* SET LIGHT BEHAVIOR! */
	fprintf(stderr, " - behavior.c: setting light behavior!\n");
	b.behavior[LIGHT] = &lightDemo;

    return 0;
}

