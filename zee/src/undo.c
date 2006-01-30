/* Undo facility functions
   Copyright (c) 1997-2004 Sandro Sigala.  All rights reserved.

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

/* This variable is set to TRUE when the undo is in execution. */
static int doing_undo = FALSE;

/*
 * Free undo records for a buffer.
 */
void free_undo(Buffer *bp)
{
  Undo *up;

  assert(bp);

  up = bp->last_undop;
  while (up != NULL) {
    Undo *next_up = up->next;
    if (up->type == UNDO_REPLACE_BLOCK)
      astr_delete(up->text);
    free(up);
    up = next_up;
  }
}

/*
 * Save a reverse delta for doing undo.
 */
void undo_save(int type, Point pt, size_t arg1, size_t arg2, int intercalate)
{
  Undo *up = (Undo *)zmalloc(sizeof(Undo));

  up->type = type;
  up->pt = pt;
  up->unchanged = !(buf.flags & BFLAG_MODIFIED);

  if (type == UNDO_REPLACE_BLOCK) {
    up->text = copy_text_block(pt, arg1);
    up->size = arg2;
    up->intercalate = intercalate;
  }

  up->next = buf.last_undop;
  buf.last_undop = up;

  if (!doing_undo)
    buf.next_undop = up;
}

/*
 * Revert an action.  Return the next undo entry.
 */
static Undo *revert_action(Undo *up)
{
  astr as;

  doing_undo = TRUE;

  if (up->type == UNDO_END_SEQUENCE) {
    undo_save(UNDO_START_SEQUENCE, up->pt, 0, 0, FALSE);
    up = up->next;
    while (up->type != UNDO_START_SEQUENCE)
      up = revert_action(up);
    undo_save(UNDO_END_SEQUENCE, up->pt, 0, 0, FALSE);
    goto_point(up->pt);
    return up->next;
  }

  goto_point(up->pt);

  assert(up->type == UNDO_REPLACE_BLOCK);
  delete_nstring(up->size, &as);
  astr_delete(as);
  insert_nstring(up->text, "\n", up->intercalate);

  doing_undo = FALSE;

  /* If reverting this undo action leaves the buffer unchanged,
     unset the modified flag. */
  if (up->unchanged)
    buf.flags &= ~BFLAG_MODIFIED;

  return up->next;
}

DEFUN("undo", undo)
/*+
Undo some previous changes.
Repeat this command to undo more changes.
+*/
{
  ok = FALSE;

  if (warn_if_readonly_buffer());
  else if (buf.next_undop == NULL) {
    minibuf_error("No further undo information");
    buf.next_undop = buf.last_undop;
  } else {
    buf.next_undop = revert_action(buf.next_undop);
    minibuf_write("Undo!");
    ok = TRUE;
  }
}
END_DEFUN
