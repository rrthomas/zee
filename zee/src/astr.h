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

#include "vector.h"

/*
 * The astr library provides dynamically allocated null-terminated C
 * strings.
 *
 * String positions start at zero, as with ordinary C strings.
 * Negative values are also allowed, and count from the end of the
 * string.
 * FIXME: With this scheme it's impossible to refer to the last
 * character with a (negative) constant.
 *
 * Where not otherwise specified, the functions return the first
 * argument string, usually named as in the function prototype.
 */

typedef vector *astr;

/*
 * Create a new string with initial contents s.
 */
astr astr_new(const char *s);

/*
 * Convert as into a C null-terminated string.
 * as[0] to as[astr_size(as) - 1] inclusive may be read.
 */
#define astr_cstr(as)           ((const char *)(((astr)(as))->array))

/*
 * Return the length of the argument string as.
 */
#define astr_len(as)            ((const size_t)(((astr)(as))->items) - 1)

/*
 * Return the address of the pos'th character of as.
 */
char *astr_char(const astr as, ptrdiff_t pos);

/*
 * Return a new astr consisting of size characters from string as
 * starting from position pos.
 */
astr astr_sub(const astr as, ptrdiff_t from, ptrdiff_t to);

/*
 * Do str[n]cmp on the contents of two strings.
 */
int astr_cmp(astr as1, astr as2);
int astr_ncmp(astr as1, astr as2, size_t n);

/*
 * Append the contents of the argument string or character to as.
 */
astr astr_ncat(astr as, const char *s, size_t csize);
astr astr_cat(astr as, const astr src);
astr astr_cat_char(astr as, int c);

/*
 * Duplicate as.
 */
astr astr_dup(const astr src);

/*
 * Replace size characters of as, starting at pos, with the argument
 * string.
 * FIXME: Change to astr_replace(astr as, ptrdiff_t from, ptrdiff_t to, astr bs)
 */
astr astr_nreplace(astr as, ptrdiff_t pos, size_t size, const char *s, size_t csize);

/*
 * Remove size chars from as at position pos.
 */
astr astr_remove(astr as, ptrdiff_t pos, size_t size);

/*
 * Truncate as to given position.
 */
astr astr_truncate(astr as, ptrdiff_t pos);

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
