// c-libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
// alsa-libs
#include <alsa/asoundlib.h>
// sampler proj headers
#include "sampler.h"
#include "config.h"
#include "audio.h"

// -------------------------------------------------------------------------- //
// ------------------------ MIX TABLE FUNCTIONS ----------------------------- //
// -------------------------------------------------------------------------- //

// clears/zero-outs an entry in the mix table
void clearMixTableEnt(int i) 
{
   mixTable[i].addr = NULL;
   mixTable[i].file = NULL;
   mixTable[i].iterIdx = 0;
   mixTable[i].nFrames = 0;
   mixTable[i].lastSampleIdx  = 0;
   mixTable[i].blackSpot = 0;
   mixTable[i].sampleStop = 0;
   mixTable[i].s = NULL;
   return;
}

// -------------------------------------------------------------------------- //

// clears/zero-outs an entry in the mix table
void clearMixTable() 
{
  int i;

  for (i = 0; i < MAX_MIX; i++)
  {
    clearMixTableEnt(i);
  }

  return;
}

// -------------------------------------------------------------------------- //

int setMixTableFile(int audioFileIdx, Sample* sample) 
{
  int openMixIdx = -1;                      // index of an open mix table slot

  if (AUDIO_PLAY_DEBUG) 
  {
    fprintf(stderr, " - audioMix.c: setMixTableFile() - ");
    fprintf(stderr, "Setting audio file table %d to PLAY\n", audioFileIdx);  
  }

  pthread_mutex_lock(&mix_lock);            // lock mix table

  // loop through table, find open table entry
  int i = 0;
  while (i < MAX_MIX) 
  {
    if (mixTable[i].addr == NULL) 
    {
      openMixIdx = i;
      break;
    }    
    i++;
  }

  // check that an entry was found
  if (openMixIdx < 0) 
  {
    fprintf(stderr, " - audioMix.c: setMixTableFile() - ");
    fprintf(stderr, "OUT OF MIX TABLE ENTRIES! NO NEW SAMPLE PLAYBACK ***\n");
    pthread_mutex_unlock(&mix_lock);        // unlock mix table
    return -1;
  }

  mixTable[openMixIdx].addr = (SAMPLE_PTR)audioTable[audioFileIdx].audioAddr;
  mixTable[openMixIdx].file = &audioTable[audioFileIdx];
  mixTable[openMixIdx].nFrames = audioTable[audioFileIdx].audioSizeSamples / FRAME_SIZE;
  mixTable[openMixIdx].lastSampleIdx = audioTable[audioFileIdx].audioSizeSamples - 1;
  mixTable[openMixIdx].s = sample;
  
  if (AUDIO_PLAY_DEBUG) 
  {
    fprintf(stderr, " - audioMix.c: setMixTableFile() - ");
    fprintf(stderr, "NUM SAMPLES = %d\n", audioTable[audioFileIdx].audioSizeSamples);
    fprintf(stderr, " - audioMix.c: setMixTableFile() - ");
    fprintf(stderr, "FRAME SIZE = %d\n", FRAME_SIZE);
    fprintf(stderr, " - audioMix.c: setMixTableFile() - ");
    fprintf(stderr, "%d frames in sample\n", mixTable[openMixIdx].nFrames);
    fprintf(stderr, " - audioMix.c: setMixTableFile() - ");  
    fprintf(stderr, "last sample idx = %d\n", mixTable[openMixIdx].lastSampleIdx);
    if (mixTable[openMixIdx].s != NULL) {
      fprintf(stderr, " - audioMix.c: setMixTableFile() - ");
      fprintf(stderr, "sample id = %d\n", mixTable[openMixIdx].s->id);
    }
  }

  pthread_mutex_unlock(&mix_lock);           // unlock mix table

  return openMixIdx;  // success - return mix table index
}

// -------------------------------------------------------------------------- //

// function to init/zero-out mix table
bool initMixTable() 
{
  clearMixTable();

  // init mix table locks!
  if (pthread_mutex_init(&mix_lock, NULL) < 0) 
  {
    fprintf(stderr, "*** FATAL ERROR: audioMix.c - initMixTable(): ");
    fprintf(stderr, " initializing mix table lock failed! ***\n");
    return FALSE;
  } 
  else if (AUDIO_INIT_DEBUG) 
  {
    fprintf(stderr, "audioMix.c - initMixTable(): ");
    fprintf(stderr, " mix table lock initialized succesfully!\n");
  }
  
  return TRUE; 
}

// -------------------------------------------------------------------------- //
// --------------------------- MIXING FUNCTIONS ----------------------------- //
// -------------------------------------------------------------------------- //

bool calcSumLimits(int mixTableIdx, int bufIdx, 
                   int* sampleIdxLo, int* sampleIdxHi)
{
  // calculate summation indice limits
  *sampleIdxLo = mixTable[mixTableIdx].iterIdx;

  if (mixTable[mixTableIdx].lastSampleIdx < *sampleIdxLo + FRAME_SIZE)
  {
    // last frame containing samples from this mixtable entry source
    *sampleIdxHi = mixTable[mixTableIdx].lastSampleIdx;
    mixTable[mixTableIdx].iterIdx = 0;
    return TRUE;
  }
  else
  {
    *sampleIdxHi = *sampleIdxLo + FRAME_SIZE;
    
    if (FRAME_SIZE < (bufIdx + *sampleIdxHi - *sampleIdxLo)) 
    { 
      // don't overflow the buffer!
      *sampleIdxHi = FRAME_SIZE - bufIdx;
    }

    mixTable[mixTableIdx].iterIdx = *sampleIdxHi;

    return FALSE;
  } 
}

