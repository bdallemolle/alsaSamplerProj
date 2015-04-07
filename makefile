audioFile: audioFile.c sampler.h audio.h 
	gcc -c audioFile.c -lasound -lpthread

audio: audio.c sampler.h audio.h audioFile
	gcc -c audio.c -lasound -lpthread

control: control.c sampler.h control.h 
	gcc -c control.c -lasound -lpthread

parse: parse.c parse.h sampler.h
	gcc -c parse.c 

main: main.c sampler.h audio.h control.h audioFile audio control parse
	gcc audio.o audioFile.o control.o parse.o main.c -o main -lasound -lpthread

clean:
	rm *.o