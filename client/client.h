#ifndef SOCKCHAT_CLIENT_H
#define SOCKCHAT_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>

#define PORT "3690" // the port client will be connecting to 
#define MAXDATASIZE 100 // max number of bytes we can get at once 

// translates an ip from network to printable form
void *get_in_addr(struct sockaddr *sa);

// init for hints
void hintsInit(struct addrinfo *hints, size_t hintssize);

// connecting to the remote host
void sockchatConnect(int *sockfd, struct addrinfo *hints, char *remote);

// polling the stdin
int SCC_poll_stdin(struct pollfd *stdinpoll, char *buf);

#endif
