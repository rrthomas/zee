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

  return bp;
}

/*
 * Free the buffer allocated memory.
 */
void free_buffer(Buffer *bp)
{
  Undo *up, *next_up;

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
  Buffer *bp, *next;

  for (bp = head_bp; bp != NULL; bp = next) {
    next = bp->next;
    free_buffer(bp);
  }
}

/*
 * Allocate a new buffer and insert it into the buffer list.
 */
Buffer *create_buffer(const char *name)
{
  Buffer *bp;

  bp = buffer_new();
  set_buffer_name(bp, name);

  bp->next = head_bp;
  head_bp = bp;

  if (lookup_bool_variable("auto-fill-mode"))
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
 * Search for a buffer named `name'.  If not buffer is found and
 * the `cflag' variable is set to `TRUE', create a new buffer.
 */
Buffer *find_buffer(const char *name, int cflag)
{
  Buffer *bp;

  for (bp = head_bp; bp != NULL; bp = bp->next)
    if (!strcmp(bp->name, name))
      return bp;

  /*
   * Create flag not specified, return NULL.
   */
  if (!cflag)
    return NULL;

  /*
   * No buffer found with the specified name, then
   * create one.
   */
  bp = create_buffer(name);

  return bp;
}

/*
 * Find the next buffer in buffer list.
 */
Buffer *get_next_buffer(void)
{
  assert(cur_bp);
  return cur_bp->next ? cur_bp->next : head_bp;
}

/*
 * Create a buffer name using the file name.
 */
astr make_buffer_name(const char *filename)
{
  const char *p;
  astr name = astr_new();
  size_t i;

  if ((p = strrchr(filename, '/')) == NULL)
    p = filename;
  else
    ++p;
  astr_cpy_cstr(name, p);

  /* This loop will terminate at the latest after checking all the
     buffers. */
  for (i = 2; find_buffer(astr_cstr(name), FALSE) != NULL; i++)
    astr_afmt(name, "%s<%d>", p, i);

  return name;
}

/* Move the selected buffer to head.  */

static void move_buffer_to_head(Buffer *bp)
{
  Buffer *it, *prev = NULL;

  for (it = head_bp; it; it = it->next) {
    if (bp == it) {
      if (prev) {
        prev->next = bp->next;
        bp->next = head_bp;
        head_bp = bp;
      }
      break;
    }
    prev = it;
  }
}

/*
 * Switch to the specified buffer.
 */
void switch_to_buffer(Buffer *bp)
{
  /* The buffer is the current buffer; return safely.  */
  if (cur_bp == bp)
    return;

  /* Set current buffer.  */
  cur_wp->bp = cur_bp = bp;

  /* Move the buffer to head.  */
  move_buffer_to_head(bp);

  thisflag |= FLAG_NEED_RESYNC;
}

/*
 * Print an error message into the echo area and return TRUE
 * if the current buffer is readonly; otherwise return FALSE.
 */
int warn_if_readonly_buffer(void)
{
  assert(cur_bp);
  if (cur_bp->flags & BFLAG_READONLY) {
    minibuf_error("Buffer is readonly: %s", cur_bp->name);
    return TRUE;
  } else
  return FALSE;
}

int warn_if_no_mark(void)
{
  assert(cur_bp);
  assert(cur_bp->mark);
  if (!cur_bp->mark_anchored) {
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
  assert(cur_bp);
  if (!cur_bp->mark_anchored)
    return FALSE;

  calculate_region(rp, cur_bp->pt, cur_bp->mark->pt);
  return TRUE;
}

/*
 * Set the specified buffer temporary flag and move the buffer
 * to the end of the buffer list.
 */
void set_temporary_buffer(Buffer *bp)
{
  Buffer *bp0;

  if (bp == head_bp) {
    if (head_bp->next == NULL)
      return;
    head_bp = head_bp->next;
  } else if (bp->next == NULL)
    return;

  for (bp0 = head_bp; bp0 != NULL; bp0 = bp0->next)
    if (bp0->next == bp) {
      bp0->next = bp0->next->next;
      break;
    }

  for (bp0 = head_bp; bp0->next != NULL; bp0 = bp0->next)
    ;

  bp0->next = bp;
  bp->next = NULL;
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
  assert(cur_bp);
  cur_bp->mark_anchored = TRUE;
}

void weigh_mark(void)
{
  assert(cur_bp);
  cur_bp->mark_anchored = FALSE;
}

/*
 * Return a safe tab width for the given buffer.
 */
size_t tab_width(Buffer *bp)
{
  size_t t;

  assert(bp);

  t = get_variable_number_bp(bp, "tab-width");

  return t ? t : 1;
}
