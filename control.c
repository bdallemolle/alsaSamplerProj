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
#include "sampler.h"
#include "audio.h"
// #include "gpio.h"

char* testfile = "gtrsample.wav";

int run() {
  char buf[MAX_NAME];
  int i = 0;

	fprintf(stderr, "*** RUN CALLED! DO \"CONTROL\" STUFF ***\n");

  // for each setting...

	// open audio files
  setAudioTable(&testfile, 1);

  // a bad hack for avoiding initial overruns
  sleep(2);

  // play audio
  startAudio();

  // play the testing sound
  setPlaybackSound(0);

  // temporary block until input fron standard in
  fgets(buf, MAX_NAME, stdin);

  // cleanup
  killAudio();                   // kills playback loop/thread
	clearAudioTable();             // clears audio table

	return 1;
}