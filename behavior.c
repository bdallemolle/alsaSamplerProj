// c-libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// alsa-proj headers
#include "sampler.h"
#include "config.h"
#include "behavior.h"

int previousButtonVal[MAX_IO];
int previousButtonState[MAX_IO];

/* -------------------------------------------------------------------------- */

void clearBehaviors() {
	int i = 0;
	b.numBehaviors = 0;							
	for (i = 0; i < MAX_IO; i++) {
		b.behavior[i] = NULL;
		previousButtonVal[i] = 1;
		previousButtonState[i] = 0;
	}
	return;
}

/* -------------------------------------------------------------------------- */

// behavior functions
int downPressButton(int i, int val) {
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

    return 0;
}

