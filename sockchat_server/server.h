#ifndef SOCKCHAT_SERVER_H
#define SOCKCHAT_SERVER_H

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define LISTENPORT "3690"
#define BACKLOG 10
#define BUFSIZE 1024

// translation an ip from network to readable form
void *get_in_addr(struct sockaddr *sa);

// sending and recieving
void get_sch(int sockfd, int in_sock);
void send_sch(int sockfd, int out_sock, char *buf, int bufsize);

// inits
void hintsInit(struct addrinfo *hints, size_t hintssize);
int servInit(struct addrinfo *serv, int *sockfd);

#endif
