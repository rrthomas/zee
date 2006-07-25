/* Undo facility functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2005-2006 Reuben Thomas.
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


/* This variable is set to true when the undo is in execution. */
static bool doing_undo = false;

/*
 * Save a reverse delta for doing undo.
 */
void undo_save(int type, Point pt, size_t arg1, size_t arg2)
{
  Undo *up = zmalloc(sizeof(Undo));

  up->type = type;
  up->pt = pt;
  up->unchanged = !(buf->flags & BFLAG_MODIFIED);

  if (type == UNDO_REPLACE_BLOCK) {
    up->text = copy_text_block(pt, arg1);
    up->size = arg2;
  }

  up->next = buf->last_undop;
  buf->last_undop = up;

  if (!doing_undo)
    buf->next_undop = up;
}

/*
 * Revert an action. Return the next undo entry.
 */
static Undo *revert_action(Undo *up)
{
  doing_undo = true;

  if (up->type == UNDO_END_SEQUENCE) {
    undo_save(UNDO_START_SEQUENCE, up->pt, 0, 0);
    up = up->next;
    while (up->type != UNDO_START_SEQUENCE)
      up = revert_action(up);
    undo_save(UNDO_END_SEQUENCE, up->pt, 0, 0);
    goto_point(up->pt);
    return up->next;
  }

  goto_point(up->pt);

  assert(up->type == UNDO_REPLACE_BLOCK);
  replace_nstring(up->size, NULL, up->text);

  doing_undo = false;

  /* If reverting this undo action leaves the buffer unchanged,
     unset the modified flag. */
  if (up->unchanged)
    buf->flags &= ~BFLAG_MODIFIED;

  return up->next;
}

DEF(edit_undo,
"\
Undo some previous changes.\n\
Repeat this command to undo more changes.\
")
{
  ok = false;

  if (warn_if_readonly_buffer());
  else if (buf->next_undop == NULL) {
    minibuf_error(rblist_from_string("No further undo information"));
    buf->next_undop = buf->last_undop;
  } else {
    buf->next_undop = revert_action(buf->next_undop);
    minibuf_write(rblist_from_string("Undo!"));
    ok = true;
  }
}
END_DEF

/*
 * Set unchanged flags to false except for the last undo action, which
 * is set to true.
 */
void undo_reset_unmodified(Undo *up)
{
  assert(up);
  up->unchanged = true;
  for (up = up->next; up; up = up->next)
    up->unchanged = false;
}
