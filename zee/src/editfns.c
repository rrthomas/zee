/* Useful editing functions
   Copyright (c) 2004 David A. Capello.
   Copyright (c) 2004-2005 Reuben Thomas.
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
#include <stdlib.h>
#include <ctype.h>

#include "main.h"
#include "extern.h"

static list mark_ring = NULL;	/* Mark ring. */

/* Push the current mark in the mark-ring. */
void push_mark(void)
{
  if (!mark_ring)
    mark_ring = list_new();

  /* Save the mark.  */
  assert(cur_bp); /* FIXME: Check this assumption. */
  assert(cur_bp->mark);
  list_append(mark_ring, marker_new(cur_bp->mark->bp, cur_bp->mark->pt));
}

/* Pop a mark from the mark-ring a put it as current mark. */
void pop_mark(void)
{
  Marker *m = list_last(mark_ring)->item;

  /* Replace the mark. */
  assert(m->bp->mark);
  free_marker(m->bp->mark);

  m->bp->mark = (m->pt.p) ? marker_new(m->bp, m->pt) : NULL;

  list_betail(mark_ring);
  free_marker(m);
}

/* Set the mark to the point position. */
void set_mark(void)
{
  assert(cur_bp); /* FIXME: Check this assumption. */
  assert(cur_bp->mark);
  move_marker(cur_bp->mark, cur_bp, cur_bp->pt);
}

int is_empty_line(void)
{
  assert(cur_bp);
  return astr_len(cur_bp->pt.p->item) == 0;
}

int is_blank_line(void)
{
  size_t c;
  assert(cur_bp);
  for (c = 0; c < astr_len(cur_bp->pt.p->item); c++)
    if (!isspace(*astr_char(cur_bp->pt.p->item, (ptrdiff_t)c)))
      return FALSE;
  return TRUE;
}

int char_after(Point *pt)
{
  if (eobp())
    return '\0';
  else if (eolp())
    return '\n';
  else
    return *astr_char(pt->p->item, (ptrdiff_t)pt->o);
}

int char_before(Point *pt)
{
  if (bobp())
    return '\0';
  else if (bolp())
    return '\n';
  else
    return *astr_char(pt->p->item, (ptrdiff_t)(pt->o - 1));
}

/* This function returns the character following point in the current
   buffer. */
int following_char(void)
{
  assert(cur_bp);
  return char_after(&cur_bp->pt);
}

/* This function returns the character preceding point in the current
   buffer. */
int preceding_char(void)
{
  assert(cur_bp);
  return char_before(&cur_bp->pt);
}

/* This function returns TRUE if point is at the beginning of the
   buffer. */
int bobp(void)
{
  assert(cur_bp);
  return (list_prev(cur_bp->pt.p) == cur_bp->lines &&
          cur_bp->pt.o == 0);
}

/* This function returns TRUE if point is at the end of the
   buffer. */
int eobp(void)
{
  assert(cur_bp);
  return (list_next(cur_bp->pt.p) == cur_bp->lines &&
          cur_bp->pt.o == astr_len(cur_bp->pt.p->item));
}

/* Returns TRUE if point is at the beginning of a line. */
int bolp(void)
{
  assert(cur_bp);
  return cur_bp->pt.o == 0;
}

/* Returns TRUE if point is at the end of a line. */
int eolp(void)
{
  assert(cur_bp);
  return cur_bp->pt.o == astr_len(cur_bp->pt.p->item);
}
