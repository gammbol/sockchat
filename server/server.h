#ifndef SOCKCHAT_SERVER_H
#define SOCKCHAT_SERVER_H

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#include "pfds/pfds.h"
#include "users/users.h"

#define LISTENPORT "3690"
#define BACKLOG 10
#define BUFSIZE 1024
#define FDCOUNTINITSIZE 5

// define for unused values
#define UNUSED(x) (void)(x)

// server state struct
typedef struct servstate {
  int sockfd;           // server socket file desctiptor
  int fd_count;         // fd poll count
  int fd_size;          // fd poll size
  struct pollfd *pfds;  // fd poll structs
  struct udb *head;     // user database struct
} servstate;

// initializing server state struct
void servStateInit();
  

// translation an ip from network to readable form
void *get_in_addr(struct sockaddr *sa);

// sending and recieving
void get_sch(int sockfd, int in_sock);
void send_sch(int sockfd, int out_sock, char *buf, int bufsize);

// inits
void hintsInit(struct addrinfo *hints, size_t hintssize);
int servInit();

// pfds
void add_to_pfds(struct pollfd *pfds[], int fd, int *fd_count, int *fd_size);
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);

// SCSs
void SCS_connection();
void SCS_sendall(char buf[], int i, int recv_bytes);
void SCS_recv(int i);
void SCS_sendto(char buf[]);

#endif
