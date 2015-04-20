#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
// sampler proj headers
#include "sampler.h"
#include "parse.h"


/* -------------------------------------------------------------------------- */

void initDebug() {
	// init audio debugging flags
	AUDIO_INIT_DEBUG = 0;
	AUDIO_PLAY_DEBUG = 0;

	// init _____ debugging flags

	// init _____ debugging flags

	// init _____ debugging flags

	return;
}

/* -------------------------------------------------------------------------- */

// set debugging flags
void set_debug_flags(int argc, char* argv[], int* i) {
	int j = 1;

	fprintf(stderr, "DEBUG FLAG READ\n");

	while (((*i + j) < argc) && (argv[*i + j][0] != '-')) {
		fprintf(stderr, "READING DEBUG ARGUMENT: %s\n", argv[*i + j]);
		if (strcmp("init", argv[*i + j]) == 0) {
			AUDIO_INIT_DEBUG = 1;
			fprintf(stderr, "*** AUDIO INIT DEBUGGING ***");
		}
		if (strcmp("play", argv[*i + j]) == 0) {
			AUDIO_PLAY_DEBUG = 1;
			fprintf(stderr, "*** AUDIO PLAY DEBUGGING ***");
		}
    	j++;
	}

	*i = *i + j;

	return;
}

/* -------------------------------------------------------------------------- */

// set audio flags
void set_audio_flags(int argc, char* argv[], int* i) {
	int j = 1;

	fprintf(stderr, "AUDIO FLAG READ\n");

	while (((*i + j) < argc) && (argv[*i + j][0] != '-')) {
		fprintf(stderr, "READING AUDIO ARGUMENT: %s\n", argv[*i + j]);
    	j++;
	}

	*i = *i + j;

	return;
}

/******************************************************************************
 *                            INTERFACED FUNCTIONS                            *
 ******************************************************************************/

void parse_commandline(int argc, char* argv[]) {
	int i = 0;

	// init debug flags
	initDebug();

	// parse commandline
	while (i < argc) {
		if (argv[i][0] == '-') {
			fprintf(stderr, "FLAG READ: %s\n", argv[i]);
			switch (argv[i][1]) {
				case 'f':
					// debug flag read
					set_debug_flags(argc, argv, &i);
					break;
				case 'a':
					// audio flag read
					set_audio_flags(argc, argv, &i);
					break;
				default:
					fprintf(stderr, "*** ERROR: UNKNOWN FLAG READ ***\n");
					break;
			}
		}
		else {
			fprintf(stderr, "INPUT READ: %s\n", argv[i]);
			i++;
		}
	}
	return;
}