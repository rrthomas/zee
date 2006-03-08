/* Randomly balanced lists.
   Copyright (c) 2006 Alistair Turnbull.
   Copyright (c) 2006 Reuben Thomas.
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

#ifndef RBLIST_H
#define RBLIST_H

/*
 * The type of lists. This is a pointer type. The structure it points to
 * is opaque. You may assume the structure is immutable (const). No
 * operation in this library ever modifies an rblist in place.
 *
 * Takes memory O(n) where 'n' is the length of the list. Typically about
 * twice as much memory will be used as would be used for an ordinary
 * C array.
 */
typedef const union rblist *rblist;

/*
 * The type of list iterators (i.e. the type of the state of a loop
 * through a list). This is a pointer type, and NULL is means that the
 * loop is finished. The structure it points to is opaque and gets
 * modified in-place by 'rblist_next()'.
 *
 * An rblist_iterator can be thought of a bit like a pointer into the
 * list (but that's not what it is). rblist_iterator_value dereferences
 * an rblist_iterator to obtain an element of the list.
 * rblist_iterator_next increments an rblist_iterator to move on to the
 * next element. The most concise way to use these functions is using the
 * FOREACH macro.
 *
 * Takes memory O(log(n)) where 'n' is the length of the list.
 */
typedef struct rblist_iterator *rblist_iterator;

/***************************/
/* Primitive constructors. */

/*
 * Make an rblist from an array. This can be achieved using
 * rblist_singleton and rblist_concat, but this method is more efficient.
 *
 * Takes time O(n) where 'n' is the length of the list.
 */
rblist rblist_from_array(const char *s, size_t length);

/*
 * The empty list. There is only one empty list, so this is a constant,
 * not a function.
 */
const rblist rblist_empty;

/*
 * Make a list of length 1. See also rblist_from_array.
 *
 * Takes time O(1).
 */
rblist rblist_singleton(char c);

/*
 * Concatenate two lists. The originals are not modified.
 *
 * Takes time O(log(n)) where 'n' is the length of the result.
 */
rblist rblist_concat(rblist left, rblist right);

/**************************/
/* Primitive destructors. */

/*
 * Read the length of an rblist.
 *
 * Take time O(1).
 */
size_t rblist_length(rblist rbl);

/*
 * Break an rblist into two at the specified position, and store the two
 * halves are stored in *left and *right. The original is not modified.
 *
 * Takes time O(log(n)) where 'n' is the length of the list.
 */
void rblist_split(rblist rbl, size_t pos, rblist *left, rblist *right);

/*
 * Constructs an iterator over the specified rblist.
 *
 * Takes time O(log(n)) where 'n' is the length of the list.
 */
rblist_iterator rblist_iterate(rblist rbl);

/*
 * Returns thelist element currently addressed by 'it'.
 *
 * Takes time O(1).
 */
char rblist_iterator_value(rblist_iterator it);

/*
 * Advances the rblist_iterator one place down its list and returns the
 * result. The original is destroyed and should be discarded. Retuens
 * NULL at the end of the list.
 *
 * Takes time O(1) on average.
 */
rblist_iterator rblist_iterator_next(rblist_iterator it);

/************************/
/* Derived destructors. */

#define RBLIST_FOR(_loop_var, rbl, body) \
  for ( \
    rblist_iterator _it_##_loop_var = rblist_iterate(rbl); \
    _it_##_loop_var; \
    _it_##_loop_var = rblist_iterator_next(_it_##_loop_var) \
  ) { \
    char _loop_var = rblist_iterator_value(_it_##_loop_var); \
    body \
  }

/* Returns one element of an rblist. */
char rblist_get(rblist rbl, size_t index);

#endif
