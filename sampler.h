/**
 * 
 * 
 */

// temporary #defines for demo
#define OUTPUT_DEV 	    "sysdefault:CARD=CODEC" 		// playback device name

// boolean typedef
typedef enum {HI = 1, LO = 0} toggle_t;

// debugging macros
int AUDIO_INIT_DEBUG;                 // debugging messages regarding audio object initialization
int AUDIO_PLAY_DEBUG;                 // debugging messages regarding audio playback