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

#include "config.h"

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "nonstd.h"
#include "zmalloc.h"
#include "astr.h"


size_t astr_str(astr haystack, size_t from, astr needle)
{
  size_t pos;

  if (from + rblist_length(needle) <= rblist_length(haystack))
    for (pos = from; (size_t)pos <= rblist_length(haystack) - rblist_length(needle); pos++)
      if (!rblist_compare(rblist_sub(haystack, (size_t)pos, (size_t)pos + rblist_length(needle)), needle))
        return pos;

  return SIZE_MAX;
}

astr astr_fread(FILE *fp)
{
  int c;
  astr as = rblist_from_string("");

  while ((c = getc(fp)) != EOF)
    as = rblist_concat_char(as, c);
  return as;
}

astr astr_fgets(FILE *fp)
{
  int c;
  astr as;

  if (feof(fp))
    return NULL;
  as = rblist_from_string("");
  while ((c = getc(fp)) != EOF && c != '\n')
    as = rblist_concat_char(as, c);
  return as;
}

astr astr_afmt(const char *fmt, ...)
{
  va_list ap;
  int len;
  char *s = NULL;

  va_start(ap, fmt);
  len = vsnprintf(s, 0, fmt, ap);
  va_end(ap);
  s = zmalloc((size_t)len + 1);

  va_start(ap, fmt);
  assert(vsnprintf(s, (size_t)len + 1, fmt, ap) == len);
  va_end(ap);

  return rblist_from_string(s);
}


#ifdef TEST

#include <stdio.h>
#include <stdlib.h>

/* Stub to make zrealloc happy */
void die(int exitcode)
{
  exit(exitcode);
}

int main(void)
{
  astr as1, as2, as3;
  FILE *fp;

  as1 = rblist_from_string("hello world!");
  as3 = rblist_sub(as1, 6, 11);
  assert(!rblist_compare(as3, rblist_from_string("world")));

  as2 = rblist_from_string("The ");
  as2 = rblist_concat(as2, as3);
  assert(!rblist_compare(as2, rblist_from_string("The world")));

  as3 = rblist_sub(as1, 6, 11);
  assert(!rblist_compare(as3, rblist_from_string("world")));

  as1 = rblist_from_string("12345");
  as2 = rblist_sub(as1, 3, rblist_length(as1));
  assert(!rblist_compare(as2, rblist_from_string("45")));

  assert(!rblist_compare(rblist_sub(rblist_from_string("12345"), 0, rblist_length(as1)),
                   rblist_from_string("12345")));

  assert(!rblist_compare(rblist_concat(astr_afmt("%s * %d = ", "5", 3), astr_afmt("%d", 15)),
                   rblist_from_string("5 * 3 = 15")));

  assert(rblist_compare(rblist_from_string("a"), rblist_from_string("abc")) == -1);

  assert(rblist_compare(rblist_from_string("abc"), rblist_from_string("a")) == 1);

  assert(astr_str(rblist_from_string("rumblebumbleapplebombleboo"), 0, rblist_from_string("apple")) == 12);

  assert(astr_str(rblist_from_string("rumblebumbleapple"), 0, rblist_from_string("apple")) == 12);

  assert(astr_str(rblist_from_string("appleabumbleapplebombleboo"), 6, rblist_from_string("apple")) == 12);

  assert(fp = fopen(SRCPATH "astr.c", "r"));
  as1 = astr_fgets(fp);
  assert(!rblist_compare(as1, rblist_from_string("/* Dynamically allocated strings")));
  assert(fclose(fp) == 0);

  return EXIT_SUCCESS;
}

#endif /* TEST */
