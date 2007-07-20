/* Point facility functions
   Copyright (c) 2004 David A. Capello.
   Copyright (c) 2005-2007 Reuben Thomas.
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

#include <stdlib.h>

#include "main.h"
#include "extern.h"

Point make_point(size_t lineno, size_t offset)
{
  Point pt;

  pt.n = lineno;
  pt.o = offset;

  return pt;
}

/*
 * Return the distance in chars between two points.
 */
// FIXME: Offset should be > size_t
ssize_t point_dist(Point pt1, Point pt2)
{
  // Subtract number of lines (== number of newlines)
  return rblist_line_to_start_pos(buf->lines, pt2.n) + pt2.o - rblist_line_to_start_pos(buf->lines, pt1.n) - pt1.o - (pt2.n - pt1.n);
}

/*
 * Return the number of lines between two points.
 */
size_t count_lines(Point pt1, Point pt2)
{
  return labs((ssize_t)pt2.n - (ssize_t)pt1.n);
}

void swap_point(Point *pt1, Point *pt2)
{
  Point pt0 = *pt1;
  *pt1 = *pt2;
  *pt2 = pt0;
}

Point point_min(Buffer *bp)
{
  (void)bp;
  Point pt;
  pt.n = 0;
  pt.o = 0;
  return pt;
}

Point point_max(Buffer *bp)
{
  Point pt;
  pt.n = bp->num_lines;
  pt.o = rblist_line_length(bp->lines, bp->pt.n);
  return pt;
}
