#include "client.h"

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    struct addrinfo hints;
    char buf[MAXDATASIZE] = "Hi there!";


    if (argc < 2 || argc > 3) {
        fprintf(stderr,"usage: client hostname [message-to-send]\n");
        exit(1);
    }

    if (argc == 3) 
      strcpy(buf, argv[2]);

    hintsInit(&hints, sizeof hints);
    sockchatConnect(&sockfd, &hints, argv[1]);


    if ((numbytes = send(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("send");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: sent '%s'\n", buf);

    while(1);


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


