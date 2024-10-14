#include "server.h"

int main(void)
{
  // server socket 
  int sockfd;


  // remote socket
  int out_sockfd;
  struct sockaddr_storage out_addr;
  socklen_t out_addr_size;

  // initializing the server
  printf("Starting the server...\n");
  if ((sockfd = servInit()) == -1) {
    fprintf(stderr, "server: error while initializing the server\n");
    return -1;
  }
  printf("Listening on port %s\n", LISTENPORT);

  int fd_count = 0;
  int fd_size = FDCOUNTINITSIZE;
  struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

  // add the listener
  pfds[0].fd = sockfd;
  pfds[0].events = POLLIN;
  fd_count = 1;


  char s[BUFSIZE];
  char buf[BUFSIZE];
  while (1) {
    int poll_count = poll(pfds, fd_count, -1);

    if (poll_count == -1) {
      perror("poll");
      exit(1);
    }

    for (int i = 0; i < fd_count; i++) {
      if (pfds[i].revents & POLLIN) {
        if (pfds[i].fd == sockfd) {
          out_addr_size = sizeof out_addr;
          out_sockfd = accept(sockfd,
            (struct sockaddr *)&out_addr,
            &out_addr_size);

          if (out_sockfd == -1) {
            perror("accept");
          } else {
            add_to_pfds(&pfds, out_sockfd, &fd_count, &fd_size);

            inet_ntop(out_addr.ss_family,
            get_in_addr((struct sockaddr *)&out_addr),
            s, sizeof s);
            printf("server: got connection from %s\n", s);
          }
        } else {
          int recv_bytes = recv(pfds[i].fd, buf, BUFSIZE, 0);

          if (recv_bytes <= 0) {
            if (recv_bytes == 0) {
              printf("server: connection %d was closed by the client\n", pfds[i].fd);
            } else {
              perror("recv");
            }
            close(pfds[i].fd);
            del_from_pfds(pfds, i, &fd_count);
          } else {
            for (int j = 0; j < fd_count; j++) {
              int dest_fd = pfds[j].fd;

              if (dest_fd != sockfd && pfds[j].fd != pfds[i].fd) {
                printf("server sending '%s' to all the hosts\n", buf);
                if (send(dest_fd, buf, recv_bytes, 0) == -1) {
                  perror("send");
                }
              }
            }
          }
        }
      }
    }
  }
  return 0;
}

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
  }
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
  }
  signal(SIGCHLD, waittokill);
}

void hintsInit(struct addrinfo *hints, size_t hintssize) {
    memset(hints, 0, hintssize);
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_flags = AI_PASSIVE;
}

int servInit() {
  int status, sockfd;
  struct addrinfo serv;
  struct addrinfo *servinfo, *p;
  int yes = 1;

  hintsInit(&serv, sizeof serv);

  if ((status = getaddrinfo(NULL, LISTENPORT, &serv, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: error: %s\n", gai_strerror(status));
    return -1;
  }

  for (p = servinfo; p; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      return -1;
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
      return -1;
  }

  return sockfd;
}

void add_to_pfds(struct pollfd *pfds[], int fd, int *fd_count, int *fd_size) {
  if (*fd_count == *fd_size) {
    *fd_size *= 2;
    *pfds = realloc(*pfds, (*fd_size) * sizeof(**pfds));
  }

  (*pfds)[*fd_count].fd = fd;
  (*pfds)[*fd_count].events = POLLIN;
  (*fd_count)++;
}

void del_from_pfds(struct pollfd pfds[], int i, int *fd_count) {
  pfds[i] = pfds[*fd_count - 1];
  (*fd_count)--;
}

