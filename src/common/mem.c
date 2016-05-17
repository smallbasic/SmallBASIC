// This file is part of SmallBASIC
//
// Simple and Virtual Memory Manager Unix/Palm
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/pmem.h"
#include "common/smbas.h"
#include "common/messages.h"

void tmplist_init(tmplist_t *lst) {
  lst->head = lst->tail = NULL;
  lst->count = 0;
}

void tmplist_clear(tmplist_t *lst) {
  tmpnode_t *cur, *pre;

  cur = lst->head;
  while (cur) {
    pre = cur;
    cur = cur->next;
    free(pre->data);
    free(pre);
  }

  tmplist_init(lst);
}

tmpnode_t *tmplist_add(tmplist_t *lst, void *data, int size) {
  tmpnode_t *np = (tmpnode_t *) malloc(sizeof(tmpnode_t));
  np->data = malloc(size);
  memcpy(np->data, data, size);
  np->next = NULL;

  if (lst->head) {
    (lst->tail->next = np, lst->tail = np);
  } else {
    lst->head = lst->tail = np;
  }

  lst->count++;
  return np;
}

