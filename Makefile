all: file_server directory_server client

file_server: file_server.c file_server.h
	gcc -g -Wall file_server.c -o file_server

directory_server: directory_server.c directory_server.h
	gcc -g -Wall directory_server.c -o directory_server

client: client.c client.h
	gcc -g client.c -o client

clean:
	rm -f *.o file_server directory_server client directory.txt procs
