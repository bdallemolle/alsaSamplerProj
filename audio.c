// c-libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>
// alsa-libs
#include <alsa/asoundlib.h>
// sampler proj headers
#include "sampler.h"
#include "config.h"
#include "audio.h"
#include "audioFile.h"

/* AUDIO DEVICE VARIABLES */
char output_dev[MAX_NAME];                    // name of output device
snd_pcm_t* output_handle = NULL;              // output device handle (ALSA)
char input_dev[MAX_NAME];                     // name of input device
snd_pcm_t* input_handle = NULL;               // input device handle (ALSA)

/* CONCURRANT CONTROL VARIABLES */
pthread_t PLAYBACK_THREAD = -1;               // playback thread id 
pthread_mutex_t mix_lock;                     // mix table lock
pthread_mutex_t exit_lock;                    // exit (playback thread) toggle lock
unsigned int EXIT_PLAYER;                     // playback loop exit toggle

/* -------------------------------------------------------------------------- */

// clears/zero-outs an entry in the mix table
void clearMixTableEnt(int i) {
   mixTable[i].addr = NULL;
   mixTable[i].file = NULL;
   mixTable[i].idx = 0;
   mixTable[i].lastIdx = 0;
   mixTable[i].nFrames = 0;
   mixTable[i].lastSubFrame = 0;
   mixTable[i].lastSubSampleIdx = 0;
   mixTable[i].blackSpot = 0;
   mixTable[i].s = NULL;
   return;
}

/* -------------------------------------------------------------------------- */

// clears/zero-outs an entry in the mix table
void clearMixTable() {
  int i;
  for (i = 0; i < MAX_MIX; i++)
    clearMixTableEnt(i);
   return;
}

/* -------------------------------------------------------------------------- */

// function to init/zero-out mix table
int initMixTable() {
  clearMixTable();

  // init mix table locks!
  if (pthread_mutex_init(&mix_lock, NULL) < 0) {
    fprintf(stderr, "*** Error initializing mix table lock ***\n");
    return -1;
  } else if (AUDIO_INIT_DEBUG) {
    fprintf(stderr, "mix table lock initialized succesfully...\n");
  }
  
  return 1;	// success
}

/* -------------------------------------------------------------------------- */

int setMixTableFile(int audioFileIdx, Sample* sample) {
  int openMixIdx = -1;

  if (AUDIO_PLAY_DEBUG) 
    fprintf(stderr, "Setting audio file table %d to PLAY\n", audioFileIdx);  

  // lock stuff would be here
  pthread_mutex_lock(&mix_lock);

  // loop through table, find open table entry
  int i = 0;
  while (i < MAX_MIX) {
    if (mixTable[i].addr == NULL) {
      // valid entry
      openMixIdx = i;
      break;
    }    
    i++;
  }

  // check that an entry was found
  if (openMixIdx < 0) {
    fprintf(stderr, "*** ERROR: OUT OF MIX TABLE ENTRIES! NO NEW SAMPLE PLAYBACK ***\n");
    pthread_mutex_unlock(&mix_lock);
    return -1;
  }

  mixTable[openMixIdx].addr = (SAMPLE_PTR) audioTable[audioFileIdx].audioAddr;
  mixTable[openMixIdx].file = &audioTable[audioFileIdx];
  mixTable[openMixIdx].nFrames = audioTable[audioFileIdx].audioSizeSamples / FRAME_SIZE;
  mixTable[openMixIdx].lastIdx = mixTable[openMixIdx].nFrames; 
  mixTable[openMixIdx].lastSubSampleIdx = (audioTable[audioFileIdx].audioSizeSamples 
                                          - (mixTable[openMixIdx].nFrames * FRAME_SIZE));
  mixTable[openMixIdx].s = sample;
  
  // print debugging
  fprintf(stderr, "%d last frame sub sample idx\n", mixTable[openMixIdx].lastSubSampleIdx);
  fprintf(stderr, "NUM SAMPLES = %d, FRAME SIZE = %d\n", audioTable[audioFileIdx].audioSizeSamples, FRAME_SIZE);
  fprintf(stderr, "%d frames in sample\n", mixTable[openMixIdx].nFrames);  
  fprintf(stderr, "audio table size samples = %d\n", audioTable[audioFileIdx].audioSizeSamples);
  fprintf(stderr, "%d last frame idx\n", mixTable[openMixIdx].lastIdx);
  if (mixTable[openMixIdx].s != NULL)
    fprintf(stderr, "sample id = %d", mixTable[openMixIdx].s->id);

  // release mix table lock
  pthread_mutex_unlock(&mix_lock);

  return openMixIdx;	// success
}
 
