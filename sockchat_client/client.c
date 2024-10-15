#include "client.h"

int main(int argc, char *argv[])
{
  int sockfd, numbytes;  
  struct addrinfo hints;
  char buf[MAXDATASIZE];


  if (argc < 2 || argc > 3) {
      fprintf(stderr,"usage: client hostname [message-to-send]\n");
      exit(1);
  }

  if (argc == 3) 
    strcpy(buf, argv[2]);

  hintsInit(&hints, sizeof hints);
  sockchatConnect(&sockfd, &hints, argv[1]);

  struct pollfd serverpoll;
  struct pollfd stdinpoll;

  // server
  serverpoll.fd = sockfd;
  serverpoll.events = POLLIN | POLLOUT;

  // stdin
  stdinpoll.fd = 0;
  stdinpoll.events = POLLIN;

  while (1) {
    int isServerPoll;
    int isStdinPoll;

    isServerPoll = poll(&serverpoll, 1, -1);

    if (isServerPoll == -1) {
      perror("poll");
    }

    if (serverpoll.revents & POLLIN) {
      numbytes = recv(serverpoll.fd, buf, MAXDATASIZE, 0);
      if (numbytes == -1) {
        perror("recv");
      } else if (numbytes == 0) {
        fprintf(stderr, "client: server has closed the connection\n");
        exit(1);
      } else {
        printf("client: recieved from server '%s'\n", buf);
      }
    } else {
      printf("enter the message: ");

      isStdinPoll = poll(&stdinpoll, 1, 2000);
      if (isStdinPoll == 0) {
        printf("TIME OUT\n");
      } else {
        if (read(0, buf, MAXDATASIZE) == 0) {
          fprintf(stderr, "client: error reading input\n");
        } else {
          printf("client: sending '%s'\n", buf);
          if (send(serverpoll.fd, buf, MAXDATASIZE, 0) == -1) {
            perror("send");
          }
        }
      }
    }
  }

  return 0;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) 
        return &(((struct sockaddr_in*)sa)->sin_addr);

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void hintsInit(struct addrinfo *hints, size_t hintssize)
{
    memset(hints, 0, hintssize);
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_STREAM;
}

void sockchatConnect(int *sockfd, struct addrinfo *hints, char *remote)
{
    int status;
    struct addrinfo *servinfo, *p;
    char s[INET6_ADDRSTRLEN];

    if ((status = getaddrinfo(remote, PORT, hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((*sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(*sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo);
}


