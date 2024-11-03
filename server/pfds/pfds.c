#include "pfds.h"

// adding to the pfds array
void add_to_pfds(struct pollfd *pfds[], int fd, int *fd_count, int *fd_size) {
  if (*fd_count == *fd_size) {
    *fd_size *= 2;
    *pfds = realloc(*pfds, (*fd_size) * sizeof(**pfds));
  }

  (*pfds)[*fd_count].fd = fd;
  (*pfds)[*fd_count].events = POLLIN;
  (*fd_count)++;
}

// deleting from the pfds array
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count) {
  pfds[i] = pfds[*fd_count - 1];
  (*fd_count)--;
}