/* -------------------------------------------------------------------------- */

// clears/zero-outs an entry in the mix table
void clearAudioTableEnt(int i) {
  if (audioTable[i].fd >= 0) {
    close(audioTable[i].fd);
    audioTable[i].fd = -1; 
  }                 
  strcpy(audioTable[i].filename, "empty");
  strcpy(audioTable[i].type, "unknown"); 
  audioTable[i].addr = NULL;                               
  audioTable[i].audioAddr = NULL;     
  audioTable[i].nChannels = 0;        
  audioTable[i].bitDepth = 0;                                           
  audioTable[i].audioSizeSamples = 0;
  audioTable[i].fileSizeBytes = 0;                  
  return;
}

/* -------------------------------------------------------------------------- */

// clears/zero-outs an entry in the mix table
void clearAudioTable() {
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, " - Clearing the audio table.\n");

  int i;
  for (i = 0; i < MAX_AUDIO_FILES; i++)
    clearAudioTableEnt(i);
  return;
}

/* -------------------------------------------------------------------------- */

// clears/zero-outs an entry in the mix table
void clearSampleTableEnt(int i) {
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

/* -------------------------------------------------------------------------- */

// clears/zero-outs an entry in the mix table
void clearSampleTable() {
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, " - Clearing the SAMPLE table.\n");

  int i;
  for (i = 0; i < MAX_SAMPLE; i++)
    clearSampleTableEnt(i);
  return;
}

/* -------------------------------------------------------------------------- */

// function to init/zero-out audio table
int initAudioTable() {
  int i;
  for (i = 0; i < MAX_AUDIO_FILES; i++)
    audioTable[i].fd = -1;
  clearAudioTable();
  return 1; 
}

/* -------------------------------------------------------------------------- */

// combine all samples from each item in sound table
int mix(SAMPLE_TYPE buf[]) {
  int i, j, k; 
  int sampleLimitHi;
  int sampleLimitLo;
  int lastIdx;  
   
  // LOCK MIX TABLE  
  pthread_mutex_lock(&mix_lock); 

  // mix all available slots in mix table
  for (i = 0; i < MAX_MIX; i++) {
    // check if entry has sample
    if (mixTable[i].addr == NULL)
      continue;       

    lastIdx = 0;
    // check if final frame of audio file
    if (mixTable[i].idx == mixTable[i].lastIdx) {
      sampleLimitLo = FRAME_SIZE * mixTable[i].lastIdx;
      sampleLimitHi = sampleLimitLo + mixTable[i].lastSubSampleIdx;
      lastIdx = 1;
    }
    else { 
      sampleLimitHi = FRAME_SIZE * (mixTable[i].idx + 1);
      sampleLimitLo = FRAME_SIZE * mixTable[i].idx;
    }

    // copy all valid samples
    for (j = sampleLimitLo, k = 0; j < sampleLimitHi; j++, k++) {
      // check for clipping (a SUPER basic hard-limiter)
      if (buf[k] + mixTable[i].addr[j] > POS_CLIP) {
        if (AUDIO_PLAY_DEBUG)
          fprintf(stderr, "[%d][%d] POSITIVE CLIP (%d) !\n", i, j, buf[k] + mixTable[i].addr[j]);
        buf[k] = POS_CLIP; 
      }
      else if (buf[k] + mixTable[i].addr[j] < NEG_CLIP) {
        if (AUDIO_PLAY_DEBUG) 
          fprintf(stderr, "[%d][%d] NEGATIVE CLIP (%d) !\n", i, j, buf[k] + mixTable[i].addr[j]);
        buf[k] = NEG_CLIP;
      }
      else buf[k] += mixTable[i].addr[j];
    }

    if (lastIdx && mixTable[i].s->loop) {
      mixTable[i].idx = 0;
      i--;
    }
    else if (lastIdx) {
      // set sample to NOT playing
      mixTable[i].s->playbackState = STOPPED;
      clearMixTableEnt(i);
    }
    else if (mixTable[i].blackSpot) {
      mixTable[i].s->playbackState = STOPPED;
      clearMixTableEnt(i);
    }
    else {
      // otherwise update idx pointer for loop
      mixTable[i].idx++;
    }
  }

  // UNLOCK MIX TABLE
  pthread_mutex_unlock(&mix_lock);

  return 1;	// success
}

/* -------------------------------------------------------------------------- */

