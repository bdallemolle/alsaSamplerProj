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
#include "audio.h"

/* -------------------------------------------------------------------------- */

// helper function for debugging
void printAudioFileInfo(int tableIdx) {
  if (MAX_FILES <= tableIdx) {
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
void readWAV(void* fileAddr, int* audioDataSize) {
  char buf[WAV_OFFSET];		// buffer for header  

  fprintf(stderr, "\n*** READING WAV FILE HEADER ***\n");
  // copy header
  memcpy(buf, fileAddr, WAV_OFFSET); 
  fprintf(stderr, "COPY SUCCESSFUL!\n");
  
  // output wav info
  fprintf(stderr, "1-4: %4.4s\n", buf);                   // marks file as RIFF

  // this needs to equal RIFF
  
  fprintf(stderr, "5-8: %d\n", *((int*)(buf+4)));         // size of the overall file (minus 8 bytes?)

  // not sure what to expect there
  
  fprintf(stderr, "9-12: %4.4s\n", (buf+8));              // file type header, always WAVE

  // this needs to equal WAVE
  
  fprintf(stderr, "13-16: %4.4s\n", (buf+12));            // "fmt"

  // should simply be "fmt" + null
  
  fprintf(stderr, "17-20: %d\n", *((int*)(buf+16)));       // length of format data as listed above (bitdepth)

  // bitdepth needs to == 16 for our application

  fprintf(stderr, "21-22: %d\n", *((short*)(buf+20)));      // type of format 

  // should be 1 (for PCM)
  
  fprintf(stderr, "22-24:  %d\n", *((short*)(buf+22)));     // number of channels        
  
  fprintf(stderr, "(AUDIO DATA SIZE?) 41-44: %d\n", *((int*)(buf+40)));
  *audioDataSize = *((int*)(buf+40)) - WAV_OFFSET;

  return;
}

/* -------------------------------------------------------------------------- */

int setDefaultAudioFile(char* filename, AudioFile* a) {
  struct stat fileStat;
  int audioDataSize = 0;

  if (AUDIO_INIT_DEBUG) {
    fprintf(stderr, "*** Opening %s ***\n", filename);
  }  

  // open file
  a->fd = open(filename, O_RDONLY);
  if (a->fd == -1) {
    fprintf(stderr, "*** ERROR: cannot open file %s ***\n", filename);
    return -1;
  }    
    
  // get file size
  if (fstat(a->fd, &fileStat) == -1) {
    fprintf(stderr, "*** ERROR: stat can't get file info ***\n");
    // close file
    close(a->fd);
    return -1;
  } 

  // store file size
  a->fileSizeBytes = fileStat.st_size;

  // debugging
  if (AUDIO_INIT_DEBUG)
    fprintf(stderr, "mmaping %s, size %d bytes\n", filename, a->fileSizeBytes);

  // mmap file
  a->addr = mmap(NULL, a->fileSizeBytes,
                PROT_READ, MAP_PRIVATE, 
                a->fd, 0);
  if (a->addr == MAP_FAILED) {
    fprintf(stderr, "*** ERROR: mmapping %s failed ***\n", filename);
    // reset data in audio file entry
    close(a->fd);
    a->fileSizeBytes = 0;
    a->addr = NULL;
    return -1;
  } 
  
  // get WAV format file info
  readWAV(a->addr, &audioDataSize);

  // store file info in data structure (should really be done in readWAV)
  a->audioSizeSamples = audioDataSize / (sizeof(SAMPLE_TYPE));  // size of audio data
  a->nChannels = NUM_CHAN;                                      // for now...hardcoded...
  a->bitDepth = BIT_DEPTH;                                      // for now...also hardcoded...
  a->audioAddr = a->addr + WAV_OFFSET;                           // start of audio data (...hardcoded kinda)
  strcpy(a->filename, filename);                            // copy filename

  return 1;
}