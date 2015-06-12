// the configuration object
typedef struct {
	char audioOutputDevice[MAX_NAME];					// audio output device name
	char audioInputDevice[MAX_NAME];					// audio input device name - NO USE YET
	char audioFiles[MAX_AUDIO_FILES][MAX_NAME];			// list of audio files for program
	int numAudioFiles;									// number of audio files in configuration
	int sampleMap[MAX_SAMPLE];							// maps audio files to their samples
	int controlDevID;									// ID of control device for I/O
	int readMap[MAX_IO];								// maps io 'port' to a read/write functionality
	int behavior[MAX_IO];								// behaviors, mapping to the indices of input control ports
	int event[MAX_IO];									// events, mapping indicies of input controls to events
	int numReadPorts;									// number of IO ports used
	int writeMap[MAX_IO];								// maps io 'port' to a read/write functionality
	int numWritePorts;									// number of IO ports used
} CONFIG;

// interfaced function to fill the configuration object
int config(CONFIG* c);