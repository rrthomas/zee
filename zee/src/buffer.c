/* Buffer-oriented functions
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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"

/*
 * Allocate a new buffer structure and set the default local
 * variable values.
 * The allocation of the first empty line is done here to simplify
 * the code.
 */
static Buffer *buffer_new(void)
{
  Buffer *bp = (Buffer *)zmalloc(sizeof(Buffer));

  /* Allocate the lines. */
  bp->lines = line_new();
  bp->pt.p = list_first(bp->lines);

  /* Set the initial mark (needs limit marker to be set up). */
  bp->mark = marker_new(bp, point_min(bp));

  /* Set default EOL string. */
  bp->eol[0] = '\n';

  /* Allocate the variables list. */
  bp->vars = list_new();

  return bp;
}

/*
 * Free the buffer allocated memory.
 */
void free_buffer(Buffer *bp)
{
  line_delete(bp->lines);

  free_undo(bp);

  /* Free markers. */
  while (bp->markers)
    free_marker(bp->markers);

  /* Free the name and the filename. */
  free(bp->name);
  free(bp->filename);

  free(bp);
}

/*
 * Free all the allocated buffers (used at exit).
 */
void free_buffers(void)
{
  free_buffer(cur_bp);
}

/*
 * Allocate a new buffer and insert it into the buffer list.
 */
Buffer *create_buffer(const char *name)
{
  Buffer *bp;

  bp = buffer_new();
  set_buffer_name(bp, name);

  if (get_variable_bool("auto-fill-mode"))
    bp->flags ^= BFLAG_AUTOFILL;

  return bp;
}

/*
 * Set a new name for the buffer.
 */
void set_buffer_name(Buffer *bp, const char *name)
{
  free(bp->name);
  bp->name = zstrdup(name);
}

/*
 * Set a new filename (and a name) for the buffer.
 */
void set_buffer_filename(Buffer *bp, const char *filename)
{
  astr name = make_buffer_name(filename);

  free(bp->filename);
  bp->filename = zstrdup(filename);

  free(bp->name);
  bp->name = zstrdup(astr_cstr(name));
  astr_delete(name);
}

/*
 * Create a buffer name using the file name.
 */
astr make_buffer_name(const char *filename)
{
  const char *p;
  astr name = astr_new();

  if ((p = strrchr(filename, '/')) == NULL)
    p = filename;
  else
    ++p;
  astr_cpy_cstr(name, p);

  return name;
}

/*
 * Print an error message into the echo area and return TRUE
 * if the current buffer is readonly; otherwise return FALSE.
 */
int warn_if_readonly_buffer(void)
{
  if (cur_bp->flags & BFLAG_READONLY) {
    minibuf_error("Buffer is readonly: %s", cur_bp->name);
    return TRUE;
  } else
  return FALSE;
}

int warn_if_no_mark(void)
{
  assert(cur_bp->mark);
  if (!(cur_bp->flags & BFLAG_ANCHORED)) {
    minibuf_error("The mark is not active now");
    return TRUE;
  } else
    return FALSE;
}

/*
 * Calculate a region size and set the region structure.
 */
void calculate_region(Region *rp, Point from, Point to)
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
  if (!(cur_bp->flags & BFLAG_ANCHORED))
    return FALSE;

  calculate_region(rp, cur_bp->pt, cur_bp->mark->pt);
  return TRUE;
}

size_t calculate_buffer_size(Buffer *bp)
{
  Line *lp = list_next(bp->lines);
  size_t size = 0;

  if (lp == bp->lines)
    return 0;

  for (;;) {
    size += astr_len(lp->item);
    lp = list_next(lp);
    if (lp == bp->lines)
      break;
    ++size;
  }

  return size;
}

void anchor_mark(void)
{
  cur_bp->flags |= BFLAG_ANCHORED;
}

void weigh_mark(void)
{
  cur_bp->flags &= ~BFLAG_ANCHORED;
}

/*
 * Return a safe tab width for the given buffer.
 */
size_t tab_width(void)
{
  size_t t = get_variable_number("tab-width");
  return t ? t : 8;
}
