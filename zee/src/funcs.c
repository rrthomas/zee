/* Miscellaneous user commands
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2007 Reuben Thomas.
   Copyright (c) 2004 David A. Capello.
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

#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "main.h"
#include "rbacc.h"
#include "extern.h"


DEF(file_suspend,
"\
Stop and return to superior process.\
")
{
  raise(SIGTSTP);
}
END_DEF

DEF(preferences_toggle_read_only,
"\
Change whether this file can be modified.\
")
{
  buf->flags ^= BFLAG_READONLY;
}
END_DEF

DEF(preferences_toggle_wrap_mode,
"\
Toggle Wrap mode.\n\
In Wrap mode, inserting a space or newline at a column beyond\n\
`wrap_column' automatically breaks the line at a previous space.\n\
Paragraphs can also be wrapped using the `wrap_paragraph'.\
")
{
  buf->flags ^= BFLAG_AUTOFILL;
}
END_DEF

DEF(edit_select_on,
"\
Start selecting text.\
")
{
  set_mark_to_point();
  minibuf_write(rblist_from_string("Mark set"));
  buf->flags |= BFLAG_ANCHORED;
}
END_DEF

DEF(edit_select_off,
"\
Stop selecting text.\
")
{
  buf->flags &= ~BFLAG_ANCHORED;
  minibuf_write(rblist_empty);
  ok = false;
}
END_DEF

DEF(edit_select_toggle,
"\
Toggle selection mode.\
")
{
  if (buf->flags & BFLAG_ANCHORED)
    CMDCALL(edit_select_off);
  else
    CMDCALL(edit_select_on);
}
END_DEF

DEF(edit_select_other_end,
"\
When selecting text, move the cursor to the other end of the selection.\
")
{
  assert(buf->mark);
  swap_point(&buf->pt, &buf->mark->pt);
  buf->flags |= BFLAG_ANCHORED;
  thisflag |= FLAG_NEED_RESYNC;
}
END_DEF

static void quoted_insert_octal(int c1)
{
  int c2, c3;
  minibuf_write(rblist_fmt("Insert octal character %d-", c1 - '0'));
  c2 = getkey();

  if (!isdigit(c2) || c2 - '0' >= 8) {
    insert_char(c1 - '0');
    insert_char(c2);
  } else {
    minibuf_write(rblist_fmt("Insert octal character %d %d-", c1 - '0', c2 - '0'));
    c3 = getkey();
    
    if (!isdigit(c3) || c3 - '0' >= 8) {
      insert_char((c1 - '0') * 8 + (c2 - '0'));
      insert_char(c3);
    } else
      insert_char((c1 - '8') * 64 + (c2 - '0') * 8 + (c3 - '0'));
  }
}

DEF(edit_insert_quoted,
"\
Read next input character and insert it.\n\
This is useful for inserting control characters.\n\
You may also type up to 3 octal digits, to insert a character with that code.\
")
{
  int c;

  minibuf_write(rblist_from_string("Insert literal character: "));
  c = xgetkey(GETKEY_UNFILTERED, 0);

  if (isdigit(c) && c - '0' < 8)
    quoted_insert_octal(c);
  else
    insert_char(c);

  minibuf_clear();
}
END_DEF

DEF_ARG(edit_repeat,
"\
Repeat a command a given number of times.\
",
UINT(reps, "Repeat count: ")
COMMAND(name, "Command: "))
{
  if (ok) {
    Command cmd = get_command(name);
    if (cmd) {
      undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);
      for (size_t i = 0; i < reps; i++)
        cmd(l);
      undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);
    }
  }
}
END_DEF

DEF(move_start_line_text,
"\
Move the cursor to the first non-whitespace character on this line.\
")
{
  buf->pt.o = 0;
  while (!eolp()) {
    if (!isspace(following_char()))
      break;
    CMDCALL(move_next_character);
  }
}
END_DEF


/***********************************************************************
			  Move through words
***********************************************************************/

DEF(move_previous_word,
"\
Move the cursor backwards one word.\
")
{
  bool gotword = false;

  for (;;) {
    if (bolp()) {
      if (!CMDCALL(move_previous_line)) {
        ok = false;
        break;
      }
      buf->pt.o = rblist_line_length(buf->lines, buf->pt.n);
    }
    while (!bolp()) {
      int c = preceding_char();
      if (!isalnum(c)) {
        if (gotword)
          break;
      } else
        gotword = true;
      buf->pt.o--;
    }
    if (gotword)
      break;
  }
}
END_DEF

