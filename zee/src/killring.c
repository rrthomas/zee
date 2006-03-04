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
#include "main.h"
#include "extern.h"


static astr killed_text;

static void clear_kill_buffer(void)
{
  killed_text = astr_new("");
}

DEF(edit_kill_line,
"\
Delete the current line.\
")
{
  astr as;

  if (!(lastflag & FLAG_DONE_KILL))
    clear_kill_buffer();

  if (warn_if_readonly_buffer())
    ok = FALSE;
  else {
    undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);

    if (!eolp()) {
      CMDCALL(move_start_line);
      astr_cat(killed_text, astr_sub(buf->pt.p->item, 0, (ptrdiff_t)astr_len(buf->pt.p->item)));
      delete_nstring(astr_len(buf->pt.p->item), &as);
      thisflag |= FLAG_DONE_KILL;
    }

    if (list_next(buf->pt.p) != buf->lines) {
      astr_cat(killed_text, astr_new("\n"));
      assert(CMDCALL(edit_delete_next_character));
      thisflag |= FLAG_DONE_KILL;
    }

    undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);
  }

  buf->flags &= ~BFLAG_ANCHORED;
}
END_DEF

DEF(edit_kill_selection,
"\
Kill between point and mark.\n\
The text is deleted but saved in the kill buffer.\n\
The command paste can retrieve it from there.\n\
If the buffer is read-only, the text will not be deleted, but it will\n\
be added to the kill buffer anyway.  This means that\n\
you can use the killing commands to copy text from a read-only buffer.\n\
If the previous command was also a kill command,\n\
the text killed this time appends to the text killed last time.\
")
{
  Region r;

  if (!(lastflag & FLAG_DONE_KILL))
    clear_kill_buffer();

  if (!(buf->flags & BFLAG_ANCHORED))
    ok = CMDCALL(edit_kill_line);
  else {
    assert(calculate_the_region(&r));

    if (buf->flags & BFLAG_READONLY)
      warn_if_readonly_buffer();
    else {
      astr as;

      if (buf->pt.p != r.start.p || r.start.o != buf->pt.o)
        CMDCALL(edit_select_other_end);
      delete_nstring(r.size, &as);
      astr_cat(killed_text, as);

      thisflag |= FLAG_DONE_KILL;
      buf->flags &= ~BFLAG_ANCHORED;
    }
  }
}
END_DEF

DEF(edit_copy,
"\
Copy the region to the kill buffer.\
")
{
  Region r;

  if (!(lastflag & FLAG_DONE_KILL))
    clear_kill_buffer();

  if (warn_if_no_mark())
    ok = FALSE;
  else {
    assert(calculate_the_region(&r));
    astr_cat(killed_text, copy_text_block(r.start, r.size));

    thisflag |= FLAG_DONE_KILL;
    buf->flags &= ~BFLAG_ANCHORED;
  }
}
END_DEF

static int kill_helper(Command cmd)
{
  int ok;

  if (!(lastflag & FLAG_DONE_KILL))
    clear_kill_buffer();

  if (warn_if_readonly_buffer())
    ok = FALSE;
  else {
    Marker *m = get_mark();
    undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);
    ok = cmd(NULL);
    if (ok)
      ok = CMDCALL(edit_kill_selection);
    undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);
    set_mark(m);
    remove_marker(m);

    thisflag |= FLAG_DONE_KILL;

    minibuf_clear();            /* Erase "Set mark" message. */
  }

  return ok;
}

DEF(edit_kill_word,
"\
Kill characters forward until encountering the end of a word.\
")
{
  ok = kill_helper(F_edit_select_word);
}
END_DEF

DEF(edit_kill_word_backward,
"\
Kill characters backward until encountering the end of a word.\
")
{
  ok = kill_helper(F_edit_select_word_backward);
}
END_DEF

DEF(edit_paste,
"\
Reinsert the stretch of killed text most recently killed.\n\
Set mark at beginning, and put point at end.\
")
{
  if (astr_len(killed_text) == 0) {
    minibuf_error(astr_new("Clipboard is empty"));
    ok = FALSE;
  } else if (!warn_if_readonly_buffer()) {
    CMDCALL(edit_select_on);
    insert_nstring(killed_text);
    buf->flags &= ~BFLAG_ANCHORED;
  }
}
END_DEF

void init_kill_ring(void)
{
  killed_text = astr_new("");
}
