/* Vectors (auto-extending arrays)
   Copyright (c) 1999-2006 Reuben Thomas.  All rights reserved.

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

#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

/* Unitialised vector elements are zeroed */
typedef struct {
  size_t itemsize;        /* size of each item in bytes */
  size_t items;           /* number of items used */
  size_t size;            /* number of items available */
  void *array;            /* the array of contents */
} vector;

vector *vec_new(size_t itemsize);
void *vec_index(vector *v, size_t idx);
void vec_shrink(vector *v, size_t idx, size_t items);
vector *vec_copy(vector *v);

#define vec_item(v, idx, ty) \
  (*(ty *)vec_index((v), (idx)))
#define vec_itemsize(v) \
  (v)->itemsize
#define vec_items(v) \
  (v)->items
#define vec_array(v) \
  (v)->array
#define vec_offset(v, obj, ty) \
  (size_t)((obj - (ty *)((v)->array)))

#endif
