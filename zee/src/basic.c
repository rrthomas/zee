/* Basic movement functions
   Copyright (c) 1997-2004 Sandro Sigala.
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

#include <limits.h>

#include "main.h"
#include "extern.h"

/* Goal column to arrive at when `edit_navigate_down/up_line'
   functions are used. */
static int cur_goalc;

DEFUN(beginning_of_line)
/*+
Move point to beginning of current line.
+*/
{
  buf.pt.o = 0;

  /* Set goalc to the beginning of line for next
     `edit_navigate_down/up_line' call. */
  thisflag |= FLAG_DONE_CPCN;
  cur_goalc = 0;
}
END_DEFUN

DEFUN(end_of_line)
/*+
Move point to end of current line.
+*/
{
  buf.pt.o = astr_len(buf.pt.p->item);

  /* Change the `goalc' to the end of line for next
     `edit-navigate-down/up-line' calls.  */
  thisflag |= FLAG_DONE_CPCN;
  cur_goalc = INT_MAX;
}
END_DEFUN

/*
 * Get the goal column.  Take care of expanding tabulations.
 */
size_t get_goalc(void)
{
  size_t col = 0, t = tab_width(), i;

  for (i = 0; i < buf.pt.o; i++) {
    if (*astr_char(buf.pt.p->item, (ptrdiff_t)i) == '\t')
      col |= t - 1;
    ++col;
  }

  return col;
}

/*
 * Go to column goalc. Take care of expanding tabulations
 */
static void goto_goalc(int goalc)
{
  int col = 0, t, w;
  size_t i;

  t = tab_width();

  for (i = 0; i < astr_len(buf.pt.p->item); i++) {
    if (col == goalc)
      break;
    else if (*astr_char(buf.pt.p->item, (ptrdiff_t)i) == '\t') {
      for (w = t - col % t; w > 0; w--)
        if (++col == goalc)
          break;
    } else
      ++col;
  }

  buf.pt.o = i;
}

DEFUN(edit_navigate_up_line)
/*+
Move cursor vertically up one line.
If there is no character in the target line exactly over the current column,
the cursor is positioned after the character in that line which spans this
column, or at the end of the line if it is not long enough.
+*/
{
  if (list_prev(buf.pt.p) == buf.lines)
    ok = FALSE;
  else {
    thisflag |= FLAG_NEED_RESYNC | FLAG_DONE_CPCN;

    if (!(lastflag & FLAG_DONE_CPCN))
      cur_goalc = get_goalc();

    buf.pt.p = list_prev(buf.pt.p);
    buf.pt.n--;

    goto_goalc(cur_goalc);
  }
}
END_DEFUN

DEFUN(edit_navigate_down_line)
/*+
Move cursor vertically down one line.
If there is no character in the target line exactly under the current column,
the cursor is positioned after the character in that line which spans this
column, or at the end of the line if it is not long enough.
+*/
{
  if (list_next(buf.pt.p) == buf.lines)
    ok = FALSE;
  else {
    thisflag |= FLAG_DONE_CPCN | FLAG_NEED_RESYNC;

    if (!(lastflag & FLAG_DONE_CPCN))
      cur_goalc = get_goalc();

    buf.pt.p = list_next(buf.pt.p);
    buf.pt.n++;

    goto_goalc(cur_goalc);
  }
}
END_DEFUN

/*
 * Jump to the specified column.
 */
int goto_column(size_t to_col)
{
  int ok = TRUE;

  if (buf.pt.o > to_col)
    do
      ok = FUNCALL(edit_navigate_backward_char);
    while (ok && buf.pt.o > to_col);
  else if (buf.pt.o < to_col)
    do
      ok = FUNCALL(edit_navigate_forward_char);
    while (ok && buf.pt.o < to_col);

  return ok;
}

