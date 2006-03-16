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
#include <string.h>

#include "nonstd.h"
#include "zmalloc.h"
#include "astr.h"


astr astr_new(const char *s)
{
  return rblist_from_string(s);
}

size_t astr_len(const astr as)
{
  return rblist_length(as);
}

char astr_char(const astr as, ptrdiff_t pos)
{
  return rblist_get(as, (size_t)pos);
}

const char *astr_cstr(const astr as)
{
  return rblist_to_string(as);
}

astr astr_cat(astr as, const astr src)
{
  return rblist_concat(as, src);
}

astr astr_cat_char(astr as, int c)
{
  return rblist_concat(as, rblist_singleton(c));
}

astr astr_dup(const astr src)
{
  return src;
}

astr astr_sub(const astr as, ptrdiff_t from, ptrdiff_t to)
{
  return rblist_sub(as, (size_t)from, (size_t)to);
}

int astr_cmp(const astr as1, const astr as2)
{
  return rblist_compare(as1, as2);
}

int astr_ncmp(const astr as1, const astr as2, size_t n)
{
  return rblist_ncompare(as1, as2, n);
}

ptrdiff_t astr_str(astr haystack, ptrdiff_t from, astr needle)
{
  ptrdiff_t pos;

  if (from + astr_len(needle) <= astr_len(haystack))
    for (pos = from; (size_t)pos <= astr_len(haystack) - astr_len(needle); pos++)
      if (!rblist_compare(rblist_sub(haystack, (size_t)pos, (size_t)pos + astr_len(needle)), needle))
        return pos;

  return -1;
}

astr astr_fread(FILE *fp)
{
  int c;
  astr as = astr_new("");

  while ((c = getc(fp)) != EOF)
    as = astr_cat_char(as, c);
  return as;
}

astr astr_fgets(FILE *fp)
{
  int c;
  astr as;

  if (feof(fp))
    return NULL;
  as = astr_new("");
  while ((c = getc(fp)) != EOF && c != '\n')
    as = astr_cat_char(as, c);
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

  return astr_new(s);
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

  as1 = astr_new("hello world!");
  as3 = astr_sub(as1, 6, 11);
  assert(!astr_cmp(as3, astr_new("world")));

  as2 = astr_new("The ");
  as2 = astr_cat(as2, as3);
  assert(!astr_cmp(as2, astr_new("The world")));

  as3 = astr_sub(as1, 6, 11);
  assert(!astr_cmp(as3, astr_new("world")));

  as1 = astr_new("12345");
  as2 = astr_sub(as1, 3, (ptrdiff_t)astr_len(as1));
  assert(!astr_cmp(as2, astr_new("45")));

  assert(!astr_cmp(astr_sub(astr_new("12345"), 0, (ptrdiff_t)astr_len(as1)),
                   astr_new("12345")));

  assert(!astr_cmp(astr_cat(astr_afmt("%s * %d = ", "5", 3), astr_afmt("%d", 15)),
                   astr_new("5 * 3 = 15")));

  assert(astr_cmp(astr_new("a"), astr_new("abc")) == -1);

  assert(astr_cmp(astr_new("abc"), astr_new("a")) == 1);

  assert(astr_str(astr_new("rumblebumbleapplebombleboo"), 0, astr_new("apple")) == 12);

  assert(astr_str(astr_new("rumblebumbleapple"), 0, astr_new("apple")) == 12);

  assert(astr_str(astr_new("appleabumbleapplebombleboo"), 6, astr_new("apple")) == 12);

  assert(fp = fopen(SRCPATH "astr.c", "r"));
  as1 = astr_fgets(fp);
  assert(!astr_cmp(as1, astr_new("/* Dynamically allocated strings")));
  assert(fclose(fp) == 0);

  return EXIT_SUCCESS;
}

#endif /* TEST */
