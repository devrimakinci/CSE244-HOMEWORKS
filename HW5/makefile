all: grepSh

grepSh: main.o
	gcc -o grepSh main.o -lpthread 

main.o :main.c
	gcc -c main.c
	
clean:
	rm *.o grepSh
