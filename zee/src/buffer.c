/* Buffer-oriented functions
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
#include "main.h"
#include "extern.h"


/*
 * Allocate a new buffer structure and set the default local
 * variable values.
 * The allocation of the first empty line is done here to simplify
 * the code.
 */
void buffer_new(void)
{
  buf = zmalloc(sizeof(Buffer));

  /* Allocate the lines. */
  buf->lines = line_new();
  buf->pt.p = list_first(buf->lines);

  /* Set the initial mark (needs limit marker to be set up). */
  buf->mark = marker_new(point_min(buf));

  if (get_variable_bool(astr_new("wrap_mode")))
    buf->flags ^= BFLAG_AUTOFILL;
}

/*
 * Print an error message into the echo area and return TRUE
 * if the current buffer is readonly; otherwise return FALSE.
 */
int warn_if_readonly_buffer(void)
{
  if (buf->flags & BFLAG_READONLY) {
    minibuf_error(astr_new("Buffer is readonly"));
    return TRUE;
  } else
  return FALSE;
}

int warn_if_no_mark(void)
{
  assert(buf->mark);
  if (!(buf->flags & BFLAG_ANCHORED)) {
    minibuf_error(astr_new("The mark is not active now"));
    return TRUE;
  } else
    return FALSE;
}

/*
 * Calculate a region size and set the region structure.
 */
static void region_size(Region *rp, Point from, Point to)
{
  if (cmp_point(from, to) < 0) {
    /* The point is before the mark. */
    rp->start = from;
    rp->end = to;
  } else {
    /* The mark is before the point. */
    rp->start = to;
    rp->end = from;
  }

  rp->size = point_dist(rp->start, rp->end);
  rp->num_lines = count_lines(rp->start, rp->end);
}

/*
 * Calculate the region size between point and mark and set the region
 * structure.
 */
int calculate_the_region(Region *rp)
{
  if (!(buf->flags & BFLAG_ANCHORED))
    return FALSE;

  region_size(rp, buf->pt, buf->mark->pt);
  return TRUE;
}

/*
 * Return a safe tab width for the given buffer.
 */
size_t tab_width(void)
{
  size_t t = get_variable_number(astr_new("tab_width"));
  return t ? t : 8;
}

/*
 * Return a string containing `size' characters from point `start'.
 */
astr copy_text_block(Point start, size_t size)
{
  size_t n = buf->pt.n, i;
  astr as = astr_new("");
  Line *lp = buf->pt.p;

  /* Have to do a linear search through the buffer to find the start of the
   * region. Doesn't matter where we start. Starting at 'buf->pt' is a good
   * heuristic.
   */
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
      astr_cat(as, astr_new("\n"));
      lp = list_next(lp);
      i = 0;
    }
  }

  return as;
}
