/* Circular doubly-linked lists/queues
   Copyright (c) 1997-2006 Reuben Thomas.
   All rights reserved.

   This file is part of Zee.

   Zee is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   Zee is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with Zee; see the file COPYING.  If not, write to the Free
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

#include <assert.h>
#include <stdlib.h>

#include "list.h"
#include "zmalloc.h"


/* Create an empty list, returning a pointer to the list */
list list_new(void)
{
  list l = zmalloc(sizeof(struct list_s));

  l->next = l->prev = l;
  l->item = NULL;

  return l;
}

/* Return the length of a list */
size_t list_length(list l)
{
  list p;
  size_t length = 0;

  for (p = l->next; p != l; p = p->next)
    ++length;

  return length;
}

/* Add an item to the head of a list */
list list_prepend(list l, const void *i)
{
  list n = zmalloc(sizeof(struct list_s));

  n->next = l->next;
  n->prev = l;
  n->item = i;
  l->next = l->next->prev = n;

  return l;
}

/* Add an item to the tail of a list */
list list_append(list l, const void *i)
{
  list n = zmalloc(sizeof(struct list_s));

  n->next = l;
  n->prev = l->prev;
  n->item = i;
  l->prev = l->prev->next = n;

  return l;
}

/* Return the first item of a list, or NULL if the list is empty */
const void *list_head(list l)
{
  return l == l->next ? NULL : l->next->item;
}

/* Remove the first item of a list, returning the item, or NULL if the
   list is empty */
const void *list_behead(list l)
{
  const void *i;
  list p = l->next;

  if (p == l)
    return NULL;
  i = p->item;
  l->next = l->next->next;
  l->next->prev = l;

  return i;
}

/* Remove the last item of a list, returning the item, or NULL if the
   list is empty */
const void *list_betail(list l)
{
  const void *i;
  list p = l->prev;

  if (p == l)
    return NULL;
  i = p->item;
  l->prev = l->prev->prev;
  l->prev->next = l;

  return i;
}

/* Return the nth item of l, or l->item (usually NULL) if that is out
   of range */
const void *list_at(list l, size_t n)
{
  size_t i;
  list p;

  assert(l);

  for (p = list_first(l), i = 0; p != l && i < n; p = list_next(p), i++)
    ;

  return p->item;
}

/* Sort list l with qsort using comparison function cmp */
void list_sort(list l, int (*cmp)(const void *p1, const void *p2))
{
  list p;
  const void **vec;
  size_t i, len = list_length(l);

  assert(l && cmp);

  vec = zmalloc(sizeof(void *) * len);

  for (p = list_first(l), i = 0; i < len; p = list_next(p), ++i)
    vec[i] = p->item;

  qsort(vec, len, sizeof(void *), cmp);

  for (p = list_first(l), i = 0; i < len; p = list_next(p), ++i)
    p->item = vec[i];
}
