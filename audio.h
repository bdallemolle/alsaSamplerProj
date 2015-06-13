// Macros
#define PLAYING         1                           // macro for "is playing" boolean behevior
#define STOPPED         0                           // macro for "is stopped" boolean behevior
#define WAV_OFFSET      44                          // size (bytes) of .wav file header 

/* EVENTUALLY OBSOLETE MACROS - USER PARAMS AND ALSA MIXING SHOULD TAKE CARE OF THIS */
#define FRAME_SAMP      256                         // number of multi-channel samples
#define NUM_CHAN        2                           // number of channels of playback
#define FRAME_SIZE      (FRAME_SAMP * NUM_CHAN)     // total size (in samples) of a playback frame
#define SAMPLE_RATE     44100                       // 44.1k sample rate (default)
#define BIT_DEPTH       16                          // playback bit depth
#define POS_CLIP        32676                       // max value of 16-bit PCM  
#define NEG_CLIP        -32768                      // min value of 16-bit PCM 
typedef short SAMPLE_TYPE;                          // size of 16-bit PCM audio
typedef short* SAMPLE_PTR;                          // pointer to 16-bit PCM audio data

// audio locking mechanisms
pthread_mutex_t mix_lock;                     // mix table lock - PUT INSIDE audioMix.c
pthread_mutex_t exit_lock;                    // exit (playback thread) toggle lock - PUT INSIDE audioControl.c

// An audio file
typedef struct {
  int fd;			                        // file descriptor
  char filename[MAX_NAME];            // for debugging
  char type[MAX_NAME];                // type of audio file (WAV/RIFF/etc...)
  void* addr;			                    // pointer to audio file                     
  void* audioAddr;                    // pointer to start of data segment of file (for uncompressed PCM audio)
  int nChannels;		                  // only stereo at the moment
  int bitDepth;                       // bit depth
  int audioSizeSamples;		            // size of the audio data in sample slices
  int fileSizeBytes;                  // size of audio data in bytes
} AudioFile;

// the abstract concept of a samples
typedef struct {
  int id;                             // sample id
  int audioIdx;                       // index into audio table
  int mixIdx;                         // index into mix table
  int playbackState;                  // playing/stopped state
  int overlay;                        // toggle for overlayed sample
  int numOverlays;                    // number of overlaid copies of sample playing
  bool loop;                          // toggle if the sample is on loop or not
  unsigned int loopCount;             // number of times this sample has looped
  unsigned int numLoops;              // number of times this sample should loop
  void (*digitalBehavior)(int);       // digital behavior function associated with sample (callback)
  // void (*analogBehavior)(double);  // analog behavior associated with sample (callback)
} Sample;

/* AudioLink object
 * - links to a playable audio file for mixing and playback
 * - keeps info about playback indices 
 * - keeps track of playback states (stop/loop/etc)
 */
typedef struct {
  SAMPLE_PTR addr;                    // pointer to audio in memory
  AudioFile* file;                    // pointer to parent file structure
  int iterIdx;                        // current position in an audio file iterator
  int nFrames;                        // number of frames
  int lastSampleIdx;                  // the index of the last sample of the audio file
  int blackSpot;                      // marked for death (stop)
  int sampleStop;                     // tell sample to say its stopped
  Sample* s;                          // sample corresponding with a playback sound
} AudioLink;

AudioFile audioTable[MAX_AUDIO_FILES];                            // the audio file table
AudioLink mixTable[MAX_MIX];                                      // the mix table
Sample sampleTable[MAX_SAMPLE];                                   // the sample table 

// Initialization functions
int initAudio(CONFIG* c);                                         // init audio device
int startAudio();                                                 // starts playback loop/thread
int setAudioTable(char filenames[MAX_AUDIO_FILES][MAX_NAME],      // sets the audio table with provided audio files
                  int nFiles);                                                                       

// Exit functions 
int exitAudio();                                                  // exits audio device
int killAudio();                                                  // kills playback loop/thread
void clearAudioTable();                                           // clears audio table

// sample functions (audioSample.c)
int setSampleTable(char filenames[MAX_AUDIO_FILES][MAX_NAME],     // set the sample table mapping
                   int sampleMap[MAX_SAMPLE]);  
int sampleStart(int sampleID);                                    // starts a sample
int sampleStop(int sampleID);                                     // stops a sample
int sampleRestart(int sampleID);                                  // restarts a sample
int sampleStartLoop(int sampleID);                                // starts a sample loop
int sampleOverlay(int sampleID);                                  // plays a sample over itself (mix table limit providing...)
int sampleStopALL();                                              // stops all sample playback

// audio file functions (audioFile.c)
void printAudioFileInfo(int tableIdx);
int addAudioFile(char* filename, AudioFile audioTable[], int i);

// mixing functions (audioMix.c)
bool initMixTable();
int setMixTableFile(int audioFileIdx, Sample* sample);
int mixBuffer(SAMPLE_TYPE buf[]);

/*** INELEGANT WAY TO SET FILES FOR PLAYBACK... ***/
int setPlaybackSound(int idx);            // sets an audio table indexed file for playback in mixer directly

