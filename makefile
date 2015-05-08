audioFile: audioFile.c sampler.h audio.h 
	gcc -c audioFile.c -lasound -lpthread

audio: audio.c sampler.h audio.h audioFile
	gcc -c audio.c -lasound -lpthread

config: config.c sampler.h config.h
	gcc -c config.c

control: control.c sampler.h control.h 
	gcc -c control.c -lpthread

device: device.c raspiGPIO.c sampler.h device.h raspiGPIO.h
	gcc -c raspiGPIO.c
	gcc -c device.c

behavior: behavior.c sampler.h behavior.h 
	gcc -c behavior.c

events: event.c sampler.h event.h 
	gcc -c event.c

main: main.c sampler.h audioFile audio config control device behavior events
	gcc audio.o audioFile.o config.o control.o device.o behavior.o raspiGPIO.o event.o main.c -o main -lasound -lpthread

clean:
	rm *.o

all: clean main


