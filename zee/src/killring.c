/* Kill facility functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2007 Reuben Thomas.
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


static rblist killed_text;

static void clear_kill_buffer(void)
{
  killed_text = rblist_empty;
}

DEF(edit_kill_line,
"\
Delete the current line.\
")
{
  rblist as;

  if (!(lastflag & FLAG_DONE_KILL))
    clear_kill_buffer();

  if (warn_if_readonly_buffer())
    ok = false;
  else {
    undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);

    if (!eolp()) {
      CMDCALL(move_start_line);
      killed_text = rblist_concat(killed_text,
                                  rblist_sub(buf->lines, rblist_line_to_start_pos(buf->lines, buf->pt.n),
                                             rblist_line_length(buf->lines, buf->pt.n)));
      replace_nstring(rblist_line_length(buf->lines, buf->pt.n), &as, NULL);
      thisflag |= FLAG_DONE_KILL;
    }

    if (buf->pt.n != rblist_nl_count(buf->lines) - 1) {
      killed_text = rblist_concat(killed_text, rblist_from_string("\n"));
      assert(CMDCALL(edit_delete_next_character));
      thisflag |= FLAG_DONE_KILL;
    }

    undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);
  }

  buf->flags &= ~BFLAG_ANCHORED;
}
END_DEF

/* Copy selection to the kill buffer, appending if the last command
   was also a kill. */
static int copy_selection(void)
{
  if (!(lastflag & FLAG_DONE_KILL))
    clear_kill_buffer();

  if (warn_if_no_mark())
    return false;
  else {
    Region r;

    assert(calculate_the_region(&r));
    killed_text = rblist_concat(killed_text, copy_text_block(r.start, r.size));

    thisflag |= FLAG_DONE_KILL;
  }

  return true;
}

DEF(edit_kill_selection,
"\
Kill between point and mark.\n\
The text is deleted, unless the buffer is read-only, and saved in the\n\
kill buffer; the `edit_paste' command retrieves it.\n\
If the previous command was also a kill command,\n\
the text killed this time appends to the text killed last time.\
")
{
  if (copy_selection()) {
    if (buf->flags & BFLAG_READONLY)
      warn_if_readonly_buffer();
    else {
      Region r;
      assert(calculate_the_region(&r));
      if (buf->pt.n != r.start.n || r.start.o != buf->pt.o)
        CMDCALL(edit_select_other_end);
      replace_nstring(r.size, NULL, NULL);
    }
  }
}
END_DEF

DEF(edit_copy,
"\
Copy the region to the kill buffer.\
")
{
  if (copy_selection())
    buf->flags &= ~BFLAG_ANCHORED;
}
END_DEF

static bool kill_helper(Command cmd)
{
  bool ok;

  if (!(lastflag & FLAG_DONE_KILL))
    clear_kill_buffer();

  if (warn_if_readonly_buffer())
    ok = false;
  else {
    Marker *m = get_mark();
    undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);
    CMDCALL(edit_select_on);
    if ((ok = cmd(NULL)))
      ok = CMDCALL(edit_kill_selection);
    undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);
    set_mark(m);
    remove_marker(m);

    thisflag |= FLAG_DONE_KILL;

    minibuf_clear();            // Erase "Set mark" message.
  }

  return ok;
}

DEF(edit_kill_word,
"\
Kill characters forward until encountering the end of a word.\
")
{
  ok = kill_helper(F_move_next_word);
}
END_DEF

DEF(edit_kill_word_backward,
"\
Kill characters backward until encountering the end of a word.\
")
{
  ok = kill_helper(F_move_previous_word);
}
END_DEF

DEF(edit_paste,
"\
Reinsert the stretch of killed text most recently killed.\n\
Set mark at beginning, and put point at end.\
")
{
  if (rblist_length(killed_text) == 0) {
    minibuf_error(rblist_from_string("Clipboard is empty"));
    ok = false;
  } else if (!warn_if_readonly_buffer()) {
    CMDCALL(edit_select_on);
    replace_nstring(0, NULL, killed_text);
    buf->flags &= ~BFLAG_ANCHORED;
  }
}
END_DEF

void init_kill_ring(void)
{
  killed_text = rblist_empty;
}
