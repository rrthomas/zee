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
 * Concatenate two lists (non-destructively).
 *
 * Takes time O(log(n)) where 'n' is the length of the result.
 */
rblist rblist_concat(rblist left, rblist right);

/* End of primitive constructors. */

/* Make a string from an rblist. */
char rblist_get(rblist rbl, size_t index);

#endif
