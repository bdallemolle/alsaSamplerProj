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

/* AUDIO DEVICE VARIABLES */
char output_dev[MAX_NAME];                    // name of output device
snd_pcm_t* output_handle = NULL;       // output device handle (ALSA)
char input_dev[MAX_NAME];                     // name of input device
snd_pcm_t* input_handle = NULL;        // input device handle (ALSA)

/* CONCURRANT CONTROL VARIABLES */
pthread_t PLAYBACK_THREAD = -1;               // playback thread id 
unsigned int EXIT_PLAYER = 0;          // playback loop exit toggle

// -------------------------------------------------------------------------- //
// ------------------------ AUDIO TABLE  FUNCTIONS -------------------------- //
// -------------------------------------------------------------------------- //

// clears/zero-outs an entry in the mix table
void clearAudioTableEnt(int i) 
{
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

// -------------------------------------------------------------------------- //

// clears/zero-outs an entry in the mix table
void clearAudioTable() 
{
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, " - Clearing the audio table.\n");

  int i;
  for (i = 0; i < MAX_AUDIO_FILES; i++)
    clearAudioTableEnt(i);
  return;
}

/* -------------------------------------------------------------------------- */

// function to init/zero-out audio table
int initAudioTable() 
{
  int i;
  for (i = 0; i < MAX_AUDIO_FILES; i++)
    audioTable[i].fd = -1;
  clearAudioTable();
  return 1; 
}

/*

// -------------------------------------------------------------------------- //

int initOutputDevice() 
{
  int err;

  // open ALSA output
  if ((err = snd_pcm_open(&output_handle, output_dev, SND_PCM_STREAM_PLAYBACK, 0)) < 0) 
  {
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
             1000))                             // latency (500 microseconds)
             < 0) 
  {
    fprintf(stderr, "*** ERROR: set parameters error(%s)\n", snd_strerror(err));
    return -1;
  }
  
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, " - parameters set for output device\n");

  return 1;
}

/* -------------------------------------------------------------------------- */

// main playback loop
int playbackLoop() 
{
  SAMPLE_TYPE buf[FRAME_SIZE];
  snd_pcm_sframes_t frames;

  // debugging
  if (AUDIO_PLAY_DEBUG)
    fprintf(stderr, "\n*** STARTING AUDIO PLAYBACK LOOP! *** \n");
  
  // loop until "killed" by main thread
  int isDone = 0;
  while (!isDone) 
  {
    // zero buffer --- memset()
    memset(buf, 0, FRAME_SIZE*sizeof(SAMPLE_TYPE));

    // mix buffer
    mixBuffer(buf);            

    // write frames to audio device
    frames = snd_pcm_writei(output_handle, buf, FRAME_SAMP);
    if (frames < 0) 
    {
      frames = snd_pcm_recover(output_handle, frames, 0);
    }
    if (frames < 0) 
    {
      fprintf(stderr, "*** ERROR: snd_pcm_write failed, exit playback loop... ***\n");
      return -1;
    }

    // check exit conditions
    pthread_mutex_lock(&exit_lock);

    if (EXIT_PLAYER) 
    {
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
void* playbackLaunch() 
{
  if (playbackLoop() < 0) 
  {
    fprintf(stderr, "*** ERROR: PLAYBACK LOOP FAILURE! ***\n" );
    // do behaviors based on error codes
  }
  if (AUDIO_PLAY_DEBUG) 
  {
    fprintf(stderr, " - Playback thread exiting...\n");
  }

  pthread_exit(NULL);
}

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

  if (AUDIO_INIT_DEBUG) 
    fprintf(stderr, "\n*** --------- OPENING AUDIO FILES --------- ***\n");

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

  if (AUDIO_INIT_DEBUG) 
    fprintf(stderr, "\n*** ------ DONE OPENING AUDIO FILES ------ ***\n");

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
