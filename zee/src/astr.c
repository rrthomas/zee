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


#define ALLOCATION_CHUNK_SIZE	16

astr astr_new(const char *s)
{
  astr as = (astr)zmalloc(sizeof *as);
  assert(s);
  as->maxlen = as->len = strlen(s);
  as->text = strcpy(zmalloc(as->len + 1), s);
  return as;
}

static void resize(astr as, size_t reqsize)
{
  assert(as);
  if (reqsize > as->maxlen) {
    size_t oldmaxlen = as->maxlen;
    as->maxlen = reqsize + ALLOCATION_CHUNK_SIZE;
    as->text = zrealloc(as->text, oldmaxlen + 1, as->maxlen + 1);
  }
}

static int abspos(astr as, ptrdiff_t pos)
{
  assert(as);
  if (pos < 0)
    pos = as->len + pos;
  assert(pos >= 0 && pos <= (int)as->len);
  return pos;
}

char *astr_char(const astr as, ptrdiff_t pos)
{
  pos = abspos(as, pos);
  return as->text + pos;
}

astr astr_dup(const astr src)
{
  return astr_cat(astr_new(""), src);
}

astr astr_ncat(astr as, const char *s, size_t csize)
{
  resize(as, as->len + csize);
  memcpy(as->text + as->len, s, csize);
  as->len += csize;
  as->text[as->len] = '\0';
  return as;
}

astr astr_cat(astr as, const astr src)
{
  assert(src);
  return astr_ncat(as, src->text, src->len);
}

astr astr_cat_char(astr as, int c)
{
  resize(as, as->len + 1);
  as->text[as->len] = (char)c;
  as->text[++as->len] = '\0';
  return as;
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

  return astr_ncat(astr_new(""), astr_char(as, from), to - from);
}

int astr_cmp(astr as1, astr as2)
{
  return strcmp(((astr)(as1))->text, ((astr)(as2))->text);
}

int astr_ncmp(astr as1, astr as2, size_t n)
{
  return strncmp(((astr)(as1))->text, ((astr)(as2))->text, n);
}

astr astr_nreplace(astr as, ptrdiff_t pos, size_t size, const char *s, size_t csize)
{
  astr tail;

  pos = abspos(as, pos);
  if (as->len - pos < size)
    size = as->len - pos;
  tail = astr_sub(as, pos + (ptrdiff_t)size, astr_len(as));
  astr_truncate(as, pos);
  astr_ncat(as, s, csize);
  astr_cat(as, tail);

  return as;
}

astr astr_remove(astr as, ptrdiff_t pos, size_t size)
{
  return astr_nreplace(as, pos, size, "", 0);
}

/* Don't define in terms of astr_remove, to avoid endless recursion */
astr astr_truncate(astr as, ptrdiff_t pos)
{
  pos = abspos(as, pos);
  if ((size_t)pos < as->len) {
    as->len = pos;
    as->text[pos] = '\0';
  }
  return as;
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

static void assert_eq(astr as, const char *s)
{
  if (strcmp(astr_cstr(as), s)) {
    printf("test failed: \"%s\" != \"%s\"\n", as->text, s);
    assert(0);
  }
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

  as1 = astr_new("12345");
  as2 = astr_sub(as1, -2, astr_len(as1));
  assert_eq(as2, "45");

  as1 = astr_new("12345");
  as2 = astr_sub(as1, -5, astr_len(as1));
  assert_eq(as2, "12345");

  as1 = astr_afmt("%s * %d = ", "5", 3);
  astr_cat(as1, astr_afmt("%d", 15));
  assert_eq(as1, "5 * 3 = 15");

  assert(fp = fopen(SRCPATH "astr.c", "r"));
  as1 = astr_fgets(fp);
  printf("The first line of astr.c is: \"%s\"\n", astr_cstr(as1));
  assert(fclose(fp) == 0);

  printf("astr tests successful.\n");

  return 0;
}

#endif /* TEST */
