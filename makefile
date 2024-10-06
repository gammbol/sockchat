cc= gcc
cflags= -Wall -Wextra

all: server client

server: server.c
	$(cc) $(cflags) server.c -o server

client: client.c
	$(cc) $(cflags) client.c -o client

clean:
	rm -rf server client
