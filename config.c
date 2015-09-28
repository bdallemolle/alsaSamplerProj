// c-libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
// alsa-proj headers
#include "sampler.h"
#include "config.h"

#define MAX_LINE	200

// config file name
static const char* configFile = "config.txt";		// config file must be in pwd (for now...)
static const char* delim = " \t\n";

// tokens for parsing config file
static const char* controlDevTok = "CONTROL_DEV";
static const char* audioDevTok = "AUDIO_DEV";
static const char* audioFileTok = "AUDIO_FILE";
static const char* actionTok = "ACTION";
static const char* inPortTok = "IN";
static const char* behaviorTok = "BEHAVE";
static const char* eventTok = "EVENT";
static const char* outPortTok = "OUT";

// -------------------------------------------------------------------------- //

bool matchTok(char* tok, const char* s) {
	if (tok == NULL) {
		fprintf(stderr, "matchTok() - null token\n");
		return FALSE;
	}

	if (strcmp(tok, s) != 0) {
		fprintf(stderr, "matchTok() - %s != %s\n", tok, s);
		return FALSE;
	} else return TRUE;
}

// -------------------------------------------------------------------------- //

void ignoreExtraArgs(char buf[]) {
	char* pch = NULL;
	pch = strtok(NULL, delim);
	if (pch != NULL) {
		fprintf(stderr, "config.c - ignoring remaining args!");
		fprintf(stderr, " buf = {%s}\n", buf);
	} 
}

// -------------------------------------------------------------------------- //

int controlDevToCode(char* device) {
	if (strcmp(device, "Commandline") == 0)
		return COMMANDLINE;
	else if (strcmp(device, "RaspiGPIO") == 0)
		return RASPI_GPIO;
	else if (strcmp(device, "RaspiSERIAL") == 0)
		return RASPI_SERIAL;
	else return -1;
}

// -------------------------------------------------------------------------- //

void parseDevices(CONFIG* c, FILE* f) {
	char buf[MAX_LINE];
	char* pch = NULL;

	// parse the control device
	if (fgets(buf, MAX_LINE - 1, f) == NULL) {
		fprintf(stderr, "*** FATAL ERROR - failed to parse control device!\n");
		exit(1);
	}

	pch = strtok(buf, delim);
	if (!matchTok(pch, controlDevTok)) {
		fprintf(stderr, "*** FATAL ERROR - failed to parse control device!\n");
		exit(1);
	}

	pch = strtok(NULL, delim);
	if (pch == NULL) {
		fprintf(stderr, "*** FATAL ERROR - failed to parse control device!\n");
		exit(1);
	}

	// debugging
	fprintf(stderr, "config.c - using %s for control I/O\n", pch);

	// set control device
	c->controlDevID = controlDevToCode(pch);
	ignoreExtraArgs(buf);

	// parse the control device
	if (fgets(buf, MAX_LINE - 1, f) == NULL) {
		fprintf(stderr, "*** FATAL ERROR ***\n");
		fprintf(stderr, " - fgets failed to retrieve config txt line\n");
		exit(1);
	}

	pch = strtok(buf, delim);
	if (!matchTok(pch, audioDevTok)) {
		fprintf(stderr, "*** FATAL ERROR ***\n");
		fprintf(stderr, " - parser failed to parse %s token\n", audioDevTok);
		exit(1);
	}

	pch = strtok(NULL, delim);
	if (pch == NULL) {
		fprintf(stderr, "*** FATAL ERROR ***\n");
		fprintf(stderr, " - parser failed to parse %s argument\n", audioDevTok);
		exit(1);
	}

	// debugging
	fprintf(stderr, "config.c - using %s for audio I/O\n", pch);

	// set audio device
	strcpy(c->audioOutputDevice, pch);		
	ignoreExtraArgs(buf);
}

// -------------------------------------------------------------------------- //

void parseAudioFiles(CONFIG* c, FILE* f) {
	char buf[MAX_LINE];
	char* pch = NULL;
	int nFiles = 0;

    while (1) {
		// parse the control device
		if (fgets(buf, MAX_LINE - 1, f) == NULL && !feof(f)) {
			fprintf(stderr, "*** FATAL ERROR - failed to parse audio file!\n");
			exit(1);
		}
		else if (feof(f)) {
			fprintf(stderr, "config.c - done reading audio files\n");
			break;
		}

		pch = strtok(buf, delim);
		if (!matchTok(pch, audioFileTok)) {
			fprintf(stderr, "config.c - done reading audio files\n");
			break;
		}

		pch = strtok(NULL, delim);
		if (pch == NULL) {
			fprintf(stderr, "*** FATAL ERROR - failed to parse audio file!\n");
			exit(1);
		}

		// debugging
		fprintf(stderr, "config.c - audio file %s registered\n", pch);

		// set audio device
		strcpy(c->audioFiles[nFiles], pch);		
		ignoreExtraArgs(buf);
		nFiles++;
	}

	c->numAudioFiles = nFiles;
}