DEF(move_next_word,
"\
Move the cursor forward one word.\
")
{
  bool gotword = false;

  for (;;) {
    while (!eolp()) {
      int c = following_char();
      if (!isalnum(c)) {
        if (gotword)
          break;
      } else
        gotword = true;
      buf->pt.o++;
    }
    if (gotword)
      break;
    buf->pt.o = rblist_line_length(buf->lines, buf->pt.n);
    if (!CMDCALL(move_next_line)) {
      ok = false;
      break;
    }
    buf->pt.o = 0;
  }
}
END_DEF

DEF(move_previous_paragraph,
"\
Move the cursor backward to the start of the paragraph.\
")
{
  while (is_empty_line() && CMDCALL(move_previous_line))
    ;
  while (!is_empty_line() && CMDCALL(move_previous_line))
    ;

  CMDCALL(move_start_line);
}
END_DEF

DEF(move_next_paragraph,
"\
Move the cursor forward to the end of the paragraph.\
")
{
  while (is_empty_line() && CMDCALL(move_next_line))
    ;
  while (!is_empty_line() && CMDCALL(move_next_line))
    ;

  if (is_empty_line())
    CMDCALL(move_start_line);
  else
    CMDCALL(move_end_line);
}
END_DEF

DEF(edit_wrap_paragraph,
"\
Wrap the paragraph at or after the cursor. The wrap column can\n\
be set using set_wrap_column.\
")
{
  size_t i, start, end;
  Marker *m = point_marker();

  undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);

  CMDCALL(move_next_paragraph);
  end = buf->pt.n;
  if (is_empty_line() && end > 0)
    end--;

  CMDCALL(move_previous_paragraph);
  start = buf->pt.n;
  if (is_empty_line()) {  // Move to next line if between two paragraphs.
    CMDCALL(move_next_line);
    start++;
  }

  for (i = start; i < end; i++) {
    CMDCALL(move_end_line);
    CMDCALL(edit_delete_next_character);
    CMDCALL(delete_horizontal_space);
    insert_char(' ');
  }

  CMDCALL(move_end_line);
  size_t wrap_col = (size_t)get_variable_number(rblist_from_string("wrap_column"));
  while (get_goalc() > wrap_col + 1 && wrap_break_line())
    ;

  thisflag &= ~FLAG_DONE_CPCN;

  buf->pt = m->pt;
  remove_marker(m);

  undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);
}
END_DEF

DEF(execute_command,
"\
Read command or macro name, then call it.\
")
{
  CMDCALL_UINT(edit_repeat, 1);
}
END_DEF

DEF(edit_shell_command,
"\
Reads a line of text using the minibuffer and creates an inferior shell\n\
to execute the line as a command; passes the selection as input to the\n\
shell command.\n\
If the shell command produces any output, it is inserted into the\n\
file, replacing the selection if any.\n\
")
{
  rblist ms;

  if ((ms = minibuf_read(rblist_from_string("Shell command: "), rblist_empty)) == NULL ||
      rblist_length(ms) == 0)
    ok = false;
  else {
    char tempfile[] = P_tmpdir "/" PACKAGE "XXXXXX";
    int fd = mkstemp(tempfile);

    if (fd == -1) {
      minibuf_error(rblist_from_string("Cannot open temporary file"));
      ok = false;
    } else {
      if (!(buf->flags & BFLAG_ANCHORED))
        CMDCALL(edit_select_on);

      Region r;
      assert(calculate_the_region(&r));
      rblist as = copy_text_block(r.start, r.size);
      write(fd, rblist_to_string(as), r.size);

      close(fd);

      rblist cmd = rblist_fmt("%r 2>&1 <%s", ms, tempfile);

      FILE *pipe;
      if ((pipe = popen(rblist_to_string(cmd), "r")) == NULL) {
        minibuf_error(rblist_from_string("Cannot open pipe to process"));
        ok = false;
      } else {
        rblist out = rblist_empty, s;

        while ((s = rbacc_to_rblist(rbacc_file_line(rbacc_new(), pipe))))
          out = rblist_concat(out, rblist_concat(s, rblist_singleton('\n')));
        pclose(pipe);
        remove(tempfile);

#ifdef CURSES
        /* We have no way of knowing whether a sub-process caught a
           SIGWINCH, so raise one. */
        raise(SIGWINCH);
#endif

        undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);
        if (buf->pt.n != r.start.n || r.start.o != buf->pt.o)
          CMDCALL(edit_select_other_end);
        ok = replace_nstring(r.size, NULL, out);
        undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);
        buf->flags &= ~BFLAG_ANCHORED;
      }
    }
  }
}
END_DEF
