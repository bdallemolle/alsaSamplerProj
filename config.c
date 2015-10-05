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
static const char* configPath = "";	// this should be a cmdline param...
static const char* configFile = "config.txt";		
static const char* delim = " \t\n";

// tokens for parsing config file
static const char* controlDevKey = "CONTROL_DEV";
static const char* audioDevKey = "AUDIO_DEV";
static const char* audioFileKey = "AUDIO_FILE";
static const char* actionKey = "ACTION";
static const char* inPortKey = "IN";
static const char* behaviorKey = "BEHAVE";
static const char* eventKey = "EVENT";
static const char* outPortKey = "OUT";

// -------------------------------------------------------------------------- //

/**
 * given character strings tok and keyword, check if the token matches
 * the desired keyword 
 *
 * returns -1 if tok is NULL, 0 if tok != keyword, and 1 if tok == keyword
 */
int matchTok(char* tok, const char* keyword) {
  if (tok == NULL) {
    fprintf(stderr, "matchTok() - null token\n");
    return -1;
  }
  else if (strcmp(tok, keyword) != 0) {
    fprintf(stderr, "matchTok() - %s != %s\n", tok, keyword);
    return 0;
  } else return 1;
}

// -------------------------------------------------------------------------- //

/**
 * a temporary function which reads input from a buffer and prints useless
 * arguments for debugging.
 */
void ignoreExtraArgs(char buf[]) {
  char* pch = strtok(NULL, delim);
  while (pch != NULL) {
    fprintf(stderr, "config.txt - WARNING: ignoring useless arg:\"%s\"!\n", pch);
    pch = strtok(NULL, delim);
  }
}

// -------------------------------------------------------------------------- //

/**
 * controlDevStringToId(char* str) 
 * given a character string, return the integer tag corresponding 
 * to the control device given by the string
 *
 * returns -1 if string does not match a valid control device.
 * otherwise, returns the integer tag corresponding to the control device
 *
 * TODO: should i move this to device.c?
 */

int controlDevStringToId(char* str) {
  if (strcmp(str, "Commandline") == 0)
    return COMMANDLINE;
  else if (strcmp(str, "RaspiGPIO") == 0)
    return RASPI_GPIO;
  else if (strcmp(str, "RaspiSERIAL") == 0)
    return RASPI_SERIAL;
  else return -1;
}

// -------------------------------------------------------------------------- //

/**
 * parseDevices(CONFIG* c, FILE* f)
 *  - given the open config file pointer and the shared config object, 
 *    parse the audio and control input devices and populate the config 
 *    object accordingly
 * 
 */

/* 		FUTURE IMPLEMENTATION STARTED HERE!  
void parseKeyword(CONFIG* c, FILE* f, char* buf, const char* kw, 
                 (void*) paf (void, void, void, void), bool getNewLine) {
  char* pch = NULL;		// character point for iterating through buf

  // parse the control device
  while (getline(buf, f, getNewLine) != NULL) {		// MAKE THIS FUNCTION
    // get keyword
    pch = strtok(buf, delim);
    if (pch == NULL) {  
      getNewLine = FALSE; 
      continue;
    }
    else if (!matchTok(pch, kw)) {
      fprintf(stderr, "*** FATAL ERROR ***\n");
      fprintf(stderr, " - failed to parse expected %s keyword!\n", kw);
      exit(1);
    }
    
    // parse arguments using specific parse argument function (paf)
    if (paf(c, f, buf, pch)) break;
  }
}
*/

// -------------------------------------------------------------------------- //

/**
 * parseDevices(CONFIG* c, FILE* f)
 *  - given the open config file pointer and the shared config object, 
 *    parse the audio and control input devices and populate the config 
 *    object accordingly
 * 
 * TODO: if possible, make this generic
 *  - add additional arguement for parse arguments that handles arg parsing
 #  - keyword for arguement to parse?
 */
