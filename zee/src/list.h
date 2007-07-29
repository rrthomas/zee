/* Circular doubly-linked lists/queues
   Copyright (c) 1997-2005 Reuben Thomas.
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

#ifndef LIST_H
#define LIST_H

#include <stddef.h>


typedef struct list_s *list;
struct list_s {
  list prev;
  list next;
  const void *item;
};

list list_new(void);
size_t list_length(list l);
list list_append(list l, const void *i);
const void *list_behead(list l);

#define list_first(l) ((l)->next)
#define list_next(l)  ((l)->next)

#define list_empty(l)  ((l)->next == l)

#endif
