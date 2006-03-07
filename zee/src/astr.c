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

/* Copies n characters from s onto the end of as, growing as as necessary. */
static astr ncat(astr as, const char *s, size_t n)
{
  size_t i, len = astr_len(as);
  for (i = 0; i < n; i++)
    *(char *)vec_index(as, len + i) = s[i];
  return as;
}

/* Converts a possibly negative pos into a definitely non-negative pos. */
static int abspos(astr as, ptrdiff_t pos)
{
  assert(as);
  if (pos < 0)
    pos = astr_len(as) + pos;
  assert(pos >= 0 && pos <= (ptrdiff_t)astr_len(as));
  return pos;
}

astr astr_new(const char *s)
{
  astr as = vec_new(sizeof(char));
  return ncat(as, s, strlen(s));
}

const char *astr_cstr(const astr as)
{
  char *s = zmalloc(astr_len(as) + 1);
  memcpy(s, vec_array(as), astr_len(as));
  return s;
}

char *astr_char(const astr as, ptrdiff_t pos)
{
  pos = abspos(as, pos);
  return ((char *)vec_array(as)) + pos;
}

astr astr_cat(astr as, const astr src)
{
  assert(src);
  return ncat(as, vec_array(src), astr_len(src));
}

astr astr_cat_char(astr as, int c)
{
  char ch = c;
  return ncat(as, &ch, 1);
}

astr astr_dup(const astr src)
{
  return astr_cat(astr_new(""), src);
}

astr astr_sub(const astr as, ptrdiff_t from, ptrdiff_t to)
{
  from = abspos(as, from);
  to = abspos(as, to);
  return ncat(astr_new(""), astr_char(as, from), (size_t)(to - from));
}

int astr_cmp(const astr as1, const astr as2)
{
  int ret = memcmp((char *)vec_array(as1), (char *)vec_array(as2),
                   min(astr_len(as1), astr_len(as2)));

  if (ret == 0 && astr_len(as1) != astr_len(as2))
    ret = astr_len(as1) < astr_len(as2) ? -1 : 1;

  return ret;
}

int astr_ncmp(const astr as1, const astr as2, size_t n)
{
  if (astr_len(as1) < n || astr_len(as2) < n)
    return astr_cmp(as1, as2);
  else
    return memcmp((char *)vec_array(as1), (char *)vec_array(as2), n);
}

ptrdiff_t astr_str(astr haystack, ptrdiff_t from, astr needle)
{
  ptrdiff_t pos;

  from = abspos(haystack, from);

  if (from + astr_len(needle) <= astr_len(haystack))
    for (pos = from; (size_t)pos <= astr_len(haystack) - astr_len(needle); pos++)
      if (!memcmp(astr_char(haystack, pos), vec_array(needle), astr_len(needle)))
        return pos;

  return -1;
}

astr astr_fread(FILE *fp)
{
  int c;
  astr as = astr_new("");

  while ((c = getc(fp)) != EOF)
    astr_cat_char(as, c);
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
    astr_cat_char(as, c);
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
  astr_cat(as2, as3);
  assert(!astr_cmp(as2, astr_new("The world")));

  as3 = astr_sub(as1, -6, 11);
  assert(!astr_cmp(as3, astr_new("world")));

  as1 = astr_new("12345");
  as2 = astr_sub(as1, -2, (ptrdiff_t)astr_len(as1));
  assert(!astr_cmp(as2, astr_new("45")));

  assert(!astr_cmp(astr_sub(astr_new("12345"), -5, (ptrdiff_t)astr_len(as1)),
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

  as1 = astr_cat(astr_new(""), astr_new("x"));
  assert(as1->size >= astr_len(as1));

  printf("astr tests passed\n");

  return EXIT_SUCCESS;
}

#endif /* TEST */
