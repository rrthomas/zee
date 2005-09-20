/* Basic movement functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2005 Reuben Thomas.
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
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"

/* Goal column to arrive at when `edit-navigate-down/up-line' functions are
   used.  */
static int cur_goalc;

DEFUN_INT("beginning-of-line", beginning_of_line)
/*+
Move point to beginning of current line.
+*/
{
  assert(cur_bp);
  cur_bp->pt = line_beginning_position();

  /* Change the `goalc' to the beginning of line for next
     `edit-navigate-down/up-line' calls.  */
  thisflag |= FLAG_DONE_CPCN;
  cur_goalc = 0;
}
END_DEFUN

DEFUN_INT("end-of-line", end_of_line)
/*+
Move point to end of current line.
+*/
{
  assert(cur_bp);
  cur_bp->pt = line_end_position();

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
  const char *sp = astr_cstr(cur_bp->pt.p->item);

  for (i = 0; i < cur_bp->pt.o; i++) {
    if (sp[i] == '\t')
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
  const char *sp;

  assert(cur_bp);
  t = tab_width();
  sp = astr_cstr(cur_bp->pt.p->item);

  for (i = 0; i < astr_len(cur_bp->pt.p->item); i++) {
    if (col == goalc)
      break;
    else if (sp[i] == '\t') {
      for (w = t - col % t; w > 0; w--)
        if (++col == goalc)
          break;
    } else
      ++col;
  }

  cur_bp->pt.o = i;
}

int edit_navigate_up_line(void)
{
  assert(cur_bp);
  if (list_prev(cur_bp->pt.p) != cur_bp->lines) {
    thisflag |= FLAG_DONE_CPCN | FLAG_NEED_RESYNC;

    if (!(lastflag & FLAG_DONE_CPCN))
      cur_goalc = get_goalc();

    cur_bp->pt.p = list_prev(cur_bp->pt.p);
    cur_bp->pt.n--;

    goto_goalc(cur_goalc);

    return TRUE;
  }

  return FALSE;
}

DEFUN_INT("edit-navigate-up-line", edit_navigate_up_line)
/*+
Move cursor vertically up one line.
If there is no character in the target line exactly over the current column,
the cursor is positioned after the character in that line which spans this
column, or at the end of the line if it is not long enough.
+*/
{
  if (!bobp()) {
    if (!edit_navigate_up_line()) {
      thisflag |= FLAG_DONE_CPCN;
      FUNCALL(beginning_of_line);
    }
  } else if (lastflag & FLAG_DONE_CPCN)
    thisflag |= FLAG_DONE_CPCN;
}
END_DEFUN

int edit_navigate_down_line(void)
{
  assert(cur_bp);
  if (list_next(cur_bp->pt.p) != cur_bp->lines) {
    thisflag |= FLAG_DONE_CPCN | FLAG_NEED_RESYNC;

    if (!(lastflag & FLAG_DONE_CPCN))
      cur_goalc = get_goalc();

    cur_bp->pt.p = list_next(cur_bp->pt.p);
    cur_bp->pt.n++;

    goto_goalc(cur_goalc);

    return TRUE;
  }

  return FALSE;
}

DEFUN_INT("edit-navigate-down-line", edit_navigate_down_line)
/*+
Move cursor vertically down one line.
If there is no character in the target line exactly under the current column,
the cursor is positioned after the character in that line which spans this
column, or at the end of the line if it is not long enough.
+*/
{
  if (!eobp()) {
    if (!edit_navigate_down_line()) {
      int old = cur_goalc;
      thisflag |= FLAG_DONE_CPCN;
      FUNCALL(end_of_line);
      cur_goalc = old;
    }
  } else if (lastflag & FLAG_DONE_CPCN)
    thisflag |= FLAG_DONE_CPCN;
}
END_DEFUN

/*
 * Go to the line `to_line', counting from 0.  Point will end up in
 * "random" column.
 */
void goto_line(size_t to_line)
{
  assert(cur_bp);
  if (cur_bp->pt.n > to_line)
    ngotoup(cur_bp->pt.n - to_line);
  else if (cur_bp->pt.n < to_line)
    ngotodown(to_line - cur_bp->pt.n);
}

DEFUN_INT("goto-char", goto_char)
/*+
Read a number N and move the cursor to character number N.
Position 1 is the beginning of the buffer.
+*/
{
  size_t to_char = 0;
  astr ms;

  do {
    if ((ms = minibuf_read("Goto char: ", "")) == NULL) {
      ok = cancel();
      break;
    }
    if ((to_char = strtoul(astr_cstr(ms), NULL, 10)) == ULONG_MAX)
      ding();
    astr_delete(ms);
  } while (to_char == ULONG_MAX);

  if (ok) {
    size_t count;
    gotobob();
    for (count = 1; count < to_char; ++count)
      if (!edit_navigate_forward_char())
        break;
  }
}
END_DEFUN

DEFUN_INT("goto-line", goto_line)
/*+
Move cursor to the beginning of the specified line.
Line 1 is the beginning of the buffer.
+*/
{
  size_t to_line = 0;
  astr ms;

  assert(cur_bp);

  do {
    if ((ms = minibuf_read("Goto line: ", "")) == NULL) {
      ok = cancel();
      break;
    }
    if ((to_line = strtoul(astr_cstr(ms), NULL, 10)) == ULONG_MAX)
      ding();
    astr_delete(ms);
  } while (to_line == ULONG_MAX);

  if (ok) {
    goto_line(to_line - 1);
    cur_bp->pt.o = 0;
  }
}
END_DEFUN

/*
 * Move point to the beginning of the buffer; do not touch the mark.
 */
void gotobob(void)
{
  assert(cur_bp);
  cur_bp->pt = point_min(cur_bp);
  thisflag |= FLAG_DONE_CPCN | FLAG_NEED_RESYNC;
}

DEFUN_INT("beginning-of-buffer", beginning_of_buffer)
/*+
Move point to the beginning of the buffer; leave mark at previous position.
+*/
{
  set_mark_command();
  gotobob();
}
END_DEFUN

/*
 * Move point to the end of the buffer; do not touch the mark.
 */
void gotoeob(void)
{
  assert(cur_bp);
  cur_bp->pt = point_max(cur_bp);
  thisflag |= FLAG_DONE_CPCN | FLAG_NEED_RESYNC;
}

DEFUN_INT("end-of-buffer", end_of_buffer)
/*+
Move point to the end of the buffer; leave mark at previous position.
+*/
{
  set_mark_command();
  gotoeob();
}
END_DEFUN

int edit_navigate_backward_char(void)
{
  assert(cur_bp);
  if (!bolp()) {
    cur_bp->pt.o--;
    return TRUE;
  } else if (!bobp()) {
    thisflag |= FLAG_NEED_RESYNC;
    cur_bp->pt.p = list_prev(cur_bp->pt.p);
    cur_bp->pt.n--;
    FUNCALL(end_of_line);
    return TRUE;
  }

  return FALSE;
}

DEFUN_INT("edit-navigate-backward-char", edit_navigate_backward_char)
/*+
Move point left one character.
On attempt to pass beginning or end of buffer, stop and signal error.
+*/
{
  if (!edit_navigate_backward_char()) {
    minibuf_error("Beginning of buffer");
    ok = FALSE;
  }
}
END_DEFUN

int edit_navigate_forward_char(void)
{
  assert(cur_bp);
  if (!eolp()) {
    cur_bp->pt.o++;
    return TRUE;
  } else if (!eobp()) {
    thisflag |= FLAG_NEED_RESYNC;
    cur_bp->pt.p = list_next(cur_bp->pt.p);
    cur_bp->pt.n++;
    FUNCALL(beginning_of_line);
    return TRUE;
  }

  return FALSE;
}

DEFUN_INT("edit-navigate-forward-char", edit_navigate_forward_char)
/*+
Move point right one character.
On reaching end of buffer, stop and signal error.
+*/
{
  if (!edit_navigate_forward_char()) {
    minibuf_error("End of buffer");
    ok = FALSE;
  }
}
END_DEFUN

int ngotoup(size_t n)
{
  assert(cur_bp);
  for (; n > 0; n--)
    if (list_prev(cur_bp->pt.p) != cur_bp->lines)
      FUNCALL(edit_navigate_up_line);
    else
      return FALSE;

  return TRUE;
}

int ngotodown(size_t n)
{
  assert(cur_bp);
  for (; n > 0; n--)
    if (list_next(cur_bp->pt.p) != cur_bp->lines)
      FUNCALL(edit_navigate_down_line);
    else
      return FALSE;

  return TRUE;
}

int scroll_down(void)
{
  assert(cur_bp);
  if (cur_bp->pt.n > 0)
    return ngotoup(win.eheight) ? TRUE : FALSE;
  else {
    minibuf_error("Beginning of buffer");
    return FALSE;
  }
}

DEFUN_INT("scroll-down", scroll_down)
/*+
Scroll text of current window downward near full screen.
+*/
{
  if (!scroll_down())
    ok = FALSE;
}
END_DEFUN

int scroll_up(void)
{
  assert(cur_bp);
  if (cur_bp->pt.n < cur_bp->num_lines)
    return ngotodown(win.eheight) ? TRUE : FALSE;
  else {
    minibuf_error("End of buffer");
    return FALSE;
  }
}

DEFUN_INT("scroll-up", scroll_up)
/*+
Scroll text of current window upward near full screen.
+*/
{
  if (!scroll_up())
    ok = FALSE;
}
END_DEFUN
