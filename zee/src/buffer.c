/* Buffer-oriented functions
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

#include <stdbool.h>

#include "config.h"
#include "main.h"
#include "extern.h"


/*
 * Allocate and initialize new buffer structure.
 */
void buffer_new(void)
{
  buf = zmalloc(sizeof(Buffer));
  buf->lines = rblist_empty;
  buf->mark = marker_new(point_min(buf));
  if (get_variable_bool("wrap_mode")) {
    buf->flags ^= BFLAG_WRAP_MODE;
  }
}

/*
 * Print an error message in the minibuffer and return true
 * if the buffer is read-only; otherwise return false.
 */
bool warn_if_readonly_buffer(void)
{
  if (buf->flags & BFLAG_READONLY) {
    minibuf_error(rblist_from_string("Buffer is readonly"));
    return true;
  } else {
    return false;
  }
}

/*
 * Print an error message in the minibuffer and return true
 * if the buffer has no mark; otherwise return false.
 */
bool warn_if_no_mark(void)
{
  assert(buf->mark);
  if (!(buf->flags & BFLAG_ANCHORED)) {
    minibuf_error(rblist_from_string("The mark is not active now"));
    return true;
  } else {
    return false;
  }
}

/*
 * Calculate a region size and set the region structure.
 */
static void region_size(Region *rp, Point from, Point to)
{
  if (point_dist(from, to) < 0) { // point before mark
    rp->start = from;
    rp->end = to;
  } else { // mark before point
    rp->start = to;
    rp->end = from;
  }

  rp->size = point_dist(rp->start, rp->end);
}

/*
 * Calculate the region size between point and mark and set the region
 * structure.
 */
bool calculate_the_region(Region *rp)
{
  if (!(buf->flags & BFLAG_ANCHORED)) {
    return false;
  }

  region_size(rp, buf->pt, buf->mark->pt);
  return true;
}

/*
 * Return a safe tab width.
 */
size_t tab_width(void)
{
  size_t t = get_variable_number("tab_width");
  return t ? t : 8;
}

/*
 * Return a safe indent width.
 */
size_t indent_width(void)
{
  size_t t = get_variable_number("indent_width");
  return t ? t : 1;
}

/*
 * Return a string containing `size' characters from point `start'.
 */
rblist copy_text_block(Point start, size_t size)
{
  return rblist_sub(buf->lines, rblist_line_to_start_pos(buf->lines, start.n) + start.o, size);
}