// -------------------------------------------------------------------------- //

int sumBuffer(SAMPLE_TYPE buf[], int mixTableIdx, int bufStartIdx,
              int limitLo, int limitHi) 
{
  int i = mixTableIdx;
  int j = limitLo;
  int k = bufStartIdx;

  // copy all valid samples
  for ( ; j < limitHi; j++, k++) 
  {
    // check for clipping (a BASIC hard-limiter)
    if (buf[k] + mixTable[i].addr[j] > POS_CLIP) 
    {
      if (AUDIO_PLAY_DEBUG) 
      {
        fprintf(stderr, " - audioMix.c: sumBuffer() - ");
        fprintf(stderr, "[%d][%d] POSITIVE CLIP (%d) !\n", i, j, buf[k] + mixTable[i].addr[j]);
      }

      // hard limit positive clip
      buf[k] = POS_CLIP; 
    }
    else if (buf[k] + mixTable[i].addr[j] < NEG_CLIP) 
    {
      if (AUDIO_PLAY_DEBUG) 
      {
        fprintf(stderr, " - audioMix.c: sumBuffer() - ");
        fprintf(stderr, "[%d][%d] NEGATIVE CLIP (%d) !\n", i, j, buf[k] + mixTable[i].addr[j]);
      }

      // hard limit negative clip
      buf[k] = NEG_CLIP;
    }
    else 
    {
      buf[k] += mixTable[i].addr[j];
    }
  }

  return k;
}

// -------------------------------------------------------------------------- //

void lastFrameLoop(SAMPLE_TYPE buf[], int mixIdx, int bufIdx, 
                   int* sampleIdxLo, int* sampleIdxHi)
{
  if (AUDIO_PLAY_DEBUG)
  {
    fprintf(stderr, " - audioMix.c: lastFrameLoop() - ");
    fprintf(stderr, "REITERATING LOOP!\n");
    fprintf(stderr, " - audioMix.c: lastFrameLoop() - ");
    fprintf(stderr, "calculating sum limits...\n");
  }

  // recalculate summmation indices
  calcSumLimits(mixIdx, bufIdx, sampleIdxLo, sampleIdxHi);

  if (AUDIO_PLAY_DEBUG)
  {
    fprintf(stderr, " - audioMix.c: lastFrameLoop() - ");
    fprintf(stderr, "limits: %d - %d\n", *sampleIdxLo, *sampleIdxHi);
  }

  // sum remaining buffer
  sumBuffer(buf, mixIdx, bufIdx, *sampleIdxLo, *sampleIdxHi);

  return;
}

// -------------------------------------------------------------------------- //

void lastFrameEnd(int mixTableIdx)
{
  if (AUDIO_PLAY_DEBUG)
  {
    fprintf(stderr, " - audioMix.c: lastFrameEnd() - ");
    fprintf(stderr, "SAMPLE ENDING!\n");
  }

  // set sample to NOT playing
  mixTable[mixTableIdx].s->playbackState = STOPPED;

  // boot from mix table
  clearMixTableEnt(mixTableIdx);

  return;
}

// -------------------------------------------------------------------------- //

void lastFrameStop(int mixTableIdx)
{
  if (AUDIO_PLAY_DEBUG)
  {
    fprintf(stderr, " - audioMix.c: lastFrameStop() - ");
    fprintf(stderr, "SAMPLE TOLD TO STOP!\n");
  }

  // if last iter, check if sample is to be marked as STOPPED
  if (mixTable[mixTableIdx].sampleStop)
  {
    mixTable[mixTableIdx].s->playbackState = STOPPED;
  }

  // boot from mix table
  clearMixTableEnt(mixTableIdx);

  return;
}

// -------------------------------------------------------------------------- //

// combine all samples from each item in sound table
int mixBuffer(SAMPLE_TYPE buf[]) 
{
  int sampleIdxHi = 0;
  int sampleIdxLo = 0;
  int bufIdx = 0;
  bool lastSampleFrame = FALSE; 
  int i = 0;
    
  pthread_mutex_lock(&mix_lock); 

  // mix all available slots in mix table
  for (i = 0; i < MAX_MIX; i++) 
  {
    // check if entry has sample
    if (mixTable[i].addr == NULL)
      continue;       

    // calculate summation limits
    lastSampleFrame = calcSumLimits(i, 0, &sampleIdxLo, &sampleIdxHi);

    // sum samples from audio file into playback buffer
    bufIdx = sumBuffer(buf, i, 0, sampleIdxLo, sampleIdxHi);

    if (lastSampleFrame && mixTable[i].s->loop) 
    {
      // loop the audio sample
      lastFrameLoop(buf, i, bufIdx, &sampleIdxLo, &sampleIdxHi);
    }
    else if (lastSampleFrame) 
    {
      lastFrameEnd(i);
    }
    else if (mixTable[i].blackSpot) 
    {
      lastFrameStop(i);
    }
  }

  pthread_mutex_unlock(&mix_lock);

  return 1; // success
}
