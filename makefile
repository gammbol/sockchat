cc= gcc
cflags= -Wall -Wextra
server= server/server.c server/pfds/pfds.c server/users/users.c
client= client/client.c
outputServer= scs
outputClient= scc

all: $(outputServer) $(outputClient)

$(outputServer): $(server)
	$(cc) $(cflags) $(server) -o $(outputServer)

$(outputClient): $(client)
	$(cc) $(cflags) $(client) -o $(outputClient)

clean:
	rm -rf $(outputServer) $(outputClient)
