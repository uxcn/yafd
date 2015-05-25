//===-- list.h - linked list --------------------------------------*- C -*-===//

#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#define container_of(ptr, type, member)                                        \
  (type*)((char*)(ptr) - offsetof(type, member))

#define list_for_each(entry, list)                                             \
  for ((entry) = (list)->head; (entry); (entry) = (entry)->next)

struct list_entry {

  struct list_entry* next;
};

struct list {

  struct list_entry* head;

  struct list_entry* tail;
};

static inline void list_init(struct list* const l) {

  l->head = l->tail = NULL;
}

static inline void list_insert(struct list_entry* const new,
                               struct list_entry* const prev,
                               struct list_entry* const next) {

  prev->next = new;
  new->next = next;
}

static inline struct list_entry* list_remove_head(struct list* const l) {

  struct list_entry* const h = l->head;
  struct list_entry* const t = l->tail;

  if (h == t)
    l-> head = l->tail = NULL;
  else
    l->head = h->next;

  return h;
}

static inline void list_add_head(struct list* const l,
                                 struct list_entry* const e) {

  struct list_entry* const h = l->head;

  e->next = h;

  l->head = e;

  if (!h)
    l->tail = e;
}

static inline void list_add_tail(struct list* const l,
                                 struct list_entry* const e) {

  struct list_entry* const t = l->tail;

  e->next = NULL;

  l->tail = e;

  if (t)
    t->next = e;
  else
    l->head = e;
}

static inline void list_add(struct list* const l, struct list_entry* const e) {

  list_add_tail(l, e);
}

static inline void list_remove(struct list* const l,
                               struct list_entry* const e) {

  struct list_entry* p = l->head;

  if (p == e) {
    l->head = l->tail = e->next = NULL;
    return;
  }

  while (p->next != e)
    p = p->next;

  p->next = e->next;

  if (l->tail == e)
    l->tail = p;

  e->next = NULL;
}

static inline bool is_empty_list(const struct list* const l) {

  return l->head == NULL;
}

static inline bool is_single(const struct list* const l) {

  return (l->head != NULL) && (l->head == l->tail);
}

static inline bool is_head(const struct list* const l, const struct list_entry* const le) {

  return l->head == le;
}

static inline bool is_tail(const struct list* const l, const struct list_entry* const le) {

  return l->tail == le;
}

#endif // LIST_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
