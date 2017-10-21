all: list

list: main.o
	gcc main.o -o list

main.o :main.c
	gcc -c -ansi -pedantic-errors main.c
	
clean:
	rm *.o list
