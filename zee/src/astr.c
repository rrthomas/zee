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

#include <stdarg.h>
#include <string.h>

#include "zmalloc.h"
#include "astr.h"
#include "main.h"


/* FIXME: stop strings being NUL-terminated (except when returned by
   astr_cstr): it causes too many damn problems. */

astr astr_new(const char *s)
{
  astr as = vec_new(sizeof(char));
  *(char *)vec_index(as, 0) = '\0'; /* Set up NUL terminator so astr_ncat will work */
  return astr_ncat(as, s, strlen(s));
}

static int abspos(astr as, ptrdiff_t pos)
{
  assert(as);
  if (pos < 0)
    pos = astr_len(as) + pos;
  assert(pos >= 0 && pos <= (ptrdiff_t)astr_len(as));
  return pos;
}

char *astr_char(const astr as, ptrdiff_t pos)
{
  pos = abspos(as, pos);
  return ((char *)vec_array(as)) + pos;
}

astr astr_ncat(astr as, const char *s, size_t n)
{
  size_t i, len = astr_len(as);
  for (i = 0; i < n; i++)
    *(char *)vec_index(as, len + i) = s[i];
  *(char *)vec_index(as, len + n) = '\0';
  return as;
}

astr astr_cat(astr as, const astr src)
{
  assert(src);
  return astr_ncat(as, vec_array(src), astr_len(src));
}

astr astr_cat_char(astr as, int c)
{
  char ch = c;
  return astr_ncat(as, &ch, 1);
}

astr astr_dup(const astr src)
{
  return astr_cat(astr_new(""), src);
}

astr astr_sub(const astr as, ptrdiff_t from, ptrdiff_t to)
{
  from = abspos(as, from);
  to = abspos(as, to);

  if (from > to) {
    ptrdiff_t temp = from;
    from = to;
    to = temp;
  }

  return astr_ncat(astr_new(""), astr_char(as, from), (size_t)(to - from));
}

int astr_cmp(astr as1, astr as2)
{
  return strcmp((char *)vec_array(as1), (char *)vec_array(as2));
}

int astr_ncmp(astr as1, astr as2, size_t n)
{
  return strncmp((char *)vec_array(as1), (char *)vec_array(as2), n);
}

astr astr_nreplace(astr as, ptrdiff_t pos, size_t size, const char *s, size_t csize)
{
  astr tail;

  pos = abspos(as, pos);
  if (astr_len(as) - pos < size)
    size = astr_len(as) - pos;
  tail = astr_sub(as, pos + (ptrdiff_t)size, (ptrdiff_t)astr_len(as));
  astr_truncate(as, pos);
  astr_ncat(as, s, csize);
  astr_cat(as, tail);

  return as;
}

/* FIXME: Make third argument a pos */
astr astr_remove(astr as, ptrdiff_t pos, size_t size)
{
  pos = abspos(as, pos);
  vec_shrink(as, (size_t)pos, size);
  return as;
}

astr astr_truncate(astr as, ptrdiff_t pos)
{
  return astr_remove(as, pos, astr_len(as));
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

/* as is an astr, s is a char *; defined as a macro to give useful
   line numbers for failed assertions. */
#define assert_eq(as, s) \
  if (strcmp(astr_cstr(as), (s))) { \
    printf("test failed: \"%s\" != \"%s\"\n", (char *)vec_array(as), (s)); \
    assert(0); \
  }

/*
 * Stub to make zmalloc &c. happy.
 */
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
  assert_eq(as3, "world");

  as2 = astr_new("The ");
  astr_cat(as2, as3);
  assert_eq(as2, "The world");

  as3 = astr_sub(as1, -6, 11);
  assert_eq(as3, "world");

  as1 = astr_new("1234567");
  astr_remove(as1, 4, 10);
  assert_eq(as1, "1234");

  as1 = astr_new("1234567");
  astr_remove(as1, 0, 1);
  assert_eq(as1, "234567");

  as1 = astr_new("123");
  astr_truncate(as1, 1);
  assert_eq(as1, "1");

  as1 = astr_new("12345");
  as2 = astr_sub(as1, -2, (ptrdiff_t)astr_len(as1));
  assert_eq(as2, "45");

  as1 = astr_new("12345");
  as2 = astr_sub(as1, -5, (ptrdiff_t)astr_len(as1));
  assert_eq(as2, "12345");

  as1 = astr_afmt("%s * %d = ", "5", 3);
  astr_cat(as1, astr_afmt("%d", 15));
  assert_eq(as1, "5 * 3 = 15");

  assert(fp = fopen(SRCPATH "astr.c", "r"));
  as1 = astr_fgets(fp);
  printf("The first line of astr.c is: \"%s\"\n", astr_cstr(as1));
  assert_eq(as1, "/* Dynamically allocated strings");
  assert(fclose(fp) == 0);

  printf("astr tests successful.\n");

  return 0;
}

#endif /* TEST */
