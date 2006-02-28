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

#include <ctype.h>
#include <signal.h>
#include <unistd.h>

#include "main.h"
#include "extern.h"


DEFUN(suspend,
"\
Stop and return to superior process.\
")
{
  raise(SIGTSTP);
}
END_DEFUN

DEFUN(cancel,
"\
Cancel current command.\
")
{
  weigh_mark();
  minibuf_error(astr_new("Quit"));
  ok = FALSE;
}
END_DEFUN

DEFUN(edit_toggle_read_only,
"\
Change whether this buffer is visiting its file read-only.\
")
{
  buf.flags ^= BFLAG_READONLY;
}
END_DEFUN

DEFUN(auto_fill_mode,
"\
Toggle Auto Fill mode.\n\
In Auto Fill mode, inserting a space at a column beyond `fill_column'\n\
automatically breaks the line at a previous space.\
")
{
  buf.flags ^= BFLAG_AUTOFILL;
}
END_DEFUN

DEFUN_INT(set_fill_column,
"\
Set the fill column.\n\
If an argument value is passed, set `fill_column' to that value,\n\
otherwise with the current column value.\
")
{
  set_variable(astr_new("fill_column"),
               astr_afmt("%d", (argc > 0) ? intarg : (int)(buf.pt.o + 1)));
}
END_DEFUN

DEFUN(set_mark,
"\
Set mark where point is.\
")
{
  set_mark_to_point();
  minibuf_write(astr_new("Mark set"));
  anchor_mark();
}
END_DEFUN

DEFUN(exchange_point_and_mark,
"\
Put the mark where point is now, and point where the mark is now.\
")
{
  assert(buf.mark);
  swap_point(&buf.pt, &buf.mark->pt);
  anchor_mark();
  thisflag |= FLAG_NEED_RESYNC;
}
END_DEFUN

DEFUN(mark_whole_buffer,
"\
Put point at beginning and mark at end of buffer.\
")
{
  FUNCALL(end_of_buffer);
  FUNCALL(set_mark);
  FUNCALL(beginning_of_buffer);
}
END_DEFUN

static int quoted_insert_octal(int c1)
{
  int c2, c3;
  minibuf_write(astr_afmt("Insert octal character %d-", c1 - '0'));
  c2 = getkey();

  if (!isdigit(c2) || c2 - '0' >= 8) {
    insert_char(c1 - '0');
    insert_char(c2);
    return TRUE;
  }

  minibuf_write(astr_afmt("Insert octal character %d %d-", c1 - '0', c2 - '0'));
  c3 = getkey();

  if (!isdigit(c3) || c3 - '0' >= 8) {
    insert_char((c1 - '0') * 8 + (c2 - '0'));
    insert_char(c3);
    return TRUE;
  }

  insert_char((c1 - '8') * 64 + (c2 - '0') * 8 + (c3 - '0'));

  return TRUE;
}

DEFUN(quoted_insert,
"\
Read next input character and insert it.\n\
This is useful for inserting control characters.\n\
You may also type up to 3 octal digits, to insert a character with that code.\
")
{
  int c;

  minibuf_write(astr_new("Insert literal character: "));
  c = xgetkey(GETKEY_UNFILTERED, 0);

  if (isdigit(c) && c - '0' < 8)
    quoted_insert_octal(c);
  else
    insert_char(c);

  minibuf_clear();
}
END_DEFUN

DEFUN(universal_argument,
"\
Begin a numeric argument for the following command.\n\
Digits or minus sign following C-u make up the numeric argument.\n\
C-u following the digits or minus sign ends the argument.\
")
{
  size_t key, i = 0, arg = 0, digit;
  int sgn = 1;
  astr as = astr_new("");

  ok = TRUE;

  for (;;) {
    minibuf_write(astr_afmt("%s-", astr_cstr(as)));
    key = getkey();
    minibuf_clear();

    if (key == KBD_CANCEL) {
      ok = FUNCALL(cancel);
      break;
    } else if (isdigit(key & 0xff)) {
      /* Digit pressed. */
      digit = (key & 0xff) - '0';
      astr_cat(as, astr_afmt(" %d", digit));
      arg = arg * 10 + digit;
      i++;
    } else if (key == (KBD_CTRL | 'u'))
      break;
    else if (key == '-' && i == 0) {
      if (sgn > 0) {
        sgn = -sgn;
        astr_cat(as, astr_new(" -"));
      }
    } else {
      ungetkey(key);
      break;
    }
  }

  uniarg = arg * sgn;
  thisflag |= FLAG_SET_UNIARG;
  minibuf_clear();
}
END_DEFUN

