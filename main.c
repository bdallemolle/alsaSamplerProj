// c-lib includes
#include <stdio.h>
#include <stdlib.h>
// sample-proj headers
#include "sampler.h"
#include "config.h"
#include "audio.h"
#include "behavior.h"
#include "device.h"
#include "control.h"

// entry point function
int main(int argc, char* argv[]) {
  // the program configuration
  CONFIG c;                    

  // parse config file
  config(&c);        

  // initialize audio
  initAudio(&c);

  // initialize devices
  initDevice(&c);

  // initialize behaviors
  initBehavior(&c);

  // initialize events
  initEvents(&c);

  // start program, call run (in control)
  run(&c);

  // cleanup
  exitAudio();
  // other cleanup routines?

  return 0;
}
