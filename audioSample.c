

/* -------------------------------------------------------------------------- */

int sampleStart(int sampleID) {
  if (AUDIO_PLAY_DEBUG)
    fprintf(stderr, " - Sample %d told to START\n", sampleID);

  if (sampleID < 0) 
    return -1;

  // don't start sample if already playing
  if (sampleTable[sampleID].playbackState == PLAYING) {
    if (AUDIO_PLAY_DEBUG)
      fprintf(stderr, " - Sample %d already playing, can't start again!\n", sampleID);

    return 1;
  }

  sampleTable[sampleID].mixIdx = setMixTableFile(sampleTable[sampleID].audioIdx, &sampleTable[sampleID]);
  sampleTable[sampleID].playbackState = PLAYING;

  return 1;
}

/* -------------------------------------------------------------------------- */

int sampleStop(int sampleID) {
  if (AUDIO_PLAY_DEBUG)
    fprintf(stderr, " - Sample %d told to STOP\n", sampleID);

  pthread_mutex_lock(&mix_lock);
  mixTable[sampleTable[sampleID].mixIdx].blackSpot = 1;
  mixTable[sampleTable[sampleID].mixIdx].sampleStop = 1;
  pthread_mutex_unlock(&mix_lock);

  return 1;
}

/* -------------------------------------------------------------------------- */

int sampleRestart(int sampleID) {
  if (AUDIO_PLAY_DEBUG)
    fprintf(stderr, " - Sample %d told to RESTART\n", sampleID);

  if (sampleTable[sampleID].playbackState == STOPPED) {
    sampleStart(sampleID);
  }
  else {
    // stop currently playing sample
    pthread_mutex_lock(&mix_lock);
    mixTable[sampleTable[sampleID].mixIdx].blackSpot = 1;
    mixTable[sampleTable[sampleID].mixIdx].sampleStop = 0;
    pthread_mutex_unlock(&mix_lock);
    // set new sample to start
    sampleTable[sampleID].mixIdx = setMixTableFile(sampleTable[sampleID].audioIdx, &sampleTable[sampleID]);
    sampleTable[sampleID].playbackState = PLAYING;
  }

  return 1;
}

/* -------------------------------------------------------------------------- */

int sampleStartLoop(int sampleID) {
  if (AUDIO_PLAY_DEBUG)
    fprintf(stderr, " - Sample %d told to START\n", sampleID);

  if (sampleID < 0) 
    return -1;

  // don't start sample if already playing
  if (sampleTable[sampleID].playbackState == PLAYING) {
    if (AUDIO_PLAY_DEBUG)
      fprintf(stderr, " - Sample %d already playing, can't start again!\n", sampleID);

    return 1;
  }

  sampleTable[sampleID].mixIdx = setMixTableFile(sampleTable[sampleID].audioIdx, &sampleTable[sampleID]);
  sampleTable[sampleID].playbackState = PLAYING;
  sampleTable[sampleID].loop = TRUE;

  return 1;
}

/* -------------------------------------------------------------------------- */

int sampleOverlay(int sampleID) {
  if (AUDIO_PLAY_DEBUG)
    fprintf(stderr, " - Sample %d told to play OVERLAID!\n", sampleID);
  // get file num for sample ID

  // set mix table playback

  return 1;
}

/* -------------------------------------------------------------------------- */

int sampleStopALL() {
  if (AUDIO_PLAY_DEBUG) 
    fprintf(stderr, " - ALL samples told to STOP");
  return 1;
}

/* -------------------------------------------------------------------------- */

int setPlaybackSound(int idx) {
  return setMixTableFile(idx, NULL);        // success
}