DEFUN(back_to_indentation,
"\
Move point to the first non-whitespace character on this line.\
")
{
  buf.pt.o = 0;
  while (!eolp()) {
    if (!isspace(following_char()))
      break;
    FUNCALL(edit_navigate_forward_char);
  }
}
END_DEFUN


/***********************************************************************
			  Move through words
***********************************************************************/

DEFUN(forward_word,
"\
Move point forward one word.\
")
{
  int gotword = FALSE;

  for (;;) {
    while (!eolp()) {
      int c = following_char();
      if (!isalnum(c)) {
        if (gotword)
          break;
      } else
        gotword = TRUE;
      buf.pt.o++;
    }
    if (gotword)
      break;
    buf.pt.o = astr_len(buf.pt.p->item);
    if (!FUNCALL(edit_navigate_down_line)) {
      ok = FALSE;
      break;
    }
    buf.pt.o = 0;
  }
}
END_DEFUN

DEFUN(backward_word,
"\
Move backward until encountering the beginning of a word.\
")
{
  int gotword = FALSE;

  for (;;) {
    if (bolp()) {
      if (!FUNCALL(edit_navigate_up_line)) {
        ok = FALSE;
        break;
      }
      buf.pt.o = astr_len(buf.pt.p->item);
    }
    while (!bolp()) {
      int c = preceding_char();
      if (!isalnum(c)) {
        if (gotword)
          break;
      } else
        gotword = TRUE;
      buf.pt.o--;
    }
    if (gotword)
      break;
  }
}
END_DEFUN

DEFUN(mark_word,
"\
Set mark to end of current word.\
")
{
  FUNCALL(set_mark);
  if ((ok = FUNCALL(forward_word)))
    FUNCALL(exchange_point_and_mark);
}
END_DEFUN

DEFUN(mark_word_backward,
"\
Set mark to start of current word.\
")
{
  FUNCALL(set_mark);
  if ((ok = FUNCALL(backward_word)))
    FUNCALL(exchange_point_and_mark);
}
END_DEFUN

DEFUN(backward_paragraph,
"\
Move backward to start of paragraph.\
")
{
  while (is_empty_line() && FUNCALL(edit_navigate_up_line))
    ;
  while (!is_empty_line() && FUNCALL(edit_navigate_up_line))
    ;

  FUNCALL(beginning_of_line);
}
END_DEFUN

DEFUN(forward_paragraph,
"\
Move forward to end of paragraph.\
")
{
  while (is_empty_line() && FUNCALL(edit_navigate_down_line))
    ;
  while (!is_empty_line() && FUNCALL(edit_navigate_down_line))
    ;

  if (is_empty_line())
    FUNCALL(beginning_of_line);
  else
    FUNCALL(end_of_line);
}
END_DEFUN

DEFUN(mark_paragraph,
"\
Put point at beginning of this paragraph, mark at end.\n\
The paragraph marked is the one that contains point or follows point.\
")
{
  FUNCALL(forward_paragraph);
  FUNCALL(set_mark);
  FUNCALL(backward_paragraph);
}
END_DEFUN

/* FIXME: fill_paragraph undo goes bananas. */
DEFUN(fill_paragraph,
"\
Fill paragraph at or after point.\
")
{
  int i, start, end;
  Marker *m = point_marker();

  undo_save(UNDO_START_SEQUENCE, buf.pt, 0, 0);

  FUNCALL(forward_paragraph);
  end = buf.pt.n;
  if (is_empty_line())
    end--;

  FUNCALL(backward_paragraph);
  start = buf.pt.n;
  if (is_empty_line()) {  /* Move to next line if between two paragraphs. */
    FUNCALL(edit_navigate_down_line);
    start++;
  }

  for (i = start; i < end; i++) {
    FUNCALL(end_of_line);
    FUNCALL(delete_char);
    FUNCALL(delete_horizontal_space);
    insert_char(' ');
  }

  FUNCALL(end_of_line);
  while (get_goalc() > (size_t)get_variable_number(astr_new("fill_column")) + 1)
    fill_break_line();

  thisflag &= ~FLAG_DONE_CPCN;

  buf.pt = m->pt;
  remove_marker(m);

  undo_save(UNDO_END_SEQUENCE, buf.pt, 0, 0);
}
END_DEFUN

#define UPPERCASE		1
#define LOWERCASE		2
#define CAPITALIZE		3

