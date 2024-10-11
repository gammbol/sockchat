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

void waittokill(int signum)
{
  wait(NULL);
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) 
        return &(((struct sockaddr_in*)sa)->sin_addr);

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void get_sch(int sockfd, int in_sock) {
  char buf[BUFSIZE];

  
  if (!fork()) { // this is the child process
      close(sockfd); // child doesn't need the listener
      if (recv(in_sock, buf, BUFSIZE, 0) == -1)
          perror("recv");
      else
        printf("server: recieved '%s'\n", buf);
      close(in_sock);
      exit(0);
  } else
    signal(SIGCHLD, waittokill);
}

void send_sch(int sockfd, int out_sock, char *buf, int bufsize) {
  if (!fork()) {
    close(sockfd);
    if (send(out_sock, buf, bufsize, 0) == -1) 
      perror("send");
    else
      printf("server: sent message '%s'\n", buf);
    close(out_sock);
    exit(0);
  } else
    signal(SIGCHLD, waittokill);
}

void hintsInit(struct addrinfo *hints, size_t hintssize) {
    memset(&hints, 0, hintssize);
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_flags = AI_PASSIVE;
}

int servInit(struct addrinfo *serv) {
  int status, sockfd;
  struct addrinfo *servinfo, *p;
  int yes = 1;

  if ((status = getaddrinfo(NULL, LISTENPORT, serv, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: error: %s\n", gai_strerror(status));
    exit(1);
  }

  for (p = servinfo; p; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(2);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      perror("bind");
      close(sockfd);
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo);

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    return -1;
  }

  if (listen(sockfd, BACKLOG) == -1) {
      perror("listen");
      exit(4);
  }

  return sockfd;
}

int main(void)
{
  // server socket 
  int sockfd;
  struct addrinfo serv;

  // remote socket
  int out_sockfd;
  struct sockaddr_storage out_addr;
  socklen_t out_addr_size;

  printf("Starting the server...\n");
  hintsInit(&serv, sizeof serv);

  sockfd = servInit(&serv);
  printf("Listening on port %s\n", LISTENPORT);


  char s[BUFSIZE];
  char buf[BUFSIZE] = "sending a test message. hello there!";
  while (1) {
    if ((out_sockfd = accept(sockfd, (struct sockaddr *)&out_addr, &out_addr_size)) == -1) {
      perror("accept");
      continue;
    }

    inet_ntop(out_addr.ss_family,
            get_in_addr((struct sockaddr *)&out_addr),
            s, sizeof s);
    printf("server: got connection from %s\n", s);
  
    get_sch(sockfd, out_sockfd);
    send_sch(sockfd, out_sockfd, buf, BUFSIZE);

    close(out_sockfd);
  }
  return 0;
}
