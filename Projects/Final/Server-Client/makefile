all: server client

server: server.o 
	gcc server.o -o server -lpthread -lm
	
client: client.o
	gcc client.o -o client -lpthread -lm

server.o :server.c
	gcc -c server.c

client.o :client.c
	gcc -c client.c
	
clean:
	rm *.o server client
