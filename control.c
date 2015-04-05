// c-lib includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <poll.h>
#include <pthread.h>
#include <ctype.h>
// sample-proj headers
#include "audio.h"
// #include "gpio.h"

char* testfile = "gtrsample.wav";

int run() {
  int i = 0;

	fprintf(stderr, "*** RUN CALLED! DO STUFF ***\n");

	// open audio files
  setAudioTable(&testfile, 1);

  // play audio
  startAudio();

  // try playing a sample
  setPlaybackSound(0);

  // sleep...lol
  while (i < 3) {
    sleep(2);
    fprintf(stderr, ".");
    i++;
  }
  fprintf(stderr, "\n");

  // cleanup
  killAudio();                   // kills playback loop/thread
	clearAudioTable();             // clears audio table

	return 1;
}