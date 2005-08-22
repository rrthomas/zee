/* Kill ring facility functions
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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "main.h"
#include "extern.h"

static astr kill_ring_text;

static void flush_kill_ring(void)
{
  astr_truncate(kill_ring_text, 0);
}

static int kill_line(void)
{
  assert(cur_bp);

  if (warn_if_readonly_buffer())
    return FALSE;

  if (!eolp()) {
    size_t len = astr_len(cur_bp->pt.p->item) - cur_bp->pt.o;
    astr_ncat(kill_ring_text, astr_char(cur_bp->pt.p->item, (ptrdiff_t)cur_bp->pt.o), (size_t)len);
    FUNCALL_ARG(delete_char, (int)len);

    thisflag |= FLAG_DONE_KILL;

    if (!bolp())
      return TRUE;
  }

  if (list_next(cur_bp->pt.p) != cur_bp->lines) {
    if (!FUNCALL(delete_char))
      return FALSE;

    astr_cat_char(kill_ring_text, '\n');

    thisflag |= FLAG_DONE_KILL;

    return TRUE;
  }

  minibuf_error("End of buffer");

  return FALSE;
}

DEFUN_INT("kill-line", kill_line)
/*+
Kill the rest of the current line; if no nonblanks there, kill thru newline.
With prefix argument, kill that many lines from point.
+*/
{
  int i;

  assert(cur_bp);

  if (!(lastflag & FLAG_DONE_KILL))
    flush_kill_ring();

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
  for (i = 0; i < uniarg; ++i)
    if (!kill_line()) {
      ok = FALSE;
      break;
    }
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0, FALSE);

  weigh_mark();
}
END_DEFUN

DEFUN_INT("kill-region", kill_region)
/*+
Kill between point and mark.
The text is deleted but saved in the kill ring.
The command C-y (yank) can retrieve it from there.
If the buffer is read-only, the text will not be deleted, but it will
be added to the kill ring anyway.  This means that
you can use the killing commands to copy text from a read-only buffer.
If the previous command was also a kill command,
the text killed this time appends to the text killed last time
to make one entry in the kill ring.
+*/
{
  Region r;

  assert(cur_bp);

  if (!(lastflag & FLAG_DONE_KILL))
    flush_kill_ring();

  if (!cur_bp->mark_anchored)
    ok = FUNCALL(kill_line);
  else {
    calculate_the_region(&r);

    if (cur_bp->flags & BFLAG_READONLY) {
      /* The buffer is read-only; save text in the kill buffer and
         complain. */
      astr as = copy_text_block(r.start, r.size);
      astr_cat(kill_ring_text, as);
      astr_delete(as);

      warn_if_readonly_buffer();
    } else {
      astr as;

      if (cur_bp->pt.p != r.start.p || r.start.o != cur_bp->pt.o)
        FUNCALL(exchange_point_and_mark);
      delete_nstring(r.size, &as);
      astr_cat_delete(kill_ring_text, as);
    }

    thisflag |= FLAG_DONE_KILL;
    weigh_mark();
  }
}
END_DEFUN

DEFUN_INT("copy-region", copy_region)
/*+
Copy the region to the kill ring.
+*/
{
  Region r;

  if (!(lastflag & FLAG_DONE_KILL))
    flush_kill_ring();

  if (warn_if_no_mark())
    ok = FALSE;
  else {
    astr as;

    calculate_the_region(&r);

    as = copy_text_block(r.start, r.size);
    astr_cat(kill_ring_text, as);
    astr_delete(as);

    thisflag |= FLAG_DONE_KILL;
    weigh_mark();
  }
}
END_DEFUN

DEFUN_INT("kill-word", kill_word)
/*+
Kill characters forward until encountering the end of a word.
With argument, do this that many times.
+*/
{
  assert(cur_bp);

  if (!(lastflag & FLAG_DONE_KILL))
    flush_kill_ring();

  if (warn_if_readonly_buffer())
    ok = FALSE;
  else {
    push_mark();
    undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
    FUNCALL_ARG(mark_word, uniarg);
    FUNCALL(kill_region);
    undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
    pop_mark();

    thisflag |= FLAG_DONE_KILL;

    minibuf_write("");	/* Don't write "Set mark" message.  */
  }
}
END_DEFUN

DEFUN_INT("backward-kill-word", backward_kill_word)
/*+
Kill characters backward until encountering the end of a word.
With argument, do this that many times.
+*/
{
  ok = FUNCALL_ARG(kill_word, (uniused == 0) ? -1 : -uniarg);
}
END_DEFUN

DEFUN_INT("kill-sexp", kill_sexp)
/*+
Kill the sexp (balanced expression) following the cursor.
With ARG, kill that many sexps after the cursor.
Negative arg -N means kill N sexps before the cursor.
+*/
{
  assert(cur_bp);

  if (!(lastflag & FLAG_DONE_KILL))
    flush_kill_ring();

  if (warn_if_readonly_buffer())
    ok = FALSE;
  else {
    push_mark();
    undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
    FUNCALL_ARG(mark_sexp, uniarg);
    FUNCALL(kill_region);
    undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
    pop_mark();

    thisflag |= FLAG_DONE_KILL;

    minibuf_write("");	/* Don't write "Set mark" message.  */
  }
}
END_DEFUN

DEFUN_INT("yank", yank)
/*+
Reinsert the last stretch of killed text.
More precisely, reinsert the stretch of killed text most recently
killed OR yanked.  Put point at end, and set mark at beginning.
+*/
{
  assert(cur_bp);

  ok = FALSE;

  if (astr_len(kill_ring_text) == 0)
    minibuf_error("Kill ring is empty");
  else if (!warn_if_readonly_buffer()) {
    set_mark_command();
    insert_nstring(kill_ring_text, FALSE);
    weigh_mark();
    ok = TRUE;
  }
}
END_DEFUN

void init_kill_ring(void)
{
  kill_ring_text = astr_new();
}

void free_kill_ring(void)
{
  astr_delete(kill_ring_text);
}
