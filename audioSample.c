// c-libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
// sampler proj headers
#include "sampler.h"
#include "config.h"
#include "audio.h"

// -------------------------------------------------------------------------- //
// ----------------------- SAMPLE TABLE FUNCTIONS --------------------------- //
// -------------------------------------------------------------------------- //

// clears/zero-outs an entry in the mix table
void clearSampleTableEnt(int i) 
{
  sampleTable[i].id = i;                          // sample id
  sampleTable[i].audioIdx = -1;                   // index into audio table
  sampleTable[i].mixIdx = -1;                     // index into mix table
  sampleTable[i].playbackState = STOPPED;         // playing/stopped state
  sampleTable[i].overlay = 0;                     // toggle for overlayed sample
  sampleTable[i].numOverlays = 0;                 // number of overlaid copies of sample playing 
  sampleTable[i].loop = FALSE;                    // 
  sampleTable[i].digitalBehavior = NULL;          // digital behavior function associated with sample             
  return;
}

// -------------------------------------------------------------------------- //

// clears/zero-outs an entry in the mix table
void clearSampleTable() {
  int i;

  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, " - Clearing the SAMPLE table.\n");

  for (i = 0; i < MAX_SAMPLE; i++)
    clearSampleTableEnt(i);

  return;
}


// -------------------------------------------------------------------------- //

// needs a configuration arguement
int setSampleTable(char filenames[MAX_AUDIO_FILES][MAX_NAME], int sampleMap[]) 
{
  int i = 0;

  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, "*** SETTING SAMPLE TABLE! ***\n");

  clearSampleTable();

  for (i = 0; i < MAX_SAMPLE; i++) 
  {
    if (sampleMap[i] >= 0) 
    {
      if (AUDIO_INIT_DEBUG)
        fprintf(stderr, " - audio.c: setting sample %d to audio file %d\n", i, sampleMap[i]);
      sampleTable[i].audioIdx = sampleMap[i];                       
    }
  }

  return 1;
}      

// -------------------------------------------------------------------------- //
// ----------------------- SAMPLE CONTROL FUNCTIONS ------------------------- //
// -------------------------------------------------------------------------- //

int sampleStart(int sampleID) 
{
  if (AUDIO_PLAY_DEBUG)
    fprintf(stderr, " - Sample %d told to START\n", sampleID);

  if (sampleID < 0) 
    return -1;

  // don't start sample if already playing
  if (sampleTable[sampleID].playbackState == PLAYING) 
  {
    if (AUDIO_PLAY_DEBUG)
      fprintf(stderr, " - Sample %d already playing, can't start again!\n", sampleID);
    return 1;
  }

  sampleTable[sampleID].mixIdx = setMixTableFile(sampleTable[sampleID].audioIdx, &sampleTable[sampleID]);
  sampleTable[sampleID].playbackState = PLAYING;

  return 1;
}

// -------------------------------------------------------------------------- //

int sampleStop(int sampleID) 
{
  if (AUDIO_PLAY_DEBUG)
    fprintf(stderr, " - Sample %d told to STOP\n", sampleID);

  pthread_mutex_lock(&mix_lock);
  mixTable[sampleTable[sampleID].mixIdx].blackSpot = 1;
  mixTable[sampleTable[sampleID].mixIdx].sampleStop = 1;
  pthread_mutex_unlock(&mix_lock);

  return 1;
}

// -------------------------------------------------------------------------- //

int sampleRestart(int sampleID) 
{
  if (AUDIO_PLAY_DEBUG)
    fprintf(stderr, " - Sample %d told to RESTART\n", sampleID);

  if (sampleTable[sampleID].playbackState == STOPPED) {
    sampleStart(sampleID);
  }
  else {
    // stop currently playing sample
    pthread_mutex_lock(&mix_lock);
    mixTable[sampleTable[sampleID].mixIdx].blackSpot = 1;
    mixTable[sampleTable[sampleID].mixIdx].sampleStop = 0;
    pthread_mutex_unlock(&mix_lock);
    // set new sample to start
    sampleTable[sampleID].mixIdx = setMixTableFile(sampleTable[sampleID].audioIdx, &sampleTable[sampleID]);
    sampleTable[sampleID].playbackState = PLAYING;
  }

  return 1;
}

// -------------------------------------------------------------------------- //

int sampleStartLoop(int sampleID) 
{
  if (AUDIO_PLAY_DEBUG)
    fprintf(stderr, " - Sample %d told to START\n", sampleID);

  if (sampleID < 0) 
    return -1;

  // don't start sample if already playing
  if (sampleTable[sampleID].playbackState == PLAYING) {
    if (AUDIO_PLAY_DEBUG)
      fprintf(stderr, " - Sample %d already playing, can't start again!\n", sampleID);

    return 1;
  }

  sampleTable[sampleID].mixIdx = setMixTableFile(sampleTable[sampleID].audioIdx, &sampleTable[sampleID]);
  sampleTable[sampleID].playbackState = PLAYING;
  sampleTable[sampleID].loop = TRUE;

  return 1;
}

// -------------------------------------------------------------------------- //

// *not* impelmented!
int sampleOverlay(int sampleID) 
{
  if (AUDIO_PLAY_DEBUG)
    fprintf(stderr, " - Sample %d told to play OVERLAID!\n", sampleID);
  // get file num for sample ID

  // set mix table playback

  return 1;
}

// -------------------------------------------------------------------------- //

// *not* impelmented!
int sampleStopALL() 
{
  if (AUDIO_PLAY_DEBUG) 
    fprintf(stderr, " - ALL samples told to STOP");

  return 1;
}

// -------------------------------------------------------------------------- //

// remove this eventually
int setPlaybackSound(int idx) 
{
  return setMixTableFile(idx, NULL);        // success
}