int initOutputDevice() {
  int err;

  // open ALSA output
  if ((err = snd_pcm_open(&output_handle, output_dev, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    fprintf(stderr, "cannot open audio device %s (%s)\n", output_dev, snd_strerror(err));
    return -1;
  }
  
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, "*** SOUND DEVICE '%s' OPEN ***\n", output_dev);

  // set ALSA parameters
  if ((err = snd_pcm_set_params(output_handle,  // playback handle
             SND_PCM_FORMAT_S16_LE,             // signed 16-bit little endian pcm
             SND_PCM_ACCESS_RW_INTERLEAVED,     // read/write interleaved stream
             NUM_CHAN,                          // number of channels
             SAMPLE_RATE,                       // sampler rate
             1,                                 // soft resample (?)
             500))                              // latency (500 microseconds)
             < 0) {
    fprintf(stderr, "*** ERROR: set parameters error(%s)\n", snd_strerror(err));
    return -1;
  }
  
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, "parameters set for output device\n");

  return 1;
}

/* -------------------------------------------------------------------------- */

// main playback loop
int playbackLoop() {
  SAMPLE_TYPE buf[FRAME_SIZE];
  snd_pcm_sframes_t frames;

  // debugging
  if (AUDIO_PLAY_DEBUG)
    fprintf(stderr, "\n*** STARTING AUDIO PLAYBACK LOOP! *** \n");
  
  // loop until "killed" by main thread
  int isDone = 0;
  while (!isDone) {
    // zero buffer --- memset()
    memset(buf, 0, FRAME_SIZE*sizeof(SAMPLE_TYPE));

    // mix buffer
    mix(buf);            

    // write frames to audio device
    frames = snd_pcm_writei(output_handle, buf, FRAME_SAMP);
    if (frames < 0) {
      frames = snd_pcm_recover(output_handle, frames, 0);
    }
    if (frames < 0) {
      fprintf(stderr, "*** ERROR: snd_pcm_write failed, exit playback loop... ***\n");
      return -1;
    }

    // check exit conditions
    pthread_mutex_lock(&exit_lock);
    if (EXIT_PLAYER) {
      if (AUDIO_PLAY_DEBUG)
        fprintf(stderr, "*** PLAYBACK LOOP STOPPED ***\n");
      isDone = 1;
    }
    pthread_mutex_unlock(&exit_lock);
  }

  return 1;
}

/* -------------------------------------------------------------------------- */

// wrapper for audio playback loop
void* playbackLaunch() {
  if (playbackLoop() < 0) {
    fprintf(stderr, "*** ERROR: PLAYBACK LOOP FAILURE! ***\n" );
    // do behaviors based on error codes
  }
  if (AUDIO_PLAY_DEBUG)
    fprintf(stderr, " - Playback thread exiting...\n");
  pthread_exit(NULL);
}

/******************************************************************************
 *                            INTERFACED FUNCTIONS                            *
 ******************************************************************************/

// initializes audio player data structures and sound devices for playback
int initAudio(CONFIG* c) {
  if (AUDIO_PLAY_DEBUG) 
    fprintf(stderr, " - Initializing audio!\n");

  // set global variables
  strcpy(output_dev, c->audioOutputDevice);

  // init audio output device
  if (initOutputDevice() < 0) {
    fprintf(stderr, "*** ERROR INITIALIZING AUDIO OUTPUT DEVICE ***\n");
    return -1;
  }

  /* thinking ahead a bit here...

  // init audio input device
  if (initInputDevice() < 0) {
   fprintf(stderr, "*** ERROR INITIALIZING AUDIO INPUT DEVICE ***\n");
   return -1;
  }

  */

  // init mix table
  if (initMixTable() < 0) {
    fprintf(stderr, "*** ERROR INITIALIZING MIX TABLE ***\n");
    return -1;
  }

  // init audio table
  if (initAudioTable() < 0) {
    fprintf(stderr, "*** ERROR INITIALIZING AUDIO TABLE ***\n");
    return -1;
  }

  /************* set up audio table and mix table! *************/

  // open audio files
  setAudioTable(c->audioFiles, c->numAudioFiles);

  // set sample table/mapping (need config arguments)
  setSampleTable(c->audioFiles, c->sampleMap);

  return 1;
}

/* -------------------------------------------------------------------------- */

int startAudio() {
  if (AUDIO_PLAY_DEBUG) 
    fprintf(stderr, " - Launching playback loop thread\n");

  // launch audio player thread
  if (pthread_create(&PLAYBACK_THREAD, NULL, playbackLaunch, NULL) < 0) {
    fprintf(stderr, "*** ERROR: launching playback thread failed! ***\n");
    return -1;
  }
  return 1;
}

/* -------------------------------------------------------------------------- */

