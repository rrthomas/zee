/* Miscellaneous Emacs functions reimplementation
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2006 Reuben Thomas.
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
#include <unistd.h>

#include "main.h"
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

DEF_ARG(set_wrap_column,
"\
Set the wrap column.\n\
If an argument value is passed, set `wrap_column' to that value,\n\
otherwise with the current column value.\
",
UINT(col, "New wrap column: "))
{
  set_variable(rblist_from_string("wrap_column"),
               astr_afmt("%lu", list_empty(l) ? buf->pt.o + 1 : col));
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
  minibuf_write(rblist_from_string(""));
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

static int quoted_insert_octal(int c1)
{
  int c2, c3;
  minibuf_write(astr_afmt("Insert octal character %d-", c1 - '0'));
  c2 = getkey();

  if (!isdigit(c2) || c2 - '0' >= 8) {
    insert_char(c1 - '0');
    insert_char(c2);
    return true;
  }

  minibuf_write(astr_afmt("Insert octal character %d %d-", c1 - '0', c2 - '0'));
  c3 = getkey();

  if (!isdigit(c3) || c3 - '0' >= 8) {
    insert_char((c1 - '0') * 8 + (c2 - '0'));
    insert_char(c3);
    return true;
  }

  insert_char((c1 - '8') * 64 + (c2 - '0') * 8 + (c3 - '0'));

  return true;
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
    buf->pt.o = rblist_length(buf->pt.p->item);
    if (!CMDCALL(move_next_line)) {
      ok = false;
      break;
    }
    buf->pt.o = 0;
  }
}
END_DEF

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
      buf->pt.o = rblist_length(buf->pt.p->item);
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

DEF(edit_select_word,
"\
Select the current word.\
")
{
  if (!eolp() && isalnum(following_char()))
    ok = CMDCALL(move_next_word) &&
      CMDCALL(edit_select_on) &&
      CMDCALL(move_previous_word) &&
      CMDCALL(edit_select_other_end);
}
END_DEF

DEF(edit_select_word_backward,
"\
Select the previous word.\
")
{
  if (!bolp() && isalnum(preceding_char()))
    ok = CMDCALL(move_previous_word) &&
      CMDCALL(edit_select_on) &&
      CMDCALL(move_next_word) &&
      CMDCALL(edit_select_other_end);
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

DEF(edit_select_paragraph,
"\
Select the current paragraph.\
")
{
  CMDCALL(move_next_paragraph);
  CMDCALL(edit_select_on);
  CMDCALL(move_previous_paragraph);
  CMDCALL(edit_select_other_end);
}
END_DEF

/* FIXME: wrap_paragraph undo goes bananas. */
DEF(edit_wrap_paragraph,
"\
Wrap the paragraph at or after the cursor. The wrap column can\n\
be set using set_wrap_column.\
")
{
  int i, start, end;
  Marker *m = point_marker();

  undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);

  CMDCALL(move_next_paragraph);
  end = buf->pt.n;
  if (is_empty_line())
    end--;

  CMDCALL(move_previous_paragraph);
  start = buf->pt.n;
  if (is_empty_line()) {  /* Move to next line if between two paragraphs. */
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
  while (get_goalc() > (size_t)get_variable_number(rblist_from_string("wrap_column")) + 1)
    wrap_break_line();

  thisflag &= ~FLAG_DONE_CPCN;

  buf->pt = m->pt;
  remove_marker(m);

  undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);
}
END_DEF

#define UPPERCASE		1
#define LOWERCASE		2
#define CAPITALIZE		3

static int setcase_word(int rcase)
{
  size_t i, size;
  int firstchar;

  if (!isalnum(following_char())) {
    if (!CMDCALL(move_next_word))
      return false;
    if (!CMDCALL(move_previous_word))
      return false;
  }

  i = buf->pt.o;
  while (i < rblist_length(buf->pt.p->item)) {
    if (!isalnum(rblist_get(buf->pt.p->item, i)))
      break;
    ++i;
  }
  if ((size = i - buf->pt.o) > 0)
    undo_save(UNDO_REPLACE_BLOCK, buf->pt, size, size);

  for (firstchar = true;
       buf->pt.o < i;
       buf->pt.o++, firstchar = false) {
    char c = rblist_get(buf->pt.p->item, buf->pt.o);

    if (isalpha(c)) {
      if (rcase == UPPERCASE)
        c = toupper(c);
      else if (rcase == LOWERCASE)
        c = tolower(c);
      else if (rcase == CAPITALIZE)
        c = firstchar ? toupper(c) : tolower(c);

      rblist_concat(rblist_sub(buf->pt.p->item, 0, buf->pt.o - 1),
                    rblist_concat(rblist_singleton(c),
                                  rblist_sub(buf->pt.p->item, buf->pt.o + 1,
                                             rblist_length(buf->pt.p->item))));
/*       rblist_get(buf->pt.p->item, buf->pt.o) = c; */
    } else if (!isdigit(c))
      break;
  }

  buf->flags |= BFLAG_MODIFIED;

  return true;
}

DEF(edit_case_lower,
"\
Convert the following word to lower case, moving over it.\
")
{
  ok = setcase_word(LOWERCASE);
}
END_DEF

DEF(edit_case_upper,
"\
Convert the following word to upper case, moving over it.\
")
{
  ok = setcase_word(UPPERCASE);
}
END_DEF

DEF(edit_case_capitalize,
"\
Capitalize the following word, moving over it.\
")
{
  ok = setcase_word(CAPITALIZE);
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
file, replacing the selection.\n\
")
{
  astr ms;

  if ((ms = minibuf_read(rblist_from_string("Shell command: "), rblist_from_string(""))) == NULL)
    ok = CMDCALL(edit_select_off);
  else if (rblist_length(ms) == 0 || warn_if_no_mark())
    ok = false;
  else {
    char tempfile[] = P_tmpdir "/" PACKAGE_NAME "XXXXXX";
    int fd = mkstemp(tempfile);

    if (fd == -1) {
      minibuf_error(rblist_from_string("Cannot open temporary file"));
      ok = false;
    } else {
      FILE *pipe;
      Region r;
      astr cmd = rblist_from_string(""), as;

      assert(calculate_the_region(&r));
      as = copy_text_block(r.start, r.size);
      write(fd, rblist_to_string(as), r.size);

      close(fd);

      cmd = astr_afmt("%s 2>&1 <%s", rblist_to_string(ms), tempfile);

      if ((pipe = popen(rblist_to_string(cmd), "r")) == NULL) {
        minibuf_error(rblist_from_string("Cannot open pipe to process"));
        ok = false;
      } else {
        astr out = rblist_from_string(""), s;

        while (rblist_length(s = astr_fgets(pipe)) > 0) {
          out = rblist_concat(out, s);
          out = rblist_concat(out, rblist_from_string("\n"));
        }
        pclose(pipe);
        remove(tempfile);

#ifdef CURSES
        /* We have no way of knowing whether a sub-process caught a
           SIGWINCH, so raise one. */
        raise(SIGWINCH);
#endif

        undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);
        calculate_the_region(&r);
        if (buf->pt.p != r.start.p
            || r.start.o != buf->pt.o)
          CMDCALL(edit_select_other_end);
        delete_nstring(r.size, &s);
        ok = insert_nstring(out);
        undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);
      }
    }
  }
}
END_DEF
