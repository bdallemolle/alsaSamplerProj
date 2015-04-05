audioFile: audioFile.c audio.h
	gcc -c audioFile.c -lasound -lpthread

audio: audio.c audio.h audioFile
	gcc -c audio.c -lasound -lpthread

control: control.c control.h 
	gcc -c control.c -lasound -lpthread

main: main.c audio.h control.h audioFile audio control
	gcc audio.o audioFile.o control.o main.c -o main -lasound -lpthread

clean:
	rm *.o