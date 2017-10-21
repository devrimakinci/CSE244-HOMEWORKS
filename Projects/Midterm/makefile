all: timerServer seeWhat showResults

timerServer: timerServer.o
	gcc -o timerServer timerServer.o
	
timerServer.o: timerServer.c
	gcc -c timerServer.c
	
seeWhat: seeWhat.o
	gcc -o seeWhat seeWhat.o -lm
	
seeWhat.o: seeWhat.c
	gcc -c seeWhat.c

showResults: showResults.o
	gcc -o showResults showResults.o
	
showResults.o: showResults.c
	gcc -c showResults.c
	
clean:
	rm *.o timerServer seeWhat showResults