DEFUN(goto_column)
/*+
Read a number N and move the cursor to character number N.
Position 1 is the beginning of the buffer.
FIXME: Make this and goto_line DEFUN_INTs and make the functions static.
+*/
{
  size_t to_char = 0;
  astr ms;

  do {
    if ((ms = minibuf_read(astr_new("Goto char: "), astr_new(""))) == NULL) {
      ok = FUNCALL(cancel);
      break;
    }
    if ((to_char = strtoul(astr_cstr(ms), NULL, 10)) == ULONG_MAX)
      ding();
  } while (to_char == ULONG_MAX);

  if (ok)
    goto_column(to_char);
}
END_DEFUN

/*
 * Go to the line `to_line', counting from 0.  Point will end up in
 * "random" column.
 */
int goto_line(size_t to_line)
{
  int ok = TRUE;

  if (buf.pt.n > to_line)
    do
      ok = FUNCALL(edit_navigate_up_line);
    while (ok && buf.pt.n > to_line);
  else if (buf.pt.n < to_line)
    do
      ok = FUNCALL(edit_navigate_down_line);
    while (ok && buf.pt.n < to_line);

  return ok;
}

/*
 * Go to the given point.
 */
int goto_point(Point pt)
{
  int ok = goto_line(pt.n);
  if (ok)
    ok = goto_column(pt.o);
  return ok;
}

DEFUN(goto_line)
/*+
Move cursor to the beginning of the specified line.
Line 1 is the beginning of the buffer.
+*/
{
  size_t to_line = 0;
  astr ms;

  do {
    if ((ms = minibuf_read(astr_new("Goto line: "), astr_new(""))) == NULL) {
      ok = FUNCALL(cancel);
      break;
    }
    if ((to_line = strtoul(astr_cstr(ms), NULL, 10)) == ULONG_MAX)
      ding();
  } while (to_line == ULONG_MAX);

  if (ok) {
    goto_line(to_line - 1);
    buf.pt.o = 0;
  }
}
END_DEFUN

DEFUN(beginning_of_buffer)
/*+
Move point to the beginning of the buffer.
+*/
{
  buf.pt = point_min(&buf);
  thisflag |= FLAG_DONE_CPCN | FLAG_NEED_RESYNC;
}
END_DEFUN

DEFUN(end_of_buffer)
/*+
Move point to the end of the buffer.
+*/
{
  buf.pt = point_max(&buf);
  thisflag |= FLAG_DONE_CPCN | FLAG_NEED_RESYNC;
}
END_DEFUN

DEFUN(edit_navigate_backward_char)
/*+
Move point left one character.
+*/
{
  if (!bolp())
    buf.pt.o--;
  else if (!bobp()) {
    thisflag |= FLAG_NEED_RESYNC;
    buf.pt.p = list_prev(buf.pt.p);
    buf.pt.n--;
    FUNCALL(end_of_line);
  } else
    ok = FALSE;
}
END_DEFUN

DEFUN(edit_navigate_forward_char)
/*+
Move point right one character.
+*/
{
  if (!eolp())
    buf.pt.o++;
  else if (!eobp()) {
    thisflag |= FLAG_NEED_RESYNC;
    buf.pt.p = list_next(buf.pt.p);
    buf.pt.n++;
    FUNCALL(beginning_of_line);
  } else
    ok = FALSE;
}
END_DEFUN

DEFUN(scroll_down)
/*+
Scroll text of current window downward near full screen.
+*/
{
  if (buf.pt.n > 0)
    ok = goto_line(buf.pt.n - win.eheight) ? TRUE : FALSE;
  else {
    minibuf_error(astr_new("Beginning of buffer"));
    ok = FALSE;
  }
}
END_DEFUN

DEFUN(scroll_up)
/*+
Scroll text of current window upward near full screen.
+*/
{
  if (buf.pt.n < buf.num_lines)
    ok = goto_line(buf.pt.n + win.eheight) ? TRUE : FALSE;
  else {
    minibuf_error(astr_new("End of buffer"));
    ok = FALSE;
  }
}
END_DEFUN
