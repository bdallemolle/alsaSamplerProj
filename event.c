// c-libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// alsa-proj headers
#include "sampler.h"
#include "config.h"
#include "device.h"
#include "audio.h"
#include "event.h"

int lightState = 0;

/* -------------------------------------------------------------------------- */

void clearEvents() {
	int i = 0;

	for (i = 0; i < MAX_IO; i++) {
		e.event[i] = NULL;
		e.args[i] = -1;
	}
	return;
}

/* -------------------------------------------------------------------------- */

int playSampleBlock(int idx) {
	fprintf(stderr, " - PLAY SAMPLE EVENT CALLED\n");
	sampleStart(e.args[idx]);
	return 1;
}

/* -------------------------------------------------------------------------- */

int playSampleWithStop(int idx) {
	fprintf(stderr, " - PLAY SAMPLE WITH STOP EVENT CALLED\n");

	if (sampleTable[e.args[idx]].playbackState == STOPPED) {
		sampleStart(e.args[idx]);
	}
	else {
		sampleStop(e.args[idx]);
	}

	return 1;
}

/* -------------------------------------------------------------------------- */

int playSampleLoopWithStop(int idx) {
	fprintf(stderr, " - PLAY SAMPLE *LOOP* WITH STOP EVENT CALLED\n");

	if (sampleTable[e.args[idx]].playbackState == STOPPED) {
		sampleStartLoop(e.args[idx]);
	}
	else {
		sampleStop(e.args[idx]);
	}

	return 1;
}

/* -------------------------------------------------------------------------- */

int oneHitSample(int idx) {
	fprintf(stderr, " - ONE HIT SAMPLE EVENT CALLED\n");
	sampleRestart(e.args[idx]);
	return 1;
}

/* -------------------------------------------------------------------------- */

int lightDemoToggle(int val) {
	if (lightState == 0 && val == 1) {
		fprintf(stderr, "*** LIGHT ON! ***\n");
		// turn on light
		controlDev.digitalWrite(light_fd, 0);
		// set light state to true
		lightState = 1;
	}
	else if (lightState == 1 && val == 0) {
		fprintf(stderr, "*** LIGHT OFF! ***\n");
		// turn off light
		controlDev.digitalWrite(light_fd, 1);
		lightState = 0;
	}
	return 1;
}

/* -------------------------------------------------------------------------- */

int initEvents(CONFIG* c) {
	int i = 0;

	fprintf(stderr, " - event.c: intializing events!\n");

	clearEvents();

	// set events
	for (i = 0; i < c->numReadPorts; i++) {
		if (c->event[i] == 1) {
			e.event[i] = &playSampleLoopWithStop;
			e.args[i] = c->sampleMap[i];
		}
		else if (c->event[i] == 2) {
			e.event[i] = &oneHitSample;
			e.args[i] = c->sampleMap[i];
		}
	}

	/* SET LIGHT BEHAVIOR! */
	e.event[LIGHT] = &lightDemoToggle;
	e.args[LIGHT] = 1;

    return 1;
}