// -------------------------------------------------------------------------- //

void parseConfigFile(CONFIG* c) {
	// open config file
    FILE* f = fopen(configFile, "r");

    if (f == NULL) {
    	fprintf(stderr, "*** FATAL ERROR - no config file! exiting...\n");
    	exit(1);
    }

    parseDevices(c, f);
    parseAudioFiles(c, f);
    // parse actions ... this is gonna be fun ...

    // close config file
    fclose(f);
}

// -------------------------------------------------------------------------- //

void clearConfigObject(CONFIG* c) {
	int i = 0;

	// debugging
	fprintf(stderr, " - config.c: setting empty config structure\n");

	c->audioOutputDevice[0] = '\0';
	c->audioInputDevice[0] = '\0';

	for (i = 0; i < MAX_AUDIO_FILES; ++i)
		c->audioFiles[i][0] = '\0';
	c->numAudioFiles = 0;
	for (i = 0; i < MAX_SAMPLE; ++i)
		c->sampleMap[i] = -1;
								
	c->controlDevID = -1;	

	for (i = 0; i < MAX_IO; ++i)
		c->readMap[0] = -1;		
	for (i = 0; i < MAX_IO; ++i)
		c->behavior[0] = -1;	
	for (i = 0; i < MAX_IO; ++i)
		c->event[0] = -1;							
	c->numReadPorts = 0;
	for (i = 0; i < MAX_IO; ++i)
		c->writeMap[0] = -1;								
	c->numWritePorts = 0;											

	return;
}

// -------------------------------------------------------------------------- //

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

	parseConfigFile(c);

	// hardcode values into the configuration file, for now
	// strcpy(c->audioOutputDevice, OUTPUT_DEV);		// set output device
	// strcpy(c->audioFiles[0], "gtrsample.wav");		// set audio files...
	// strcpy(c->audioFiles[1], "snarehit1.wav");		// ...
	// c->numAudioFiles = 2;							// hardcoding number of audio files
	// c->controlDevID = RASPI_GPIO;					// RASPI_GPIO number
	c->numReadPorts = 2;							// set number of read ports
	c->numWritePorts = 1;							// set number of read ports

	/**
	 * for now, the following numbers correspond to behaviors
	 * (1) down press button
	 * (2) release button
	 *
	 * for now, the following numbers correspond to playback events
	 * (1) regular one hit sample playback
	 * (2) blocking sample playback
	 * (3) start/stop sample playback
	 * (4) looping playback
	 */

	// configure button 1
	c->sampleMap[0] = 0;							// set sample maps...
	c->event[0] = 4;								// EVENT 1 
	c->readMap[0] = 4;								// set raspi pin 4
	c->behavior[0] = 2;								// BEHAVE 1 == DOWN PRESS BUTTON

	// configutre button 2
	c->sampleMap[1] = 1;							// ... 
	c->event[1] = 1;								// EVENT 2 
	c->readMap[1] = 17;								// set raspi pin 17
	c->behavior[1] = 1;								// BEHAVE 2 == UP PRESS BUTTON
	
	// configure light
	c->writeMap[0] = 22;							// set raspi pin 22

	// set debug flags
	AUDIO_INIT_DEBUG = 0;
	AUDIO_PLAY_DEBUG = 0;
	DEVICE_INIT_DEBUG = 0;
	GENERAL_DEBUG = 0;

	// set some debug flags
	#ifdef AUDIOINITDEBUG
		AUDIO_INIT_DEBUG = 1;
	#endif

	#ifdef AUDIOPLAYDEBUG
		AUDIO_PLAY_DEBUG = 1;
	#endif

	#ifdef DEVICEINITDEBUG
		DEVICE_INIT_DEBUG = 1;
	#endif

	#ifdef GENERALEBUG
		GENERAL_DEBUG = 1;
	#endif

	return 1;	// success!
}