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
 * Tuning parameter which controls memory/CPU compromise.
 * rblists shorter than this will be implemented using a primitive array.
 * Increase to use less memory and more CPU.
 */
#define RBLIST_MINIMUM_NODE_LENGTH 64

/* The type of lists. Instances are immutable. */
typedef const union rblist *rblist;

/* The empty list. */
rblist rblist_empty;

/* Make a list of length 1. */
rblist rblist_singleton(char c);

/* Concatenate two lists (non-destructive). */
rblist rblist_concat(rblist left, rblist right);

/* Make an rblist from a string. */
rblist rblist_from_string(const char *s);

/* Make a string from an rblist. */
rblist rblist_get(rblist rbl);

#endif
