// currently supported io device drivers (in progress...)
#define COMMANDLINE		1 					// a basic commandline interface for sampler	- NOT IMPLEMENTED!
#define RASPI_GPIO		2					// raspi gpio device communication
#define RASPI_SERIAL	3					// raspi serial communication abstraction		- NOT IMPLEMENTED!

// a control device structure
typedef struct {
	int id;										// device id
	bool isActive;								// toggle if device is active
	int readPorts[MAX_IO];						// fd's of input ports
	int numReadPorts;							// number of active input ports
	int writePorts[MAX_IO];						// fd's of output ports
	int numWritePorts;							// number of active output
	void (*digitalRead)(int, char*);			// digital read from fd
	void (*digitalWrite)(int, char);			// digital write from fd
} DEVICE;

// the control device (eventually, this would be a list)
DEVICE controlDev;

// function to init a device passed as an argument
int initDevice(CONFIG* c);

