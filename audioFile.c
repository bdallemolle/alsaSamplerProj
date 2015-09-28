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
#include "audio.h"

/* -------------------------------------------------------------------------- */

// helper function for debugging
void printAudioFileInfo(int tableIdx) {
  if (MAX_AUDIO_FILES <= tableIdx) {
    fprintf(stderr, "*** ERROR: audio file table index error (print) ***\n");
    return;
  }
  fprintf(stderr, "------------------------------\n");
  fprintf(stderr, "FILE INFO, tblIdx: %d\n", tableIdx);
  fprintf(stderr, "filename: %s\n", audioTable[tableIdx].filename);
  fprintf(stderr, "nChannels: %d\n", audioTable[tableIdx].nChannels);
  fprintf(stderr, "files size (bytes): %d\n", audioTable[tableIdx].fileSizeBytes);
  fprintf(stderr, "audio size (samples): %d\n", audioTable[tableIdx].audioSizeSamples);
  fprintf(stderr, "------------------------------\n");
  return;
}

/* -------------------------------------------------------------------------- */

// function to read WAV/RIFF file header
int readWAV(AudioFile* a) {
  char buf[WAV_OFFSET];		// buffer for header 
  int audioDataSize = 0;

  // debugging
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, "*** READING WAV FILE HEADER ***\n");

  // copy header
  memcpy(buf, a->addr, WAV_OFFSET); 
  
  // output wav info
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, "1-4: %4.4s\n", buf);                  

  // check for RIFF file
  if (strncmp(buf, "RIFF", 4) == 0) {
    if (AUDIO_INIT_DEBUG)
      fprintf(stderr, " - Valid RIFF file\n");
  }
  else {
    fprintf(stderr, "*** ERROR: NOT A RIFF FILE ***\n");
    return -1;
  }

  // output file size (minus 8 bytes?)
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, "5-8: %d\n", *((int*)(buf+4)));       

  // output file header (should be WAVE)
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, "9-12: %4.4s\n", (buf+8));

  // check for WAVE file                
  if (strncmp(buf+8, "WAVE", 4) == 0) {
    if (AUDIO_INIT_DEBUG)
      fprintf(stderr, " - Valid WAVE file\n");
  }
  else {
    fprintf(stderr, "*** ERROR: NOT A WAVE FILE ***\n");
    return -1;
  }

  // "fmt"
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, "13-16: %4.4s\n", (buf+12));             

  // length of format data as listed above (bitdepth)
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, "17-20: %d\n", *((int*)(buf+16)));        

  // check that it is 16 (later, or 24)
  if (*((int*)(buf+16)) == BIT_DEPTH) {
    if (AUDIO_INIT_DEBUG)
      fprintf(stderr, " - Valid bit depth (16)\n");
    a->bitDepth = *((int*)(buf+16));
  }
  else {
    fprintf(stderr, "*** ERROR: INVALID BIT DEPTH ***\n");
    return -1;
  }

  // type of format 
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, "21-22: %d\n", *((short*)(buf+20)));      

  // should be 1 (for PCM)?

  // number of channels
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, "22-24: %d\n", *((short*)(buf+22))); 

  // check that it is 2! 
  if (*((short*)(buf+22)) == NUM_CHAN) {
    if (AUDIO_INIT_DEBUG)
      fprintf(stderr, " - Valid number of channels (2)\n");
    a->nChannels = *((short*)(buf+22));
  }
  else {
    fprintf(stderr, "*** ERROR: INVALID NUM CHANNELS ***\n");
    return -1;
  }           
  
  // size of audio data (minus 44 apparently too...)
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, "(AUDIO DATA SIZE?) 41-44: %d\n", *((int*)(buf+40)));

  // store audio file length
  audioDataSize = *((int*)(buf+40)) - WAV_OFFSET;
  a->audioSizeSamples = audioDataSize / (sizeof(SAMPLE_TYPE));  

  return 1;
}

/* -------------------------------------------------------------------------- */

int addAudioFile(char* filename, AudioFile audioTable[], int i) {
  struct stat fileStat;

  if (AUDIO_INIT_DEBUG) {
    fprintf(stderr, "*** Opening %s ***\n", filename);
  }  

  // open file
  audioTable[i].fd = open(filename, O_RDONLY);
  if (audioTable[i].fd == -1) {
    fprintf(stderr, "*** ERROR: cannot open file %s ***\n", filename);
    return -1;
  }    
    
  // get file size
  if (fstat(audioTable[i].fd, &fileStat) == -1) {
    fprintf(stderr, "*** ERROR: stat can't get file info ***\n");
    // close file
    close(audioTable[i].fd);
    return -1;
  } 

  // store file size
  audioTable[i].fileSizeBytes = fileStat.st_size;

  // debugging
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, "mmaping %s, size %d bytes\n", filename, audioTable[i].fileSizeBytes);

  // mmap file
  audioTable[i].addr = mmap(NULL, audioTable[i].fileSizeBytes,
                            PROT_READ, MAP_PRIVATE, 
                            audioTable[i].fd, 0);
  if (audioTable[i].addr == MAP_FAILED) {
    fprintf(stderr, "*** ERROR: mmapping %s failed ***\n", filename);
    // reset data in audio file entry
    close(audioTable[i].fd);
    audioTable[i].fileSizeBytes = 0;
    audioTable[i].addr = NULL;
    return -1;
  } 

  // start of audio data (...hardcoded kinda)
  audioTable[i].audioAddr = audioTable[i].addr + WAV_OFFSET; 

  // copy filename 
  strcpy(audioTable[i].filename, filename);                                            
  
  // get WAV format file info
  if (readWAV(&audioTable[i]) < 0) {
    // open .wav failed, reset data in audio file entry
    close(audioTable[i].fd);
    strcpy(audioTable[i].filename, "empty");
    strcpy(audioTable[i].type, "unknown"); 
    audioTable[i].fileSizeBytes = 0;
    audioTable[i].addr = NULL;                               
    audioTable[i].audioAddr = NULL;     
    audioTable[i].nChannels = 0;        
    audioTable[i].bitDepth = 0;                                           
    audioTable[i].audioSizeSamples = 0;
    audioTable[i].fileSizeBytes = 0;     
    return -1;
  }

  return 1;
}