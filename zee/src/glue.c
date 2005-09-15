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
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

#include "config.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"

/*
 * Emit an error sound and cancel any macro definition.
 */
void ding(void)
{
  term_beep();

  if (thisflag & FLAG_DEFINING_MACRO)
    cancel_kbd_macro();
}

#define MAX_KEY_BUF	16

static int key_buf[MAX_KEY_BUF];
static int *keyp = key_buf;

/*
 * Get a keystroke, waiting for up to timeout 10ths of a second if
 * mode contains GETKEY_DELAYED, and translating it into a
 * keycode unless mode contains GETKEY_UNFILTERED.
 */
size_t xgetkey(int mode, size_t timeout)
{
  size_t key;

  if (keyp > key_buf)
    return *--keyp;

  key = term_xgetkey(mode, timeout);
  if (thisflag & FLAG_DEFINING_MACRO)
    add_key_to_cmd(key);
  return key;
}

/*
 * Wait for a keystroke indefinitely, and return the
 * corresponding keycode.
 */
size_t getkey(void)
{
  return xgetkey(0, 0);
}

/*
 * Wait for timeout 10ths if a second or until a key is pressed.
 * The key is then available with [x]getkey().
 */
void waitkey(size_t timeout)
{
  ungetkey(xgetkey(GETKEY_DELAYED, timeout));
}

void ungetkey(size_t key)
{
  if (keyp < key_buf + MAX_KEY_BUF && key != KBD_NOKEY)
    *keyp++ = key;
}

/*
 * Copy a region of text from the current buffer into an allocated buffer.
 *  start - the starting point.
 *  size - number of characters to copy.
 * If the region includes any line endings, they are turned into '\n'
 * irrespective of 'cur_bp->eol', and count as one character.
 */
astr copy_text_block(Point start, size_t size)
{
  size_t n, i;
  astr as = astr_new();
  Line *lp;

  assert(cur_bp);

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
  for (i = start.o; astr_len(as) < size;) {
    if (i < astr_len(lp->item))
      astr_cat_char(as, *astr_char(lp->item, (ptrdiff_t)(i++)));
    else {
      astr_cat_char(as, '\n');
      lp = list_next(lp);
      i = 0;
    }
  }

  return as;
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
  assert(cur_bp);

  if (cur_bp->pt.n > pt.n)
    do
      FUNCALL(edit_navigate_up_line);
    while (cur_bp->pt.n > pt.n);
  else if (cur_bp->pt.n < pt.n)
    do
      FUNCALL(edit_navigate_down_line);
    while (cur_bp->pt.n < pt.n);

  if (cur_bp->pt.o > pt.o)
    do
      FUNCALL(edit_navigate_backward_char);
    while (cur_bp->pt.o > pt.o);
  else if (cur_bp->pt.o < pt.o)
    do
      FUNCALL(edit_navigate_forward_char);
    while (cur_bp->pt.o < pt.o);
}
