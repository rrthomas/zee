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

#include "main.h"
#include "extern.h"

/*
 * Emit an error sound.
 * Also, calls cancel_mbd_macro() and sets FLAG_GOT_ERROR.
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
 * mode contains GETKEY_DELAYED, and translating it into a
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
 * Wait for a keystroke, waiting indefinitely, and return the
 * corresponding keycode.
 */
size_t getkey(void)
{
  return term_xgetkey(0, 0);
}

/*
 * Wait for 'delay' 10ths if a second or until a key is pressed.
 * The key is then available with getkey().
 */
void waitkey(size_t delay)
{
  term_ungetkey(term_xgetkey(GETKEY_DELAYED, delay));
}

/*
 * Copy a region of text from the current buffer into an allocated buffer.
 *  start - the starting point.
 *  size - number of characters to copy.
 * If the region includes any line endings, they are turned into '\n'
 * irrespective of 'cur_bp->eol', and count as one character.
 */
char *copy_text_block(Point start, size_t size)
{
  char *buf, *dp;
  size_t max_size, n, i;
  Line *lp;

  max_size = 10;
  dp = buf = (char *)zmalloc(max_size);

  /* Have to do a linear search through the buffer to find the start of the
   * region. Doesn't matter where we start. Starting at 'cur_bp->pt' is a good
   * heuristic.
   */
  lp = cur_bp->pt.p;
  n = cur_bp->pt.n;
  if (n > start.n)
    do
      lp = list_prev(lp);
    while (--n > start.n);
  else if (n < start.n)
    do
      lp = list_next(lp);
    while (++n < start.n);

  /* Copy one character at a time. */
  for (i = start.o; dp - buf < (int)size;) {
    if (dp >= buf + max_size) {
      int save_off = dp - buf;
      max_size += 10 + (max_size >> 2);
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

  return zrealloc(buf, size);
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
