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

static void flush_kill_buffer(void)
{
  killed_text = astr_new("");
}

static int kill_line(void)
{
  astr as;

  if (warn_if_readonly_buffer())
    return FALSE;

  if (!eolp()) {
    size_t len = astr_len(buf->pt.p->item) - buf->pt.o;

    astr_cat(killed_text, astr_sub(buf->pt.p->item, (ptrdiff_t)buf->pt.o, (ptrdiff_t)astr_len(buf->pt.p->item)));
    delete_nstring(len, &as);

    thisflag |= FLAG_DONE_KILL;

    if (!bolp())
      return TRUE;
  }

  if (list_next(buf->pt.p) != buf->lines) {
    if (!CMDCALL(delete_char))
      return FALSE;

    astr_cat(killed_text, astr_new("\n"));

    thisflag |= FLAG_DONE_KILL;

    return TRUE;
  }

  minibuf_error(astr_new("End of buffer"));

  return FALSE;
}

DEF(kill_line,
"\
Kill the rest of the current line; if no nonblanks there, kill thru newline.\
")
{
  if (!(lastflag & FLAG_DONE_KILL))
    flush_kill_buffer();

  undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);
  if (!kill_line())
    ok = FALSE;
  undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);

  weigh_mark();
}
END_DEF

DEF(kill_region,
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
    flush_kill_buffer();

  if (!(buf->flags & BFLAG_ANCHORED))
    ok = CMDCALL(kill_line);
  else {
    assert(calculate_the_region(&r));

    if (buf->flags & BFLAG_READONLY) {
      /* The buffer is read-only; save text in the kill buffer and
         complain. */
      astr_cat(killed_text, copy_text_block(r.start, r.size));

      warn_if_readonly_buffer();
    } else {
      astr as;

      if (buf->pt.p != r.start.p || r.start.o != buf->pt.o)
        CMDCALL(exchange_point_and_mark);
      delete_nstring(r.size, &as);
      astr_cat(killed_text, as);
    }

    thisflag |= FLAG_DONE_KILL;
    weigh_mark();
  }
}
END_DEF

DEF(copy,
"\
Copy the region to the kill buffer.\
")
{
  Region r;

  if (!(lastflag & FLAG_DONE_KILL))
    flush_kill_buffer();

  if (warn_if_no_mark())
    ok = FALSE;
  else {
    assert(calculate_the_region(&r));
    astr_cat(killed_text, copy_text_block(r.start, r.size));

    thisflag |= FLAG_DONE_KILL;
    weigh_mark();
  }
}
END_DEF

static int kill_helper(Command cmd)
{
  int ok;

  if (!(lastflag & FLAG_DONE_KILL))
    flush_kill_buffer();

  if (warn_if_readonly_buffer())
    ok = FALSE;
  else {
    Marker *m = get_mark();
    undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);
    ok = cmd(NULL);
    if (ok)
      ok = CMDCALL(kill_region);
    undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);
    set_mark(m);
    remove_marker(m);

    thisflag |= FLAG_DONE_KILL;

    minibuf_clear();            /* Erase "Set mark" message. */
  }

  return ok;
}

DEF(kill_word,
"\
Kill characters forward until encountering the end of a word.\
")
{
  ok = kill_helper(F_mark_word);
}
END_DEF

DEF(backward_kill_word,
"\
Kill characters backward until encountering the end of a word.\
")
{
  ok = kill_helper(F_mark_word_backward);
}
END_DEF

DEF(paste,
"\
Reinsert the stretch of killed text most recently killed.\n\
Set mark at beginning, and put point at end.\
")
{
  if (astr_len(killed_text) == 0) {
    minibuf_error(astr_new("Kill ring is empty"));
    ok = FALSE;
  } else if (!warn_if_readonly_buffer()) {
    CMDCALL(set_mark);
    insert_nstring(killed_text);
    weigh_mark();
  }
}
END_DEF

void init_kill_ring(void)
{
  killed_text = astr_new("");
}
