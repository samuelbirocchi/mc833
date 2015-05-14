all: client server

server: aux_functions.o
	gcc -g aux_functions.o server_udp.c -o server
client: aux_functions.o
	gcc -g client_udp.c -o client
aux_functions.o:
	gcc -c aux_functions.c
clean:
	rm -rf *.o makefile~ client server
