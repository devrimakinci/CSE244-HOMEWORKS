all: exe

exe: main.o
	gcc main.o -o exe

main.o :main.c
	gcc -c -pedantic-errors -ansi main.c
	
clean:
	rm *.o exe
