#include "server.h"

// TODO: make the proper protocol

servstate srvst;

int main(void)
{
  servStateInit();

  // initializing the server
  printf("Starting the server...\n");
  if (servInit() == -1) {
    fprintf(stderr, "server: error while initializing the server\n");
    return -1;
  }
  printf("Listening on port %s\n", LISTENPORT);

  // initializing server's listener 
  srvst.pfds[0].fd = srvst.sockfd;
  srvst.pfds[0].events = POLLIN;
  srvst.fd_count = 1;

  while (1) {
    int poll_count = poll(srvst.pfds, srvst.fd_count, -1);

    if (poll_count == -1) {
      perror("poll");
      exit(1);
    }

    for (int i = 0; i < srvst.fd_count; i++) {
      if (srvst.pfds[i].revents & POLLIN) {
        if (srvst.pfds[i].fd == srvst.pfds[0].fd) {
          SCS_connection();
        } else {
          SCS_recv(i);
        }
      }
    }
  }
  return 0;
}

void servStateInit()
{
  srvst.fd_count = 0;
  srvst.fd_size = FDCOUNTINITSIZE;
  srvst.pfds = malloc(sizeof(*srvst.pfds) * srvst.fd_size);
  srvst.head = NULL;
}

void waittokill(int signum)
{
  UNUSED(signum);
  wait(NULL);
}

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
    hints->ai_flags = AI_PASSIVE;
}

int servInit() 
{
  int status;
  struct addrinfo serv;
  struct addrinfo *servinfo, *p;
  int yes = 1;

  hintsInit(&serv, sizeof serv);

  if ((status = getaddrinfo(NULL, LISTENPORT, &serv, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: error: %s\n", gai_strerror(status));
    return -1;
  }

  for (p = servinfo; p; p = p->ai_next) {
    if ((srvst.sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("socket");
      continue;
    }

    if (setsockopt(srvst.sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      return -1;
    }

    if (bind(srvst.sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      perror("bind");
      close(srvst.sockfd);
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo);

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    return -1;
  }

  if (listen(srvst.sockfd, BACKLOG) == -1) {
      perror("listen");
      return -1;
  }

  return 0;
}

void SCS_connection() 
{
  struct sockaddr_storage out_addr;
  socklen_t out_addr_size = sizeof out_addr;
  int out_sockfd = accept(srvst.pfds[0].fd,
    (struct sockaddr *)&out_addr,
    &out_addr_size);

  if (out_sockfd == -1) {
    perror("accept");
  } else {
    add_to_pfds(&srvst.pfds, out_sockfd, &srvst.fd_count, &srvst.fd_size);

    char s[BUFSIZE];
    inet_ntop(out_addr.ss_family,
    get_in_addr((struct sockaddr *)&out_addr),
    s, sizeof s);
    printf("server: got connection from %s\n", s);
  }
}

void SCS_sendall(char buf[], int i, int recv_bytes) 
{
  for (int j = 0; j < srvst.fd_count; j++) {
    int dest_fd = srvst.pfds[j].fd;

    if (dest_fd != srvst.pfds[0].fd && srvst.pfds[j].fd != srvst.pfds[i].fd) {
      printf("server: sending '%s' to all the hosts\n", buf);
      if (send(dest_fd, buf, recv_bytes, 0) == -1) {
        perror("send");
      }
    }
  }
}

void SCS_recv(int i) 
{
  char buf[BUFSIZE];
  int recv_bytes = recv(srvst.pfds[i].fd, buf, BUFSIZE, 0);

  if (recv_bytes <= 0) {
    if (recv_bytes == 0) {
      printf("server: connection %d was closed by the client\n", srvst.pfds[i].fd);
    } else {
      perror("recv");
    }
    close(srvst.pfds[i].fd);
    del_from_pfds(srvst.pfds, i, &srvst.fd_count);
  } else {
    // adding a user to the users database
    char *usr = strtok(buf, ":");
    if (strcmp(usr, "SOCKCHATUSERNAME") == 0) {
      usr = strtok(NULL, ":");
      udb_add(&srvst.head, usr, srvst.pfds[i].fd);
      printf("server: registering user %d as '%s'\n", srvst.pfds[i].fd, usr);
    } else {
      // TODO: redo the sending process

      // sending the message to all the hosts
      // SCS_sendall(pfds, *fd_count, buf, i, recv_bytes);
      
      SCS_sendto(buf);
    }
  }
}

void SCS_sendto(char buf[]) {
  char *msg = strtok(buf, "\n");
  char *recp = strtok(NULL, "\n");

  struct udb *user = udb_search(srvst.head, recp);

  if (user == NULL) {
    fprintf(stderr, "error: no user with name %s!\n", recp);
    return;
  }

  if (send(user->fd, msg, strlen(msg), 0) == -1) {
    perror("send");
  }
  printf("server: sending '%s' to %s\n", msg, recp);
}
