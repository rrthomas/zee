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

#include "config.h"

#include <string.h>
#include <stdint.h>

#include "zmalloc.h"
#include "vector.h"
#include "main.h"

/*
 * Create a vector whose items' size is size
 */
vector *vec_new(size_t itemsize)
{
  vector *v = zmalloc(sizeof(vector));
  vec_itemsize(v) = itemsize;
  vec_items(v) = 0;
  v->size = 0;
  v->array = NULL;
  return v;
}

/*
 * Resize a vector v to items elements
 */
static void resize(vector *v, size_t items)
{
  v->array = zrealloc(v->array, items * vec_itemsize(v));
  v->size = items;
}

/*
 * Return the address of a vector element, growing the array if needed
 */
void *vec_index(vector *v, size_t idx)
{
  if (idx >= v->size)
    resize(v, idx >= v->size * 2 ? idx + 1 : v->size * 2);
  if (idx >= vec_items(v))
    vec_items(v) = idx + 1;
  return (void *)((uint8_t *)v->array + idx * vec_itemsize(v));
}

/*
 * Shrink a vector v at index idx by items items
 */
void vec_shrink(vector *v, size_t idx, size_t items)
{
  if (idx >= v->size)
    return;
  if (idx + items > v->size)
    items = v->size - idx;
  memcpy((uint8_t *)v->array + idx * vec_itemsize(v),
         (uint8_t *)v->array + (idx + items) * vec_itemsize(v),
         items * vec_itemsize(v));
  resize(v, v->size - items);
}

/*
 * Return a copy of a vector
 */
vector *vec_copy(vector *v)
{
  vector *w = vec_new(vec_itemsize(v));
  resize(w, vec_items(v));
  memcpy(vec_array(w), vec_array(v), vec_items(v) * vec_itemsize(v));
  return w;
}
