cc= gcc
cflags= -Wall -Wextra
server= sockchat_server/server.c sockchat_server/pfds/pfds.c
client= sockchat_client/client.c

all: server client

server: $(server)
	$(cc) $(cflags) $(server) -o server

client: $(client)
	$(cc) $(cflags) $(client) -o client

clean:
	rm -rf server client
