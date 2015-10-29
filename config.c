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

#define MAX_LINE    255
#define MAX_BUF     80

// sample tag for sample -> audio file translation
typedef struct {
  char tag[MAX_BUF];
  char audio[MAX_BUF];
} SAMPLE_TAG;

// config file name
static const char* configPath = "";	              // TODO: commandline parameter
static const char* configFile = "config.txt";		
static const char* delim = " \t\n";

// tokens for parsing config file
static const char* controlDevKey = "CONTROL_DEV";
static const char* audioDevKey = "AUDIO_DEV";
static const char* audioFileKey = "AUDIO_FILE";
static const char* sampleKey = "SAMPLE";
static const char* actionKey = "ACTION";
static const char* inPortKey = "IN";
static const char* behaviorKey = "BEHAVE";
static const char* eventKey = "EVENT";
static const char* outPortKey = "OUT";

// action tag data
static int sampleTagIdx = 0;
static SAMPLE_TAG sampleTagTable[MAX_SAMPLE];

// -------------------------------------------------------------------------- //

/**
 * given character strings tok and keyword, check if the token matches
 * the desired keyword 
 *
 * returns -1 if tok is NULL, 0 if tok != keyword, and 1 if tok == keyword
 */
int matchTok(char* tok, const char* keyword) {
  if (tok == NULL) {
    if (CONFIG_DEBUG)
      fprintf(stderr, "matchTok() - null token\n");
    return -1;
  }
  else if (strcmp(tok, keyword) != 0) {
    if (CONFIG_DEBUG)
      fprintf(stderr, "matchTok() - %s != %s\n", tok, keyword);
    return 0;
  } 
  else {
    if (CONFIG_DEBUG)
      fprintf(stderr, "matchTok() - %s = %s\n", tok, keyword);
    return 1;
  }
}

// -------------------------------------------------------------------------- //

/**
 * utility function which reads input from a buffer and prints useless
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
 * 
 * given a character string, return the integer tag corresponding 
 * to the control device given by the string
 *
 * returns -1 if string does not match a valid control device.
 * otherwise, returns the integer tag corresponding to the control device
 *
 * NOTE: should i move this to device.c?
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
 * lookUpActionTag(char* buf)
 *  - given a character string, return the index of an action tag
 *  - returns the index of the action tag, or -1 if not found
 */

int lookUpAudioFile(char* buf, CONFIG* c) {
  for (int i = 0; i < MAX_AUDIO_FILES; ++i)
    if (strcmp(buf, c->audioFiles[i]) == 0)
      return i;
  return -1;
}

// -------------------------------------------------------------------------- //

/**
 * lookUpSampleTag(char* buf)
 *  - given a character string, return the index of an sample tag
 *  - returns the index of the sample tag, or -1 if not found
 */

int lookUpSampleTag(char* buf) {
  for (int i = 0; i < MAX_SAMPLE; ++i)
    if (strcmp(buf, sampleTagTable[i].tag) == 0)
      return i;
  return -1;
}

// -------------------------------------------------------------------------- //

/**
 * parseDevices(CONFIG* c, FILE* f)
 *  - given the open config file pointer and the shared config object, 
 *    parse the audio and control input devices and populate the config 
 *    object accordingly
 *  - program cannot proceed without successfully parsing a control
 *    device and an audio device!
 */

