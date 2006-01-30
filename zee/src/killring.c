/* Kill facility functions
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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "main.h"
#include "extern.h"

static astr killed_text;

static void flush_kill_buffer(void)
{
  astr_truncate(killed_text, 0);
}

static int kill_line(void)
{
  astr as;

  if (warn_if_readonly_buffer())
    return FALSE;

  if (!eolp()) {
    size_t len = astr_len(buf.pt.p->item) - buf.pt.o;

    astr_ncat(killed_text, astr_char(buf.pt.p->item, (ptrdiff_t)buf.pt.o), (size_t)len);
    delete_nstring(len, &as);
    astr_delete(as);

    thisflag |= FLAG_DONE_KILL;

    if (!bolp())
      return TRUE;
  }

  if (list_next(buf.pt.p) != buf.lines) {
    if (!FUNCALL(delete_char))
      return FALSE;

    astr_cat_cstr(killed_text, buf.eol);

    thisflag |= FLAG_DONE_KILL;

    return TRUE;
  }

  minibuf_error("End of buffer");

  return FALSE;
}

DEFUN("kill-line", kill_line)
/*+
Kill the rest of the current line; if no nonblanks there, kill thru newline.
+*/
{
  if (!(lastflag & FLAG_DONE_KILL))
    flush_kill_buffer();

  undo_save(UNDO_START_SEQUENCE, buf.pt, 0, 0, FALSE);
  if (!kill_line())
    ok = FALSE;
  undo_save(UNDO_END_SEQUENCE, buf.pt, 0, 0, FALSE);

  weigh_mark();
}
END_DEFUN

DEFUN("kill-region", kill_region)
/*+
Kill between point and mark.
The text is deleted but saved in the kill buffer.
The command paste can retrieve it from there.
If the buffer is read-only, the text will not be deleted, but it will
be added to the kill buffer anyway.  This means that
you can use the killing commands to copy text from a read-only buffer.
If the previous command was also a kill command,
the text killed this time appends to the text killed last time.
+*/
{
  Region r;

  if (!(lastflag & FLAG_DONE_KILL))
    flush_kill_buffer();

  if (!(buf.flags & BFLAG_ANCHORED))
    ok = FUNCALL(kill_line);
  else {
    calculate_the_region(&r);

    if (buf.flags & BFLAG_READONLY) {
      /* The buffer is read-only; save text in the kill buffer and
         complain. */
      astr as = copy_text_block(r.start, r.size);
      astr_cat_delete(killed_text, as);

      warn_if_readonly_buffer();
    } else {
      astr as;

      if (buf.pt.p != r.start.p || r.start.o != buf.pt.o)
        FUNCALL(exchange_point_and_mark);
      delete_nstring(r.size, &as);
      astr_cat_delete(killed_text, as);
    }

    thisflag |= FLAG_DONE_KILL;
    weigh_mark();
  }
}
END_DEFUN

DEFUN("copy", copy)
/*+
Copy the region to the kill buffer.
+*/
{
  Region r;

  if (!(lastflag & FLAG_DONE_KILL))
    flush_kill_buffer();

  if (warn_if_no_mark())
    ok = FALSE;
  else {
    astr as;

    calculate_the_region(&r);

    as = copy_text_block(r.start, r.size);
    astr_cat_delete(killed_text, as);

    thisflag |= FLAG_DONE_KILL;
    weigh_mark();
  }
}
END_DEFUN

static int kill_helper(Function func)
{
  int ok;

  if (!(lastflag & FLAG_DONE_KILL))
    flush_kill_buffer();

  if (warn_if_readonly_buffer())
    ok = FALSE;
  else {
    Marker *m = get_mark();
    undo_save(UNDO_START_SEQUENCE, buf.pt, 0, 0, FALSE);
    ok = func(0, 0, NULL);
    if (ok)
      ok = FUNCALL(kill_region);
    undo_save(UNDO_END_SEQUENCE, buf.pt, 0, 0, FALSE);
    set_mark(m);
    free_marker(m);

    thisflag |= FLAG_DONE_KILL;

    minibuf_write("");	/* Erase "Set mark" message. */
  }

  return ok;
}

DEFUN("kill-word", kill_word)
/*+
Kill characters forward until encountering the end of a word.
+*/
{
  ok = kill_helper(F_mark_word);
}
END_DEFUN

DEFUN("backward-kill-word", backward_kill_word)
/*+
Kill characters backward until encountering the end of a word.
+*/
{
  ok = kill_helper(F_mark_word_backward);
}
END_DEFUN

DEFUN("paste", paste)
/*+
Reinsert the stretch of killed text most recently killed.
Set mark at beginning, and put point at end.
+*/
{
  if (astr_len(killed_text) == 0) {
    minibuf_error("Kill ring is empty");
    ok = FALSE;
  } else if (!warn_if_readonly_buffer()) {
    FUNCALL(set_mark);
    insert_nstring(killed_text, buf.eol, FALSE);
    weigh_mark();
  }
}
END_DEFUN

void init_kill_ring(void)
{
  killed_text = astr_new();
}

void free_kill_ring(void)
{
  astr_delete(killed_text);
}
