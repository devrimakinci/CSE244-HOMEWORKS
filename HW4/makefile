all: grepTh

grepTh: main.o
	gcc -o grepTh main.o -lpthread 

main.o :main.c
	gcc -c main.c
	
clean:
	rm *.o grepTh
