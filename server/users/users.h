#ifndef USERS_H
#define USERS_H

#include <stdlib.h>
#include <poll.h>
#include <string.h>

struct udb {
  char username[50];
  int fd;
  struct udb *next;
};

void udb_add(struct udb **head, char username[], int fd);
void udb_del(struct udb *head, int fd);
struct udb *udb_search(struct udb *head, char username[]);
void udb_free(struct udb *head);

#endif
