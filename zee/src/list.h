/* Circular doubly-linked lists/queues
   Copyright (c) 1997-2007 Reuben Thomas.
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
#include <stdint.h>
#include <stdbool.h>


typedef int list;

list list_new(void);
void list_free(list l);
size_t list_length(list l);
list list_append(list l, void *i);
list list_set_string(list l, size_t n, const char *s);
const char *list_get_string(int l, size_t n);
const void *list_behead(list l);
const char *list_behead_string(list l);
const void *list_betail(list l);
#define list_empty(l) (list_length(l) == 0)

#endif
