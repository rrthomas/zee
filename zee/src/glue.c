/* Miscellaneous functions
   Copyright (c) 1997-2004 Sandro Sigala.  All rights reserved.

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
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

#include "config.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zee.h"
#include "extern.h"

/*
 * Emit an error sound.
 */
void ding(void)
{
  if (thisflag & FLAG_DEFINING_MACRO)
    cancel_kbd_macro();

  term_beep();

  thisflag |= FLAG_GOT_ERROR;
}

/*
 * Get a keystroke, waiting for up to timeout 10ths of a second if
 * mode contains GETKEY_DELAYED, and translating it into a Zee
 * keycode unless mode contains GETKEY_UNFILTERED.
 */
size_t xgetkey(int mode, size_t timeout)
{
  size_t key = term_xgetkey(mode, timeout);
  if (thisflag & FLAG_DEFINING_MACRO)
    add_key_to_cmd(key);
  return key;
}

/*
 * Wait for a keystroke and return the Zee key code.
 */
size_t getkey(void)
{
  return term_xgetkey(0, 0);
}

/*
 * Wait for two seconds or until a key is pressed.
 * The key is then available with getkey().
 */
void waitkey(size_t delay)
{
  term_ungetkey(term_xgetkey(GETKEY_DELAYED, delay));
}

/*
 * Copy a region of text into an allocated buffer.
 */
char *copy_text_block(size_t startn, size_t starto, size_t size)
{
  char *buf, *dp;
  size_t max_size, n, i;
  Line *lp;

  max_size = 10;
  dp = buf = (char *)zmalloc(max_size);

  lp = cur_bp->pt.p;
  n = cur_bp->pt.n;
  if (n > startn)
    do
      lp = list_prev(lp);
    while (--n > startn);
  else if (n < startn)
    do
      lp = list_next(lp);
    while (++n < startn);

  for (i = starto; dp - buf < (int)size;) {
    if (dp >= buf + max_size) {
      int save_off = dp - buf;
      max_size += 10;
      buf = (char *)zrealloc(buf, max_size);
      dp = buf + save_off;
    }
    if (i < astr_len(lp->item))
      *dp++ = *astr_char(lp->item, (ptrdiff_t)(i++));
    else {
      *dp++ = '\n';
      lp = list_next(lp);
      i = 0;
    }
  }

  return buf;
}

/*
 * Return a string of maximum length `maxlen' beginning with a `...'
 * sequence if a cut is need.
 */
astr shorten_string(char *s, int maxlen)
{
  int len;
  astr as = astr_new();

  if ((len = strlen(s)) <= maxlen)
    astr_cpy_cstr(as, s);
  else {
    astr_cpy_cstr(as, "...");
    astr_cat_cstr(as, s + len - maxlen + 3);
  }

  return as;
}

/*
 * Jump to the specified line number and offset.
 */
void goto_point(Point pt)
{
  if (cur_bp->pt.n > pt.n)
    do
      FUNCALL(previous_line);
    while (cur_bp->pt.n > pt.n);
  else if (cur_bp->pt.n < pt.n)
    do
      FUNCALL(next_line);
    while (cur_bp->pt.n < pt.n);

  if (cur_bp->pt.o > pt.o)
    do
      FUNCALL(backward_char);
    while (cur_bp->pt.o > pt.o);
  else if (cur_bp->pt.o < pt.o)
    do
      FUNCALL(forward_char);
    while (cur_bp->pt.o < pt.o);
}

/*
 * Read an arbitrary length string.
 */
char *getln(FILE *fp)
{
  size_t len = 256;
  int c;
  char *l = zmalloc(len), *s = l;

  for (c = getc(fp); c != '\n' && c != EOF; c = getc(fp)) {
    if (s == l + len)
      zrealloc(l, len *= 2);
    *s++ = c;
  }
  *s = '\0';

  return l;
}
