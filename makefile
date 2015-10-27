# makefile for ALSA SAMPLER PROJECT
# by Bryan Dalle Molle
# 
# NOTE TO SELF: this is getting long and unorderly...
# 		better way to do this with less makefile lines?

CC = gcc
DEBUG = -DAUDIOINITDEBUG -DAUDIOPLAYDEBUG -DEVICEINITDEBUG -DCONFIGDEBUG -DAUDIODISABLED
FLAGS = -g -Wall -lasound -lpthread $(DEBUG)
AUDIO_OBJ = audioMix.o audioSample.o audioFile.o audio.o
OBJ = $(AUDIO_OBJ) behavior.o config.o control.o device.o event.o raspiGPIO.o

# audio components

audio.o: audio.c audio.h 
	$(CC) $(FLAGS) -c audio.c 

audioFile.o: audioFile.c audio.h 
	$(CC) $(FLAGS) -c audioFile.c

audioMix.o: audioMix.c audio.h 
	$(CC) $(FLAGS) -c audioMix.c  

audioSample.o: audioSample.c audio.h 
	$(CC) $(FLAGS) -c audioSample.c 

# control components

config.o: config.c config.h
	$(CC) $(FLAGS) -c config.c

control.o: control.c control.h 
	$(CC) $(FLAGS) -c control.c 

raspiGPIO.o: raspiGPIO.c raspiGPIO.h
	$(CC) $(FLAGS) -c raspiGPIO.c

device.o: device.c device.h
	$(CC) $(FLAGS) -c device.c

behavior.o: behavior.c behavior.h 
	$(CC) $(FLAGS) -c behavior.c

event.o: event.c event.h 
	$(CC) $(FLAGS) -c event.c

main: main.c sampler.h $(OBJ)
	$(CC) $(FLAGS) $(OBJ) main.c -o main 

clean:
	rm -f *.o main

all: main


