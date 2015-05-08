// c-libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// alsa-proj headers
#include "sampler.h"
#include "config.h"

void clearConfigObject(CONFIG* c) {
	int i = 0;

	// debugging
	fprintf(stderr, " - config.c: setting empty config structure\n");

	// clear names of audio i/o
	c->audioOutputDevice[0] = '\0';
	c->audioInputDevice[0] = '\0';

	// clean all audio files
	for (i = 0; i < MAX_AUDIO_FILES; i++)
		c->audioFiles[i][0] = '\0';
	c->numAudioFiles = 0;
	for (i = 0; i < MAX_SAMPLE; i++)
		c->sampleMap[i] = -1;

	// control device									
	c->controlDevID = -1;	

	// input/output fd mapping
	for (i = 0; i < MAX_IO; i++)
		c->readMap[0] = -1;		
	for (i = 0; i < MAX_IO; i++)
		c->behavior[0] = -1;							
	c->numReadPorts = 0;
	for (i = 0; i < MAX_IO; i++)
		c->writeMap[0] = -1;								
	c->numWritePorts = 0;											

	return;
}

/** 
 * config()
 * - calls functino to read and parse the configuration file, 
 *   then populates the config object with appropriate data so
 *   program can be initialized as desired by user in config file
 */
int config(CONFIG* c) {
	fprintf(stderr, " - config.c: CONFIG FUNC CALLED\n");

	// set config structures initial values!
	clearConfigObject(c);

	// hardcode values into the configuration file, for now
	strcpy(c->audioOutputDevice, OUTPUT_DEV);		// set output device
	strcpy(c->audioFiles[0], "gtrsample.wav");		// set audio files...
	strcpy(c->audioFiles[1], "snarehit1.wav");		// ...
	c->sampleMap[0] = 0;							// set sample maps...
	c->sampleMap[1] = 1;							// ... 
	c->event[0] = 1;								// EVENT 1 == REGULAR SAMPLE START/STOP
	c->event[1] = 2;								// EVENT 2 == ONE-HIT SAMPLE
	c->numAudioFiles = 2;							// hardcoding number of audio files
	// set devices...
	c->controlDevID = 2;							// RASPI_GPIO number
	c->readMap[0] = 4;								// set raspi pin 4
	c->behavior[0] = 2;								// BEHAVE 1 == DOWN PRESS BUTTON
	c->readMap[1] = 17;								// set raspi pin 17
	c->behavior[1] = 1;								// BEHAVE 2 == UP PRESS BUTTON
	c->numReadPorts= 2;								// set number of read ports
	c->writeMap[0] = 22;							// set raspi pin 22
	c->numWritePorts= 1;							// set number of read ports

	// set some debug flags
	AUDIO_INIT_DEBUG = 1;
	AUDIO_PLAY_DEBUG = 1;
	DEVICE_INIT_DEBUG = 1;

	return 1;	// success!
}