all: listdir

listdir: main.o
	gcc main.o -o listdir

main.o :main.c
	gcc -c -pedantic-errors -ansi main.c
	
clean:
	rm *.o listdir
