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
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

#include "config.h"

#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zee.h"
#include "extern.h"

/* Goal-column to arrive when `prev/next-line' functions are used.  */
static int cur_goalc;

DEFUN_INT("beginning-of-line", beginning_of_line)
  /*+
    Move point to beginning of current line.
    +*/
{
  cur_bp->pt = line_beginning_position(uniarg);

  /* Change the `goalc' to the beginning of line for next
     `prev/next-line' calls.  */
  thisflag |= FLAG_DONE_CPCN;
  cur_goalc = 0;
}
END_DEFUN

DEFUN_INT("end-of-line", end_of_line)
  /*+
    Move point to end of current line.
    +*/
{
  cur_bp->pt = line_end_position(uniarg);

  /* Change the `goalc' to the end of line for next
     `prev/next-line' calls.  */
  thisflag |= FLAG_DONE_CPCN;
  cur_goalc = INT_MAX;
}
END_DEFUN

/*
 * Get the goal column.  Take care of expanding tabulations.
 */
size_t get_goalc_bp(Buffer *bp, Point pt)
{
  size_t col = 0, t = tab_width(bp), i;
  const char *sp = astr_cstr(pt.p->item);

  for (i = 0; i < pt.o; i++) {
    if (sp[i] == '\t')
      col |= t - 1;
    ++col;
  }

  return col;
}

size_t get_goalc_wp(Window *wp)
{
  return get_goalc_bp(wp->bp, window_pt(wp));
}

size_t get_goalc(void)
{
  return get_goalc_bp(cur_bp, cur_bp->pt);
}

/*
 * Go to the column `goalc'.  Take care of expanding
 * tabulations.
 */
static void goto_goalc(int goalc)
{
  int col = 0, t = tab_width(cur_bp), w;
  size_t i;
  const char *sp = astr_cstr(cur_bp->pt.p->item);

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

int previous_line(void)
{
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

DEFUN_INT("previous-line", previous_line)
  /*+
    Move cursor vertically up one line.
    If there is no character in the target line exactly over the current column,
    the cursor is positioned after the character in that line which spans this
    column, or at the end of the line if it is not long enough.
    +*/
{
  int i;

  if (uniarg < 0)
    ok = FUNCALL_ARG(next_line, -uniarg);
  else if (!bobp()) {
    for (i = 0; i < uniarg; i++)
      if (!previous_line()) {
        thisflag |= FLAG_DONE_CPCN;
        FUNCALL(beginning_of_line);
        break;
      }
  } else if (lastflag & FLAG_DONE_CPCN)
    thisflag |= FLAG_DONE_CPCN;
}
END_DEFUN

int next_line(void)
{
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

DEFUN_INT("next-line", next_line)
  /*+
    Move cursor vertically down one line.
    If there is no character in the target line exactly under the current column,
    the cursor is positioned after the character in that line which spans this
    column, or at the end of the line if it is not long enough.
    +*/
{
  if (uniarg < 0)
    ok = FUNCALL_ARG(previous_line, -uniarg);
  else if (!eobp()) {
    int i;
    for (i = 0; i < uniarg; i++)
      if (!next_line()) {
        int old = cur_goalc;
        thisflag |= FLAG_DONE_CPCN;
        FUNCALL(end_of_line);
        cur_goalc = old;
        break;
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
  char *ms;
  size_t to_char = 0;

  do {
    if ((ms = minibuf_read("Goto char: ", "")) == NULL) {
      ok = cancel();
      break;
    }
    if ((to_char = strtoul(ms, NULL, 10)) == ULONG_MAX)
      ding();
  } while (to_char == ULONG_MAX);

  if (ok) {
    size_t count;
    gotobob();
    for (count = 1; count < to_char; ++count)
      if (!forward_char())
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
  char *ms;
  size_t to_line = 0;

  do {
    if ((ms = minibuf_read("Goto line: ", "")) == NULL) {
      ok = cancel();
      break;
    }
    if ((to_line = strtoul(ms, NULL, 10)) == ULONG_MAX)
      ding();
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

int backward_char(void)
{
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

DEFUN_INT("backward-char", backward_char)
  /*+
    Move point left N characters (right if N is negative).
    On attempt to pass beginning or end of buffer, stop and signal error.
    +*/
{
  int i;

  if (uniarg < 0)
    ok = FUNCALL_ARG(forward_char, -uniarg);
  else
    for (i = 0; i < uniarg; i++)
      if (!backward_char()) {
        minibuf_error("Beginning of buffer");
        ok = FALSE;
        break;
      }
}
END_DEFUN

int forward_char(void)
{
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

DEFUN_INT("forward-char", forward_char)
  /*+
    Move point right N characters (left if N is negative).
    On reaching end of buffer, stop and signal error.
    +*/
{
  int i;

  if (uniarg < 0)
    ok = FUNCALL_ARG(backward_char, -uniarg);
  else
    for (i = 0; i < uniarg; i++)
      if (!forward_char()) {
        minibuf_error("End of buffer");
        ok = FALSE;
        break;
      }
}
END_DEFUN

int ngotoup(size_t n)
{
  for (; n > 0; n--)
    if (list_prev(cur_bp->pt.p) != cur_bp->lines)
      FUNCALL(previous_line);
    else
      return FALSE;

  return TRUE;
}

int ngotodown(size_t n)
{
  for (; n > 0; n--)
    if (list_next(cur_bp->pt.p) != cur_bp->lines)
      FUNCALL(next_line);
    else
      return FALSE;

  return TRUE;
}

int scroll_down(void)
{
  if (cur_bp->pt.n > 0)
    return ngotoup(cur_wp->eheight) ? TRUE : FALSE;
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
  int i;

  if (uniarg < 0)
    ok = FUNCALL_ARG(scroll_up, -uniarg);
  else
    for (i = 0; i < uniarg; i++)
      if (!scroll_down()) {
        ok = FALSE;
        break;
      }
}
END_DEFUN

int scroll_up(void)
{
  if (cur_bp->pt.n < cur_bp->num_lines)
    return ngotodown(cur_wp->eheight) ? TRUE : FALSE;
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
  int i;

  if (uniarg < 0)
    ok = FUNCALL_ARG(scroll_down, -uniarg);
  else
    for (i = 0; i < uniarg; i++)
      if (!scroll_up()) {
        ok = FALSE;
        break;
      }
}
END_DEFUN
