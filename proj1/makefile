all: client server_sqlite

server: aux_functions.o
	gcc -g aux_functions.o server.c -o server

server_sqlite:
	gcc -lsqlite3 server_sqlite.c -o server_sqlite

client: aux_functions.o
	gcc -g client.c -o client

aux_functions.o:
	gcc -c aux_functions.c

clean:
	rm -rf *.o makefile~ server client server_sqlite