void parseDevices(CONFIG* c, FILE* f) {
  char buf[MAX_LINE];		       // buffer for a line of the input file
  char* pch = NULL;		         // character point for iterating through buf

  // parse the control device
  while (fgets(buf, MAX_LINE - 1, f) != NULL) {
    // find CONTROL_DEV keyword
    pch = strtok(buf, delim);
    if (pch == NULL) 
      continue;
    else if (matchTok(pch, controlDevKey) <= 0) {
      fprintf(stderr, "*** FATAL ERROR ***\n");
      fprintf(stderr, " - failed to parse required %s keyword!\n", controlDevKey);
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
  while (fgets(buf, MAX_LINE - 1, f) != NULL) {
    // find audio device keyword
    pch = strtok(buf, delim);
    if (pch == NULL) 
      continue;
    else if (matchTok(pch, audioDevKey) <= 0) {
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

void parseAudioFile(CONFIG* c, FILE* f) {
  char* pch = NULL;

  pch = strtok(NULL, delim);
  if (pch == NULL) {
    fprintf(stderr, "*** FATAL ERROR ***\n");
    fprintf(stderr, " - no audio file name provided!\n");
    exit(1);
  }

  // debugging
  if (CONFIG_DEBUG)
    fprintf(stderr, "config.c - audio file %s registered\n", pch);

  if (lookUpAudioFile(pch, c) < 0) {
    // set audio device
    strcpy(c->audioFiles[c->numAudioFiles], pch);	
    c->numAudioFiles++;	
  }
  else {
    fprintf(stderr, "config.c - WARNING: ignoring duplicate audio file\n");
  }
}

// -------------------------------------------------------------------------- //

void parseSample(CONFIG *c, FILE *f) {
  char *pch = NULL;
  int audioIdx = 0;

  pch = strtok(NULL, delim);
  if (pch == NULL) {
    fprintf(stderr, "*** FATAL ERROR *** \n");
    fprintf(stderr, " - config syntax: no sample tag provided!\n");
    exit(1);
  }

  // add sample tag
  if (sampleTagIdx == MAX_SAMPLE) {
    fprintf(stderr, "*** FATAL ERROR ***\n");
    fprintf(stderr, " - config syntax: too many samples!\n");
    exit(1);
  }
  else if (lookUpSampleTag(pch) < 0) {
    strcpy(&sampleTagTable[sampleTagIdx].tag[0], pch);
  }
  else {
    fprintf(stderr, "*** FATAL ERROR ***\n");
    fprintf(stderr, " - config syntax: duplicate sample tag!\n");
    exit(1);
  }

  // grab the audio file
  pch = strtok(NULL, delim);
  if (pch == NULL) {
    fprintf(stderr, "*** FATAL ERROR *** \n");
    fprintf(stderr, " - config syntax: no audio file provided for sample!\n");
    exit(1);
  }

  // verify audio file exists
  if ((audioIdx = lookUpAudioFile(pch, c)) > 0) {
    strcpy(&sampleTagTable[sampleTagIdx].audio[0], pch);
  }
  else {
    fprintf(stderr, "*** FATAL ERROR ***\n");
    fprintf(stderr, " - config syntax: sample's audio file does not exist!\n");
    exit(1);
  }

  sampleTagIdx++;
}

// -------------------------------------------------------------------------- //

void parseConfigFile(CONFIG *c) {
  FILE *f = fopen(configFile, "r");
  char buf[MAX_LINE];
  char *pch = NULL;
  int ret = 0;   

  if (f == NULL) {
    fprintf(stderr, "*** FATAL ERROR *** \n"); 
    fprintf(stderr, " - no config file! exiting...\n");
    exit(1);
  }

  // global definitions, must be initialized in order
  parseDevices(c, f);

  // parse AUDIO FILES
  while (c->numAudioFiles < MAX_AUDIO_FILES) {
    // grab an audio file
    if (fgets(buf, MAX_LINE - 1, f) == NULL) {
      if (CONFIG_DEBUG)
        fprintf(stderr, "*** DONE PARSING *** \n");
      break;
    }

    // check for AUDIO FILE keyword
    pch = strtok(buf, delim);
    if ((ret = matchTok(pch, audioFileKey)) > 0) {
      if (CONFIG_DEBUG)
        fprintf(stderr, "config.c - parsing audio file\n");
      parseAudioFile(c, f);
      ignoreExtraArgs(buf);
    }
    else if (ret == 0) {
      // no AUDIO_FILE keyword, done with audio file section
      if (CONFIG_DEBUG)
        fprintf(stderr, "config.c - done parsing audio files\n");
      break;
    }
  }

  // NOTE: HERE WE WOULD PARSE SETTINGS IN A LOOP

  // NOTE: this should be cleared per setting!
  // this way, action tags have 'setting scope'
  for (int i = 0; i < MAX_SAMPLE; ++i) {
    sampleTagTable[i].tag[0] = '\0';
    sampleTagTable[i].audio[0] = '\0';
  }

  // parse SAMPLES
  while (sampleTagIdx < MAX_SAMPLE) {
    // grab an SAMPLE
    if (fgets(buf, MAX_LINE - 1, f) == NULL) {
      fprintf(stderr, "*** DONE PARSING *** \n");
      break;
    }

    // check for SAMPLE keyword
    pch = strtok(buf, delim);
    if ((ret = matchTok(pch, sampleKey)) > 0) {
      if (CONFIG_DEBUG)
        fprintf(stderr, "config.c - parsing sample\n");
      parseSample(c, f);
      ignoreExtraArgs(buf);
    }
    else if (ret == 0) {
      if (CONFIG_DEBUG)
        fprintf(stderr, "config.c - done parsing samples\n");
      break;
    }
  }

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
 *   macros set from compilation. debug settings are "baked" into executable 
 *   see makefile to filter debugging messages
 */

void setDebugMessages() {
  // set debug flags
  AUDIO_INIT_DEBUG = 0;
  AUDIO_PLAY_DEBUG = 0;
  DEVICE_INIT_DEBUG = 0;
  GENERAL_DEBUG = 0;
  CONFIG_DEBUG = 0;
  AUDIO_DISABLED = 0;

  // set some debug flags
  #ifdef AUDIOINITDEBUG
    AUDIO_INIT_DEBUG = 1;
    fprintf(stderr, " config.c - AUDIO INIT DEBUGGING\n");
  #endif

  #ifdef AUDIOPLAYDEBUG
    AUDIO_PLAY_DEBUG = 1;
    fprintf(stderr, " config.c - AUDIO PLAY DEBUGGING\n");
  #endif

  #ifdef DEVICEINITDEBUG
    DEVICE_INIT_DEBUG = 1;
    fprintf(stderr, " config.c - DEVICE INIT DEBUGGING\n");
  #endif

  #ifdef CONFIGDEBUG
    CONFIG_DEBUG = 1;
    fprintf(stderr, " config.c - CONFIG DEBUGGING\n");
  #endif

  #ifdef GENERALDEBUG
    GENERAL_DEBUG = 1;
    fprintf(stderr, " config.c - GENERAL DEBUGGING\n");
  #endif

  // audio disabled for debugging
  #ifdef AUDIODISABLED
    AUDIO_DISABLED = 1;
    fprintf(stderr, " config.c - AUDIO DISABLED\n");
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
  setDebugMessages();

  // initialize the config structure by parsing the config file
  clearConfigObject(c);
  parseConfigFile(c); 

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
   c->sampleMap[0] = 0;							  // set sample maps...
   c->event[0] = 4;								    // EVENT 1 
   c->readMap[0] = 4;								  // set raspi pin 4
   c->behavior[0] = 2;								// BEHAVE 1 == DOWN PRESS BUTTON

   // configutre button 2
   c->sampleMap[1] = 1;							  // ... 
   c->event[1] = 1;								    // EVENT 2 
   c->readMap[1] = 17;								// set raspi pin 17
   c->behavior[1] = 1;								// BEHAVE 2 == UP PRESS BUTTON
	
   // configure light
   c->writeMap[0] = 22;							  // set raspi pin 22

   return 1;	// success!
}
