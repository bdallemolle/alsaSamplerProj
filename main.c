// c-lib includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
// sample-proj headers
#include "sampler.h"
#include "audio.h"
#include "device.h"
#include "control.h"
#include "parse.h"
// #include "config.h"

// debug messages with info on usage
void output_commandline() {
  fprintf(stdout, "RUNNING SAMPLER VERSION 1 WITH FOLLOWING PARAMETERS: \n");
  fprintf(stdout, "FRAME SIZE: %d\n", FRAME_SIZE);
  fprintf(stdout, "SAMPLE RATE: %d\n", SAMPLE_RATE);
  fprintf(stdout, "\n");
  return;
}

// debug messages with info on usage
void output_config() {
  return;
}

// entry point function
int main(int argc, char* argv[]) {
  // parse commandline arguments
  parse_commandline(argc, argv);
  output_commandline();

  // parse config file - NOT YET ESTABLISHED

  // initialize audio
  initAudio(OUTPUT_DEV);        // init the default output device

  // initialize devices
  initDevice(COMMANDLINE);      // init the commandline as a device

  // initialize behaviors

  // initialize events

  // initialize responses

  // start program, call run()
  run();

  // cleanup
  exitAudio();

  return 0;
}
