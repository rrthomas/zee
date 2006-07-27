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

#include "nonstd.h"
#include "zmalloc.h"
#include "astr.h"
#include "rblist.h"


rblist astr_nl(void)
{
  static rblist ret = NULL;
  // FIXME: It is infuriating to have to do this "if"! Is there a better way?
  if (!ret)
    ret = rblist_singleton('\n');
  return ret;
}

char *astr_to_string(rblist rbl)
{
  char *s = zmalloc(rblist_length(rbl) + 1);
  *rblist_to_string(rbl, s) = 0;
  return s;
}

size_t astr_str(rblist haystack, size_t from, rblist needle)
{
  size_t pos;

  for (pos = from; pos + rblist_length(needle) <= rblist_length(haystack); pos++)
    if (!rblist_compare(rblist_sub(haystack, (size_t)pos, (size_t)pos + rblist_length(needle)), needle))
      return pos;

  return SIZE_MAX;
}

rblist astr_fread(FILE *fp)
{
  int c;
  rblist as = rblist_empty;

  while ((c = getc(fp)) != EOF)
    as = rblist_append(as, c);
  return as;
}

rblist astr_fgets(FILE *fp)
{
  int c;
  rblist as;

  if (feof(fp))
    return NULL;
  as = rblist_empty;
  while ((c = getc(fp)) != EOF && c != '\n')
    as = rblist_append(as, c);
  return as;
}

/*
 * Used internally by astr_afmt. Formats an unsigned number in any
 * base up to 16. If the result has more than 64 digits, higher
 * digits are discarded.
 */
static rblist fmt_number(size_t x, int base)
{
  static const char const *digits = "0123456789abcdef";
  #define MAX_DIGITS 64
  static char buf[MAX_DIGITS];

  if (!x)
    return rblist_from_string("0");
  size_t i;
  for (i = MAX_DIGITS; i && x; x /= base)
    buf[--i] = digits[x % base];
  return rblist_from_array(&buf[i], MAX_DIGITS - i);
}

/*
 * Formats a string a bit like printf, and returns the result as an
 * rblist. This function understands "%r" to mean an rblist. This
 * function does not undertand the full syntax for printf format
 * strings. The only conversion specifications it understands are:
 *
 *   %r - an rblist (non-standard!),
 *   %s - a C string,
 *   %c - a char,
 *   %d - an int,
 *   %o - an unsigned int, printed in octal,
 *   %x - an unsigned int, printed in hexadecimal,
 *   %% - a % character.
 *
 * Width and precision specifiers are not supported.
 */
rblist astr_afmt(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  rblist ret = rblist_empty;
  int x;
  size_t i;
  while (1) {
    // Skip to next '%' or to end of string.
    for (i = 0; format[i] && format[i] != '%'; i++);
    ret = rblist_concat(ret, rblist_from_array(format, i));
    if (!format[i++])
      break;
    // We've found a '%'.
    switch (format[i++]) {
      case 'c':
        ret = rblist_append(ret, (char)va_arg(ap, int));
        break;
      case 'd': {
        x = va_arg(ap, int);
        if (x < 0)
          ret = rblist_concat(rblist_append(ret, '-'), fmt_number((unsigned)-x, 10));
        else
          ret = rblist_concat(ret, fmt_number((unsigned)x, 10));
        break;
      }
      case 'o':
        ret = rblist_concat(ret, fmt_number(va_arg(ap, unsigned int), 8));
        break;
      case 'r':
        ret = rblist_concat(ret, va_arg(ap, rblist));
        break;
      case 's':
        ret = rblist_concat(ret, rblist_from_string(va_arg(ap, const char *)));
        break;
      case 'x':
        ret = rblist_concat(ret, fmt_number(va_arg(ap, unsigned int), 16));
        break;
      case '%':
        ret = rblist_append(ret, '%');
        break;
      default:
        assert(0);
    }
    format = &format[i];
  }
  va_end(ap);
  return ret;
}


#ifdef TEST

#include <stdio.h>
#include <stdlib.h>

// Stub to make zrealloc happy
void die(int exitcode)
{
  exit(exitcode);
}

int main(void)
{
  rblist as1, as2, as3;
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

#endif // TEST
