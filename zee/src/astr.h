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

/*
 * The astr library provides dynamically allocated null-terminated C
 * strings.
 *
 * String positions start at zero, as with ordinary C strings.
 */

typedef rblist astr;

/*
 * Find first occurrence of needle in haystack starting at position
 * pos; return -1 if no occurrence.
 */
size_t astr_str(const astr haystack, size_t pos, const astr needle);

/*
 * Read a file into an astr.
 */
astr astr_fread(FILE *fp);

/*
 * Read a line from the stream fp and return it. The trailing newline
 * is removed from the string. If the stream is at eof when astr_fgets
 * is called, it returns NULL.
 */
astr astr_fgets(FILE *fp);

/*
 * Format text into an astr
 */
astr astr_afmt(const char *fmt, ...);


#endif /* ASTR_H */