// function to initiliaze audio file table (opens files)
int setAudioTable(char filenames[MAX_AUDIO_FILES][MAX_NAME], int nFiles) {
  int i;

  // check valid number of sample files
  if (nFiles > MAX_AUDIO_FILES) {
    fprintf(stderr, "*** ERROR: Trying to open too many audio files. ***\n");
    return -1;
  }

  // open all files
  for (i = 0; i < nFiles; i++) {
    addAudioFile(filenames[i], audioTable, i);        // change this to audio table + index
    if (AUDIO_INIT_DEBUG)
      printAudioFileInfo(i);
  }
  
  // debugging
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, " - %d files opened\n", i);

  // set extra audioTable file entries to blank
  if (i < MAX_AUDIO_FILES) {
    if (AUDIO_INIT_DEBUG) {
      fprintf(stderr, " - Zeroing out remaining entries\n");
    }
    while (i < MAX_AUDIO_FILES) {
      clearAudioTableEnt(i);
      i++;
    }
  }  

  return 1; 
}

/* -------------------------------------------------------------------------- */

// needs a configuration arguement
int setSampleTable(char filenames[MAX_AUDIO_FILES][MAX_NAME], int sampleMap[]) {
  int i = 0;
  fprintf(stderr, "*** SETTING SAMPLE TABLE! ***\n");
  clearSampleTable();

  for (i = 0; i < MAX_SAMPLE; i++) {
    if (sampleMap[i] >= 0) {
      if (AUDIO_INIT_DEBUG)
        fprintf(stderr, " - audio.c: setting sample %d to audio file %d\n", i, sampleMap[i]);
      sampleTable[i].audioIdx = sampleMap[i];                       
    }
  }

  return 1;
}                     

/* -------------------------------------------------------------------------- */

int exitAudio() {
  if (AUDIO_PLAY_DEBUG)
    fprintf(stderr, " - CLOSING AUDIO DEVICE\n");

  if (snd_pcm_close(output_handle) < 0) {
    fprintf(stderr, "*** ERROR: CLOSING AUDIO DEVICE FAILED! ***\n");
  }
  return 1;
}

/* -------------------------------------------------------------------------- */

int killAudio() {
  if (AUDIO_PLAY_DEBUG) 
    fprintf(stderr, " - KILL AUDIO SIGNALLED\n");

  // set endloop toggle...
  pthread_mutex_lock(&exit_lock);
  EXIT_PLAYER = 1;
  pthread_mutex_unlock(&exit_lock);

  // thread join playback thread with current thread
  if (pthread_join(PLAYBACK_THREAD, NULL) < 0) {
    fprintf(stderr, "*** error playback joining thread! ***\n");
    return -1;
  }

  // clear mix table
  clearMixTable();
  
  return 1;
}

/* -------------------------------------------------------------------------- */

int sampleStart(int sampleID) {
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

  return 1;
}

/* -------------------------------------------------------------------------- */

int sampleStop(int sampleID) {
  if (AUDIO_PLAY_DEBUG)
    fprintf(stderr, " - Sample %d told to STOP\n", sampleID);

  pthread_mutex_lock(&mix_lock);
  mixTable[sampleTable[sampleID].mixIdx].blackSpot = 1;
  pthread_mutex_unlock(&mix_lock);

  return 1;
}

/* -------------------------------------------------------------------------- */

int sampleRestart(int sampleID) {
  if (AUDIO_PLAY_DEBUG)
    fprintf(stderr, " - Sample %d told to RESTART\n", sampleID);

  if (sampleTable[sampleID].playbackState == STOPPED) {
    sampleStart(sampleID);
  }
  else {
    sampleStop(sampleID);
    sampleTable[sampleID].mixIdx = setMixTableFile(sampleTable[sampleID].audioIdx, &sampleTable[sampleID]);
    sampleTable[sampleID].playbackState = PLAYING;
  }

  return 1;
}

/* -------------------------------------------------------------------------- */

int sampleStartLoop(int sampleID) {
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

/* -------------------------------------------------------------------------- */

int sampleOverlay(int sampleID) {
  if (AUDIO_PLAY_DEBUG)
    fprintf(stderr, " - Sample %d told to play OVERLAID!\n");
  // get file num for sample ID

  // set mix table playback

  return 1;
}

/* -------------------------------------------------------------------------- */

int sampleStopALL() {
  if (AUDIO_PLAY_DEBUG) 
    fprintf(stderr, " - ALL samples told to STOP");
  return 1;
}

/* -------------------------------------------------------------------------- */

int setPlaybackSound(int idx) {
  return setMixTableFile(idx, NULL);        // success
}
