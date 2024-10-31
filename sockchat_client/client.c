#include "client.h"

int main(int argc, char *argv[])
{
  int sockfd, numbytes;  
  struct addrinfo hints;
  char sendto[MAXDATASIZE];
  char msg[MAXDATASIZE];
  int isServerPoll;
  int bufread;

  if (argc != 3) {
      fprintf(stderr,"usage: client hostname username\n");
      exit(1);
  }

  sprintf(msg, "SOCKCHATUSERNAME:%s", argv[2]);

  hintsInit(&hints, sizeof hints);
  sockchatConnect(&sockfd, &hints, argv[1]);

  if (send(sockfd, msg, MAXDATASIZE, 0) == -1) {
    perror("send");
    exit(1);
  } 

  struct pollfd serverpoll;
  struct pollfd stdinpoll;

  // server
  serverpoll.fd = sockfd;
  serverpoll.events = POLLIN | POLLOUT;

  // stdin
  stdinpoll.fd = 0;
  stdinpoll.events = POLLIN;

  while (1) {
    memset(msg, 0, MAXDATASIZE);

    isServerPoll = poll(&serverpoll, 1, -1);

    if (isServerPoll == -1) {
      perror("poll");
    }

    if (serverpoll.revents & POLLIN) {
      numbytes = recv(serverpoll.fd, msg, MAXDATASIZE, 0);
      if (numbytes == -1) {
        perror("recv");
      } else if (numbytes == 0) {
        fprintf(stderr, "client: server has closed the connection\n");
        exit(1);
      } else {
        printf("client: recieved from server '%s'\n", msg);
      }
    } else {
<<<<<<< HEAD
      printf("enter the message(%s): ", argv[2]);
=======
      // entering the message
      printf("enter the message: ");
>>>>>>> 646bb1c (refactor client for dialog dev)
      fflush(stdout);
      if (SCC_poll_stdin(&stdinpoll, msg) != 0)
        continue;

      // entering the recipient
      printf("recipient: ");
      fflush(stdout);
      if (SCC_poll_stdin(&stdinpoll, sendto) != 0)
        continue;

      size_t msglen = strlen(msg);
      size_t sendtolen = strlen(sendto);
      if (strlen(msg) == 0 || strlen(sendto) == 0) {
        fprintf(stderr, "error: message or recipient are empty!");
        continue;
      } else {
        if ((msglen + sendtolen + 1) >= MAXDATASIZE) {
          size_t offset = MAXDATASIZE - (msglen + sendtolen) + 1;
          msg[msglen-offset] = '\0';
        }
        strcat(msg, "\n");
        strcat(msg, sendto);
      }
      
      printf("client: sending '%s'\n", msg);
      if (send(serverpoll.fd, msg, MAXDATASIZE, 0) == -1) {
        perror("send");
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

int SCC_poll_stdin(struct pollfd *stdinpoll, char *buf)
{
  int isStdinPoll;
  int bufread;

  isStdinPoll = poll(stdinpoll, 1, 10000);
  if (isStdinPoll == 0) {
    printf("TIME OUT\n");
    return -1;
  } else {
    bufread = read(0, buf, MAXDATASIZE);
    buf[bufread-1] = '\0';
    if (bufread == 0) {
      fprintf(stderr, "client: error reading input\n");
      return 1;
    } 
  }
  return 0;
}
