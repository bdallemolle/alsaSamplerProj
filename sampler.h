/**
 * 
 * 
 */

// project-wide macros
#define MAX_NAME 		80		// max filename size
#define MAX_AUDIO_FILES	4		// max sound files used in sampler
#define MAX_MIX   	    16		// max sounds that can be mixed at ones
#define MAX_SAMPLE      8		// max number of samples you can use
#define NUM_CDEV		3		// hardcoded number of avaiable control devices
#define MAX_IO			24		// max number of in/out ports on a device
#define STDIN_FD		0		// file descriptor of standard input

// baseline supported control device drivers (in progress...)
#define COMMANDLINE		0 		// a basic commandline interface for sampler - NOT IMPLEMENTED!
#define RASPI_GPIO		1		// raspi gpio device communication
#define RASPI_SERIAL	2		// raspi serial communication abstraction - NOT IMPLEMENTED!

/*** temporary #defines for demo ***/
#define LIGHT			23
int light_fd;
/*** get rid of these eventually! ***/

// toggle typedef (for digital HI/LO binary reading)
typedef enum {HI = 1, LO = 0} toggle_t;

// boolean typedef 
typedef enum {TRUE = 1, FALSE = 0} bool;

// debugging globals
int AUDIO_INIT_DEBUG;		// debugging messages regarding audio object initialization
int AUDIO_PLAY_DEBUG;		// debugging messages regarding audio playback
int DEVICE_INIT_DEBUG;		// debugging messages for device initialization
int CONFIG_DEBUG;			// debugging messages for configuration and parsing
int GENERAL_DEBUG;			// general debugging messages
int AUDIO_DISABLED;			// disables audio

