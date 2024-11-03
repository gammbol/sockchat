#ifndef PFDS_H
#define PFDS_H

#include <stdlib.h>
#include <poll.h>

void add_to_pfds(struct pollfd *pfds[], int fd, int *fd_count, int *fd_size);
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);

#endif
