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

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) 
        return &(((struct sockaddr_in*)sa)->sin_addr);

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void getmsg(int sockfd) {
  int in_sock;
  struct sockaddr_storage income_socket;
  socklen_t sock_size;

  if ((in_sock = accept(sockfd, (struct sockaddr *)&income_socket, &sock_size)) == -1) {
    perror("accept");
    return;
  }

  char s[BUFSIZE];
  char buf[BUFSIZE];

  inet_ntop(income_socket.ss_family,
          get_in_addr((struct sockaddr *)&income_socket),
          s, sizeof s);
  printf("server: got connection from %s\n", s);

  if (!fork()) { // this is the child process
      close(sockfd); // child doesn't need the listener
      if (recv(in_sock, buf, BUFSIZE, 0) == -1)
          perror("recv");
      else
        printf("server: recieved '%s'\n", buf);
      close(in_sock);
      exit(0);
  }
  close(in_sock);
}

int main(void)
{
  int status, sockfd;
  struct addrinfo serv, *servinfo, *p;

  printf("Starting the server...\n");

  memset(&serv, 0, sizeof hints);
  serv.ai_family = AF_UNSPEC;
  serv.ai_socktype = SOCK_STREAM;
  serv.ai_flags = AI_PASSIVE;


  if ((status = addrinfo(NULL, PORT, &serv, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  for (p = servinfo; p; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 1, sizeof(int)) == -1) {
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
    exit(3);
  }

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(4);
  }

  printf("Listening on port %s\n", PORT);


  char buf[BUFSIZE];
  while (1) {
    getmsg(buf, BUFSIZE);
    // sendmsg(buf, BUFSIZE);
  }
  return 0;
}
