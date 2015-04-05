// Macros
#define MAX_FILES     4			                      // Max sound files used in sampler (*** FOR NOW ***)
#define MAX_MIX   	  16			                    // Max sounds that can be mixed at ones (*** FOR NOW ***)
#define MAX_NAME  	  80                          // Max filename size
#define MAX_SAMPLE    8                           // Max number of samples you can use

// EVENTUALLY OBSOLETE MACROS - USER PARAMS AND ALSA MIXING SHOULD TAKE CARE OF THIS
#define FRAME_SAMP      256                         // Number of multi-channel samples
#define NUM_CHAN        2                           // Number of channels of playback
#define FRAME_SIZE      (FRAME_SAMP * NUM_CHAN)     // Total size (in samples) of a playback frame
#define SAMPLE_RATE     44100                       // 44.1k sample rate (default)
#define WAV_OFFSET      44                          // Size (bytes) of .wav file header
#define BIT_DEPTH       16                          // Playback bit depth
#define POS_CLIP        32676                       // Max value of 16-bit PCM  
#define NEG_CLIP        -32768                      // Min value of 16-bit PCM 
#define OUTPUT_DEV 	    "sysdefault:CARD=CODEC" 		// playback device name
typedef short SAMPLE_TYPE;
typedef short* SAMPLE_PTR;

// debugging macros
#define AUDIO_INIT_DEBUG	   1		    // initialization debugging
#define AUDIO_PLAY_DEBUG     1        // playback debugging

// An audio file
typedef struct {
  int fd;			                        // file descriptor
  char filename[MAX_NAME];            // for debugging
  char type[MAX_NAME];                // type of audio file (WAV/RIFF/etc...)
  void* addr;			                    // pointer to audio file                     
  void* audioAddr;                    // pointer to audio data in file
  int nChannels;		                  // only stereo at the moment
  int bitDepth;                       // bit depth                                 {this will be obselete eventually}
  int audioSizeSamples;		            // size of the audio data in sample slices
  int fileSizeBytes;                  // size of audio data in bytes
} AudioFile;

typedef struct {
  int id;
} Sample;

typedef struct {
  SAMPLE_PTR addr;                    // pointer to audio in memory
  AudioFile* file;                    // pointer to parent file structure
  int idx;                            // current copy over index
  int lastIdx;                        // last index of addr*
  int nFrames;                        // number of frames
  int lastSubFrame;                   // number 
  int lastSubSampleIdx;               // index of last sub sample (sub frame)
  int blackSpot;                      // marked for death (stop)
} AudioLink;


AudioFile audioTable[MAX_FILES];          // The audio file table
AudioLink mixTable[MAX_MIX];              // The mix table
Sample sampleTable[MAX_SAMPLE];           // the 

// Initialization functions
int initAudio(char* output_dev_name);     // init audio device - EVENTUALLY MORE GENERAL INIT ARGUEMENT
int startAudio();                         // starts playback loop/thread
int setAudioTable(char* filenames[],      // sets the audio table with provided audio files
                  int nFiles);                            
int setSampleTable();                     // set the sample table mapping

// Exit functions 
int exitAudio();                   // exits audio device
int killAudio();                   // kills playback loop/thread
void clearAudioTable();             // clears audio table
int clearSampleTable();            // clears sample table

// Sample functions
int sampleStart(int sampleID);     // starts a sample
int sampleStop(int sampleID);      // stops a sample
int sampleRestart(int sampleID);   // restarts a sample
int sampleOverlay(int sampleID);   // plays a sample over itself (mix table limit providing...)
int sampleStopALL();               // stops all sample playback

/*** ARCHAIC WAY TO SET FILES FOR PLAYBACK... ***/
int setPlaybackSound(int idx);

// file open functions
void printAudioFileInfo(int tableIdx);
int setDefaultAudioFile(char* filename, AudioFile* a);

