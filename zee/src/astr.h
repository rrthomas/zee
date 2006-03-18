/* Dynamically allocated strings
   Copyright (c) 2001-2004 Sandro Sigala.
   Copyright (c) 2003-2006 Reuben Thomas.
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

#ifndef ASTR_H
#define ASTR_H

#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>

#include "rblist.h"

/* Useful constant: rblist_singleton('\n') */
rblist astr_nl(void);

/*
 * Find first occurrence of needle in haystack starting at position
 * pos; return -1 if no occurrence.
 */
size_t astr_str(const rblist haystack, size_t pos, const rblist needle);

/*
 * Read a file into an astr.
 */
rblist astr_fread(FILE *fp);

/*
 * Read a line from the stream fp and return it. The trailing newline
 * is removed from the string. If the stream is at eof when astr_fgets
 * is called, it returns NULL.
 */
rblist astr_fgets(FILE *fp);

/*
 * Format text into an astr
 */
rblist astr_afmt(const char *fmt, ...);

/*
 * Returns the elements of `rbl' in a freshly allocated array with a 0
 * terminator (i.e. as a C string).
 *
 * Takes time O(n) where `n' is the length of the list.
 */
char *astr_to_string(rblist rbl);

#endif /* ASTR_H */
