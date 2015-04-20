// currently supported io device drivers
#define COMMANDLINE		1 					// a basic commandline interface for sampler
#define RASPI_GPIO		2					// raspi gpio device communication
#define RASPI_SERIAL	3					// raspi serial communication abstraction
// other device constants
#define NUM_DEV			3					// hardcoded number of output devices
#define MAX_IO			16					// max number of in/out ports on a device

typedef struct {
	int id;										// device id
	int isActive;								// toggle if device is active
	int ioPorts[MAX_IO];						// fd's of input/output ports
	int numPorts;								// number of active listening ports
	void (*digitalRead)(int, char*);			// digital read from port id
	void (*digitalWrite)(int, char);			// digital write from port id
	// some ideas
	// void (*toggleRead)(int, toggle_t*);
	// void (*toggleWrite)(int, toggle_t);
	// void (*pcmRead)(int, toggle_t*);
	// void (*pcmWrite)(int, toggle_t);
	// void (*analogRead)(int);
	// void (*analogWrite)(int, int); 
} controlDev;

controlDev controlDevices[NUM_DEV];

// function to init a device passed as an argument
void initDevice(int deviceID);