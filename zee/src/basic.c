/* Basic movement functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2007 Reuben Thomas.
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

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#include "main.h"
#include "extern.h"

/* Goal column to arrive at when `move_down/up_line'
   commands are used. */
static size_t cur_goalc;

DEF(move_start_line,
"\
Move the cursor to the beginning of the line.\
")
{
  buf->pt.o = 0;

  /* Set goalc to the beginning of line for next
     `move_next/previous_line' call. */
  thisflag |= FLAG_DONE_UPDOWN;
  cur_goalc = 0;
}
END_DEF

DEF(move_end_line,
"\
Move the cursor to the end of the line.\
")
{
  buf->pt.o = rblist_line_length(buf->lines, buf->pt.n);

  /* Change the `goalc' to the end of line for next
     `edit_move_next/previous_line' calls.  */
  thisflag |= FLAG_DONE_UPDOWN;
  cur_goalc = SIZE_MAX;
}
END_DEF

/*
 * Get the goal column. Take care of expanding tabulations.
 */
size_t get_goalc(void)
{
  return string_display_width(rblist_sub(rblist_line(buf->lines, buf->pt.n), 0, buf->pt.o));
}

DEF(move_previous_line,
"\
Move cursor vertically up one line.\n\
If there is no character in the target line exactly over the current column,\n\
the cursor is positioned after the character in that line which spans this\n\
column, or at the end of the line if it is not long enough.\
")
{
  if (buf->pt.n == 0)
    ok = false;
  else {
    thisflag |= FLAG_DONE_UPDOWN;

    if (!(lastflag & FLAG_DONE_UPDOWN))
      cur_goalc = get_goalc();

    buf->pt.n--;
    buf->pt.o = column_to_character(rblist_line(buf->lines, buf->pt.n), cur_goalc);
  }
}
END_DEF

DEF(move_next_line,
"\
Move cursor vertically down one line.\n\
If there is no character in the target line exactly under the current column,\n\
the cursor is positioned after the character in that line which spans this\n\
column, or at the end of the line if it is not long enough.\
")
{
  if (buf->pt.n == rblist_nl_count(buf->lines))
    ok = false;
  else {
    thisflag |= FLAG_DONE_UPDOWN;

    if (!(lastflag & FLAG_DONE_UPDOWN))
      cur_goalc = get_goalc();

    buf->pt.n++;
    buf->pt.o = column_to_character(rblist_line(buf->lines, buf->pt.n), cur_goalc);
  }
}
END_DEF

DEF_ARG(edit_goto_column,
"\
Read a number N and move the cursor to column number N.\n\
",
UINT(to_col, "Goto column: "))
{
  if (ok) {
    if (buf->pt.o > to_col)
      do {
        CMDCALL(move_previous_character);
      } while (ok && buf->pt.o > to_col);
    else if (buf->pt.o < to_col)
      do {
        CMDCALL(move_next_character);
      } while (ok && buf->pt.o < to_col);
  }
}
END_DEF

/*
 * Go to the given point.
 */
bool goto_point(Point pt)
{
  bool ok = goto_line(pt.n);
  if (ok) {
    CMDCALL_UINT(edit_goto_column, pt.o);
  }
  return ok;
}

/*
 * Go to the line `to_line', counting from 0. Point will end up in a
 * "random" column.
 */
bool goto_line(size_t to_line)
{
  bool ok = true;

  if (buf->pt.n > to_line) {
    do {
      CMDCALL(move_previous_line);
    } while (ok && buf->pt.n > to_line);
  } else if (buf->pt.n < to_line) {
    do {
      CMDCALL(move_next_line);
    }
    while (ok && buf->pt.n < to_line);
  }

  return ok;
}

DEF_ARG(edit_goto_line,
"\
Move the cursor to the beginning of the specified line.\n\
Line 1 is the beginning of the buffer.\
",
UINT(to_line, "Goto line: "))
{
  if (ok && to_line > 0) {
    goto_line(to_line - 1);
    buf->pt.o = 0;
  }
}
END_DEF

DEF(move_start_file,
"\
Move the cursor to the beginning of the file.\
")
{
  buf->pt = point_min(buf);
  thisflag |= FLAG_DONE_UPDOWN;
}
END_DEF

DEF(move_end_file,
"\
Move the cursor to the end of the file.\
")
{
  buf->pt = point_max(buf);
  thisflag |= FLAG_DONE_UPDOWN;
}
END_DEF

DEF(move_previous_character,
"\
Move the cursor left one character.\
")
{
  if (!bolp())
    buf->pt.o--;
  else if (!bobp()) {
    buf->pt.n--;
    CMDCALL(move_end_line);
  } else
    ok = false;
}
END_DEF

DEF(move_next_character,
"\
Move the cursor right one character.\
")
{
  if (!eolp())
    buf->pt.o++;
  else if (!eobp()) {
    buf->pt.n++;
    CMDCALL(move_start_line);
  } else
    ok = false;
}
END_DEF

DEF(move_previous_page,
"\
Scroll text of window downward near full screen.\
")
{
  if (buf->pt.n > 0)
    ok = goto_line(buf->pt.n > win.eheight ? buf->pt.n - win.eheight : 0) ? true : false;
  else {
    minibuf_error(rblist_from_string("Beginning of buffer"));
    ok = false;
  }
}
END_DEF

DEF(move_next_page,
"\
Scroll text of window upward near full screen.\
")
{
  if (buf->pt.n < rblist_nl_count(buf->lines))
    ok = goto_line(buf->pt.n + win.eheight) ? true : false;
  else {
    minibuf_error(rblist_from_string("End of buffer"));
    ok = false;
  }
}
END_DEF
