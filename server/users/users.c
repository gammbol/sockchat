#include "users.h"

void udb_add(struct udb **head, char username[], int fd) {
  struct udb *new = calloc(1, sizeof(struct udb));
  strcpy(new->username, username);
  new->fd = fd;

  if (!(*head)) {
    *head = new;
    return;
  }

  struct udb *p = *head;
  for(; p->next; p = p->next);
  p->next = new;
}

void udb_del(struct udb *head, int fd)
{
  struct udb *p = head;
  struct udb *prev;
  for(; p->fd != fd; p = p->next)
    prev = p;
  prev->next = p->next;
  free(p);
}

struct udb *udb_search(struct udb *head, char username[])
{
  struct udb *p = head;
  for(; p; p = p->next)
    if (strcmp(p->username, username) == 0)
      return p;
  return NULL;
}

void udb_free(struct udb *head)
{
  struct udb *p = head->next;
  struct udb *prev = head;
  for(; p; p = p->next) {
    free(prev);
    prev = p;
  }
  free(prev);
}