void parseDevices(CONFIG* c, FILE* f) {
  char buf[MAX_LINE];		// buffer for a line of the input file
  char* pch = NULL;		// character point for iterating through buf

  // parse the control device
  while (fgets(buf, MAX_LINE -1, f) != NULL) {
    // find control device keyword
    pch = strtok(buf, delim);
    if (pch == NULL) 
      continue;
    else if (!matchTok(pch, controlDevKey)) {
      fprintf(stderr, "*** FATAL ERROR ***\n");
      fprintf(stderr, " - failed to parse control device!\n");
      exit(1);
    }

    // get control device argument
    pch = strtok(NULL, delim);
    if (pch == NULL) {
      fprintf(stderr, "*** FATAL ERROR ***\n");
      fprintf(stderr, " - no control device provided!\n");
      exit(1);
    }
    else {  
      // set control device
      c->controlDevID = controlDevStringToId(pch);
      ignoreExtraArgs(buf);
      
      if (CONFIG_DEBUG)
        fprintf(stderr, "config.c - using %s for control I/O\n", pch);
      
      break;
    }
  }

  // parse the audio device
  while (fgets(buf, MAX_LINE -1, f) != NULL) {
    // find audio device keyword
    pch = strtok(buf, delim);
    if (pch == NULL) 
      continue;
    else if (!matchTok(pch, audioDevKey)) {
      fprintf(stderr, "*** FATAL ERROR ***\n");
      fprintf(stderr, " - parser failed to parse %s keyword\n", audioDevKey);
      exit(1);
    }

    // get audio device argument
    pch = strtok(NULL, delim);
    if (pch == NULL) {
      fprintf(stderr, "*** FATAL ERROR ***\n");
      fprintf(stderr, " - no %s argument provided (required)\n", audioDevKey);
      exit(1);
    }
    else {  
      // set audio device
      strcpy(c->audioOutputDevice, pch);		
      ignoreExtraArgs(buf);

      if (CONFIG_DEBUG)
        fprintf(stderr, "config.c - using %s for audio I/O\n", pch);
      
      break;
    }
  }

   // check for empty input (end of file)
  if (buf == NULL) {
    fprintf(stderr, "*** FATAL ERROR ***\n");
    fprintf(stderr, " - failed to read required data in config.txt\n"); 
    exit(1);
  }
}

// -------------------------------------------------------------------------- //

void parseAudioFiles(CONFIG* c, FILE* f) {
  char buf[MAX_LINE];
  char* pch = NULL;
  int nFiles = 0;

  while (1) {
    // parse the control device
    if (fgets(buf, MAX_LINE - 1, f) == NULL) {
      fprintf(stderr, "*** FATAL ERROR *** \n");
      fprintf(stderr, " - nothing after audio files! - eventually this will exit\n");
      // exit(1);
      break;
    }

    pch = strtok(buf, delim);
    if (!matchTok(pch, audioFileKey)) {
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
  FILE* f = fopen(configFile, "r");

  if (f == NULL) {
    fprintf(stderr, "*** FATAL ERROR *** \n"); 
    fprintf(stderr, " - no config file! exiting...\n");
    exit(1);
  }

  parseDevices(c, f);
  parseAudioFiles(c, f);
  // parse actions ... this is gonna be fun ...

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
 * setDebugMessages()
 * - sets global debugging flags for different types of errors based on
 *   macros set from compilation. see makefile to filter debugging messages
 */

void setDebugMessages() {
  // set debug flags
  AUDIO_INIT_DEBUG = 0;
  AUDIO_PLAY_DEBUG = 0;
  DEVICE_INIT_DEBUG = 0;
  GENERAL_DEBUG = 0;
  CONFIG_DEBUG = 0;

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

  #ifdef CONFIGDEBUG
    CONFIG_DEBUG = 1;
  #endif

  #ifdef GENERALDEBUG
    GENERAL_DEBUG = 1;
  #endif
}

// -------------------------------------------------------------------------- //

/** 
 * config()
 * - calls function to read and parse the configuration file, 
 *   then populates the give config object with appropriate data so
 *   program can be initialized as desired by user in config file
 *
 * RENAME initConfig();
 */

int config(CONFIG* c) {
  // set debug flags from makefile
  setDebugMessages();

  // set config object initial values!
  clearConfigObject(c);

  // initialize the config structure by parsing the config file
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

   return 1;	// success!
}