static int setcase_word(int rcase)
{
  size_t i, size;
  int firstchar;

  if (!isalnum(following_char())) {
    if (!FUNCALL(forward_word))
      return FALSE;
    if (!FUNCALL(backward_word))
      return FALSE;
  }

  i = buf.pt.o;
  while (i < astr_len(buf.pt.p->item)) {
    if (!isalnum(*astr_char(buf.pt.p->item, (ptrdiff_t)i)))
      break;
    ++i;
  }
  if ((size = i - buf.pt.o) > 0)
    undo_save(UNDO_REPLACE_BLOCK, buf.pt, size, size);

  for (firstchar = TRUE;
       buf.pt.o < i;
       buf.pt.o++, firstchar = FALSE) {
    char c = *astr_char(buf.pt.p->item, (ptrdiff_t)buf.pt.o);

    if (isalpha(c)) {
      if (rcase == UPPERCASE)
        c = toupper(c);
      else if (rcase == LOWERCASE)
        c = tolower(c);
      else if (rcase == CAPITALIZE)
        c = firstchar ? toupper(c) : tolower(c);

      *astr_char(buf.pt.p->item, (ptrdiff_t)buf.pt.o) = c;
    } else if (!isdigit(c))
      break;
  }

  buf.flags |= BFLAG_MODIFIED;

  return TRUE;
}

DEFUN(downcase_word,
"\
Convert following word to lower case, moving over.\
")
{
  ok = setcase_word(LOWERCASE);
}
END_DEFUN

DEFUN(upcase_word,
"\
Convert following word to upper case, moving over.\
")
{
  ok = setcase_word(UPPERCASE);
}
END_DEFUN

DEFUN(capitalize_word,
"\
Capitalize the following word, moving over.\
")
{
  ok = setcase_word(CAPITALIZE);
}
END_DEFUN

DEFUN(execute_command,
"\
Read command or macro name, then call it.\n\
FIXME: Make it work non-interactively.\
")
{
  astr name;
  Function func;
  Macro *mp;

  if ((name = minibuf_read_function_name(astr_new("Execute command: ")))) {
    if ((func = get_function(name)))
      ok = func(NULL);
    else if ((mp = get_macro(name)))
      call_macro(mp);
    else
      ok = FALSE;
  } else
    ok = FALSE;
}
END_DEFUN

DEFUN(shell_command,
"\
Reads a line of text using the minibuffer and creates an inferior shell\n\
to execute the line as a command; passes the contents of the region as\n\
input to the shell command.\n\
If the shell command produces any output, it is inserted into the\n\
current buffer, overwriting the current region.\n\
FIXME: Use better-shell.c\
")
{
  astr ms;

  if ((ms = minibuf_read(astr_new("Shell command: "), astr_new(""))) == NULL)
    ok = FUNCALL(cancel);
  else if (astr_len(ms) == 0 || warn_if_no_mark())
    ok = FALSE;
  else {
    char tempfile[] = P_tmpdir "/" PACKAGE_NAME "XXXXXX";
    int fd = mkstemp(tempfile);

    if (fd == -1) {
      minibuf_error(astr_new("Cannot open temporary file"));
      ok = FALSE;
    } else {
      FILE *pipe;
      Region r;
      astr cmd = astr_new(""), as;

      assert(calculate_the_region(&r));
      as = copy_text_block(r.start, r.size);
      write(fd, astr_cstr(as), r.size);

      close(fd);

      cmd = astr_afmt("%s 2>&1 <%s", astr_cstr(ms), tempfile);

      if ((pipe = popen(astr_cstr(cmd), "r")) == NULL) {
        minibuf_error(astr_new("Cannot open pipe to process"));
        ok = FALSE;
      } else {
        astr out = astr_new(""), s;

        while (astr_len(s = astr_fgets(pipe)) > 0) {
          astr_cat(out, s);
          astr_cat(out, astr_new("\n"));
        }
        pclose(pipe);
        remove(tempfile);

#ifdef CURSES
        /* We have no way of knowing whether a sub-process caught a
           SIGWINCH, so raise one. */
        raise(SIGWINCH);
#endif

        undo_save(UNDO_START_SEQUENCE, buf.pt, 0, 0);
        calculate_the_region(&r);
        if (buf.pt.p != r.start.p
            || r.start.o != buf.pt.o)
          FUNCALL(exchange_point_and_mark);
        delete_nstring(r.size, &s);
        ok = insert_nstring(out);
        undo_save(UNDO_END_SEQUENCE, buf.pt, 0, 0);
      }
    }
  }
}
END_DEFUN
