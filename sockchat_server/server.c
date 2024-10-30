#include "server.h"

int main(void)
{
  // server socket 
  int sockfd;

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


  while (1) {
    int poll_count = poll(pfds, fd_count, -1);

    if (poll_count == -1) {
      perror("poll");
      exit(1);
    }

    for (int i = 0; i < fd_count; i++) {
      if (pfds[i].revents & POLLIN) {
        if (pfds[i].fd == sockfd) {
          SCS_connection(pfds, &fd_count, &fd_size);
        } else {
          SCS_recv(pfds, &fd_count, i);
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

void SCS_connection(struct pollfd pfds[], int *fd_count, int *fd_size) {
  struct sockaddr_storage out_addr;
  socklen_t out_addr_size = sizeof out_addr;
  int out_sockfd = accept(pfds[0].fd,
    (struct sockaddr *)&out_addr,
    &out_addr_size);

  if (out_sockfd == -1) {
    perror("accept");
  } else {
    add_to_pfds(&pfds, out_sockfd, fd_count, fd_size);

    char s[BUFSIZE];
    inet_ntop(out_addr.ss_family,
    get_in_addr((struct sockaddr *)&out_addr),
    s, sizeof s);
    printf("server: got connection from %s\n", s);
  }
}

void SCS_sendall(struct pollfd pfds[], int fd_count, char buf[], int i, int recv_bytes) {
  for (int j = 0; j < fd_count; j++) {
    int dest_fd = pfds[j].fd;

    if (dest_fd != pfds[0].fd && pfds[j].fd != pfds[i].fd) {
      printf("server: sending '%s' to all the hosts\n", buf);
      if (send(dest_fd, buf, recv_bytes, 0) == -1) {
        perror("send");
      }
    }
  }
}

void SCS_recv(struct pollfd pfds[], int *fd_count, int i) {
  char buf[BUFSIZE];
  int recv_bytes = recv(pfds[i].fd, buf, BUFSIZE, 0);

  if (recv_bytes <= 0) {
    if (recv_bytes == 0) {
      printf("server: connection %d was closed by the client\n", pfds[i].fd);
    } else {
      perror("recv");
    }
    close(pfds[i].fd);
    del_from_pfds(pfds, i, fd_count);
  } else {
    SCS_sendall(pfds, *fd_count, buf, i, recv_bytes);
  }
}
