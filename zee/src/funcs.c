/* Miscellaneous Emacs functions reimplementation
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2005 Reuben Thomas.
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

#include <assert.h>
#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "main.h"
#include "extern.h"
#include "vars.h"


DEFUN_INT("suspend", suspend)
/*+
Stop and return to superior process.
+*/
{
  raise(SIGTSTP);
}
END_DEFUN

int cancel(void)
{
  weigh_mark();
  minibuf_error("Quit");
  return FALSE;
}

DEFUN_INT("keyboard-quit", keyboard_quit)
/*+
Cancel current command.
+*/
{
  ok = cancel();
}
END_DEFUN

DEFUN_INT("edit-toggle-read-only", edit_toggle_read_only)
/*+
Change whether this buffer is visiting its file read-only.
+*/
{
  assert(cur_bp);
  cur_bp->flags ^= BFLAG_READONLY;
}
END_DEFUN

DEFUN_INT("auto-fill-mode", auto_fill_mode)
/*+
Toggle Auto Fill mode.
In Auto Fill mode, inserting a space at a column beyond `fill-column'
automatically breaks the line at a previous space.
+*/
{
  assert(cur_bp);
  cur_bp->flags ^= BFLAG_AUTOFILL;
}
END_DEFUN

DEFUN_INT("set-fill-column", set_fill_column)
/*+
Set the fill column.
If an argument value is passed, set the `fill-column' variable with
that value, otherwise with the current column value.
+*/
{
  assert(cur_bp);
  if (lastflag & FLAG_SET_UNIARG)
    variableSetNumber(&cur_bp->vars, "fill-column", uniarg);
  else
    variableSetNumber(&cur_bp->vars, "fill-column", (int)(cur_bp->pt.o + 1));
}
END_DEFUN

void set_mark_command(void)
{
  set_mark();
  minibuf_write("Mark set");
}

DEFUN_INT("set-mark-command", set_mark_command)
/*+
Set mark at where point is.
+*/
{
  set_mark_command();
  anchor_mark();
}
END_DEFUN

void exchange_point_and_mark(void)
{
  assert(cur_bp);
  assert(cur_bp->mark);

  /* Swap the point with the mark. */
  swap_point(&cur_bp->pt, &cur_bp->mark->pt);
}

DEFUN_INT("exchange-point-and-mark", exchange_point_and_mark)
/*+
Put the mark where point is now, and point where the mark is now.
+*/
{
  exchange_point_and_mark();
  anchor_mark();
  thisflag |= FLAG_NEED_RESYNC;
}
END_DEFUN

DEFUN_INT("mark-whole-buffer", mark_whole_buffer)
/*+
Put point at beginning and mark at end of buffer.
+*/
{
  gotoeob();
  FUNCALL(set_mark_command);
  gotobob();
}
END_DEFUN

static int quoted_insert_octal(int c1)
{
  int c2, c3;
  minibuf_write("C-q %d-", c1 - '0');
  c2 = getkey();

  if (!isdigit(c2) || c2 - '0' >= 8) {
    insert_char(c1 - '0');
    insert_char(c2);
    return TRUE;
  }

  minibuf_write("C-q %d %d-", c1 - '0', c2 - '0');
  c3 = getkey();

  if (!isdigit(c3) || c3 - '0' >= 8) {
    insert_char((c1 - '0') * 8 + (c2 - '0'));
    insert_char(c3);
    return TRUE;
  }

  insert_char((c1 - '8') * 64 + (c2 - '0') * 8 + (c3 - '0'));

  return TRUE;
}

DEFUN_INT("quoted-insert", quoted_insert)
/*+
Read next input character and insert it.
This is useful for inserting control characters.
You may also type up to 3 octal digits, to insert a character with that code.
+*/
{
  int c;

  minibuf_write("C-q-");
  c = xgetkey(GETKEY_UNFILTERED, 0);

  if (isdigit(c) && c - '0' < 8)
    quoted_insert_octal(c);
  else
    insert_char(c);

  minibuf_clear();
}
END_DEFUN

int universal_argument(int keytype, int xarg)
{
  int i = 0, arg = 4, sgn = 1, digit;
  size_t key;
  astr as = astr_new();

  if (keytype == KBD_META) {
    astr_cpy_cstr(as, "ESC");
    ungetkey((size_t)(xarg + '0'));
  } else
    astr_cpy_cstr(as, "C-u");

  for (;;) {
    astr_cat_cstr(as, "-"); /* Add the '-' character. */
    minibuf_write("%s", astr_cstr(as));
    key = getkey();
    minibuf_clear();
    astr_truncate(as, -1); /* Remove the '-' character. */

    /* Cancelled. */
    if (key == KBD_CANCEL)
      return cancel();
    /* Digit pressed. */
    else if (isdigit(key & 0xff)) {
      digit = (key & 0xff) - '0';

      if (key & KBD_META)
        astr_cat_cstr(as, " ESC");

      astr_afmt(as, " %d", digit);

      if (i == 0)
        arg = digit;
      else
        arg = arg * 10 + digit;

      i++;
    } else if (key == (KBD_CTRL | 'u')) {
      astr_cat_cstr(as, " C-u");
      if (i == 0)
        arg *= 4;
    } else if (key == '-') {
      /* After any number && if sign doesn't change. */
      if (i == 0 && sgn > 0) {
        sgn = -sgn;
        astr_cat_cstr(as, " -");
        /* The default negative arg isn't -4, it's -1. */
        arg = 1;
      } else if (i != 0) {
        /* If i == 0 do nothing. */
        ungetkey(key);
        break;
      }
    } else {
      ungetkey(key);
      break;
    }
  }

  last_uniarg = arg * sgn;
  thisflag |= FLAG_SET_UNIARG;
  minibuf_clear();
  astr_delete(as);

  return TRUE;
}

DEFUN_INT("universal-argument", universal_argument)
/*+
Begin a numeric argument for the following command.
Digits or minus sign following C-u make up the numeric argument.
C-u following the digits or minus sign ends the argument.
C-u without digits or minus sign provides 4 as argument.
Repeating C-u without digits or minus sign multiplies the argument
by 4 each time.
+*/
{
  ok = universal_argument(KBD_CTRL | 'u', 0);
}
END_DEFUN

DEFUN_INT("back-to-indentation", back_to_indentation)
/*+
Move point to the first non-whitespace character on this line.
+*/
{
  assert(cur_bp);
  cur_bp->pt = line_beginning_position(0);
  while (!eolp()) {
    if (!isspace(following_char()))
      break;
    edit_navigate_forward_char();
  }
}
END_DEFUN


/***********************************************************************
			  Move through words
***********************************************************************/

#define ISWORDCHAR(c)	(isalnum(c) || c == '$')

static int forward_word(void)
{
  int gotword = FALSE;

  assert(cur_bp);

  for (;;) {
    while (!eolp()) {
      int c = following_char();
      if (!ISWORDCHAR(c)) {
        if (gotword)
          return TRUE;
      } else
        gotword = TRUE;
      cur_bp->pt.o++;
    }
    if (gotword)
      return TRUE;
    cur_bp->pt.o = astr_len(cur_bp->pt.p->item);
    if (!edit_navigate_down_line())
      break;
    cur_bp->pt.o = 0;
  }
  return FALSE;
}

DEFUN_INT("forward-word", forward_word)
/*+
Move point forward one word (backward if the argument is negative).
With argument, do this that many times.
+*/
{
  int uni;

  if (uniarg < 0)
    ok = FUNCALL_ARG(backward_word, -uniarg);
  else
    for (uni = 0; uni < uniarg; ++uni)
      if (!forward_word()) {
        ok = FALSE;
        break;
      }
}
END_DEFUN

static int backward_word(void)
{
  int gotword = FALSE;

  assert(cur_bp);

  for (;;) {
    if (bolp()) {
      if (!edit_navigate_up_line())
        break;
      cur_bp->pt.o = astr_len(cur_bp->pt.p->item);
    }
    while (!bolp()) {
      int c = preceding_char();
      if (!ISWORDCHAR(c)) {
        if (gotword)
          return TRUE;
      } else
        gotword = TRUE;
      cur_bp->pt.o--;
    }
    if (gotword)
      return TRUE;
  }
  return FALSE;
}

DEFUN_INT("backward-word", backward_word)
/*+
Move backward until encountering the beginning of a word.
With argument, do this that many times.
If the argument is negative, move forward.
+*/
{
  int uni;

  if (uniarg < 0)
    ok = FUNCALL_ARG(forward_word, -uniarg);
  else
    for (uni = 0; uni < uniarg; ++uni)
      if (!backward_word()) {
        ok = FALSE;
        break;
      }
}
END_DEFUN


/***********************************************************************
	       Move through balanced expressions (sexp)
***********************************************************************/

#define ISSEXPCHAR(c)	      (isalnum(c) || c == '$' || c == '_')

#define ISOPENBRACKETCHAR(c)  ((c=='(') || (c=='[') || (c=='{')	||	\
			       ((c=='\"') && !double_quote) ||		\
			       ((c=='\'') && !single_quote))

#define ISCLOSEBRACKETCHAR(c) ((c==')') || (c==']') || (c=='}')	||	\
			       ((c=='\"') && double_quote) ||		\
			       ((c=='\'') && single_quote))

#define ISSEXPSEPARATOR(c)    (ISOPENBRACKETCHAR(c) ||	\
			       ISCLOSEBRACKETCHAR(c))

#define CONTROL_SEXP_LEVEL(open, close)					\
	if (open(c)) {							\
		if (level == 0 && gotsexp)				\
			return TRUE;					\
									\
		level++;						\
		gotsexp = TRUE;						\
		if (c == '\"') double_quote ^= 1;			\
		if (c == '\'') single_quote ^= 1;			\
	}								\
	else if (close(c)) {						\
		if (level == 0 && gotsexp)				\
			return TRUE;					\
									\
		level--;						\
		gotsexp = TRUE;						\
		if (c == '\"') double_quote ^= 1;			\
		if (c == '\'') single_quote ^= 1;			\
									\
		if (level < 0) {					\
			minibuf_error("Scan error: \"Containing "	\
				      "expression ends prematurely\"");	\
			return FALSE;					\
		}							\
	}

int forward_sexp(void)
{
  int gotsexp = FALSE;
  int level = 0;
  int double_quote = 0;
  int single_quote = 0;

  assert(cur_bp);
  for (;;) {
    while (!eolp()) {
      int c = following_char();

      /* Jump quotes that aren't sexp separators. */
      if (c == '\\'
          && cur_bp->pt.o+1 < astr_len(cur_bp->pt.p->item)
          && ((*astr_char(cur_bp->pt.p->item, (ptrdiff_t)(cur_bp->pt.o + 1))
               == '\"') ||
              (*astr_char(cur_bp->pt.p->item, (ptrdiff_t)(cur_bp->pt.o + 1))
                  == '\''))) {
        cur_bp->pt.o++;
        c = 'a'; /* Convert \' and \" into a word char. */
      }

      CONTROL_SEXP_LEVEL(ISOPENBRACKETCHAR,
                         ISCLOSEBRACKETCHAR);

      cur_bp->pt.o++;

      if (!ISSEXPCHAR(c)) {
        if (gotsexp && level == 0) {
          if (!ISSEXPSEPARATOR(c))
            cur_bp->pt.o--;
          return TRUE;
        }
      }
      else
        gotsexp = TRUE;
    }
    if (gotsexp && level == 0)
      return TRUE;
    cur_bp->pt.o = astr_len(cur_bp->pt.p->item);
    if (!edit_navigate_down_line()) {
      if (level != 0)
        minibuf_error("Scan error: \"Unbalanced parentheses\"");
      break;
    }
    cur_bp->pt.o = 0;
  }
  return FALSE;
}

DEFUN_INT("forward-sexp", forward_sexp)
/*+
Move forward across one balanced expression (sexp).
With argument, do it that many times.  Negative arg -N means
move backward across N balanced expressions.
+*/
{
  int uni;

  if (uniarg < 0)
    ok = FUNCALL_ARG(backward_sexp, -uniarg);
  else
    for (uni = 0; uni < uniarg; ++uni)
      if (!forward_sexp()) {
        ok = FALSE;
        break;
      }
}
END_DEFUN

int backward_sexp(void)
{
  int gotsexp = FALSE;
  int level = 0;
  int double_quote = 1;
  int single_quote = 1;

  assert(cur_bp);
  for (;;) {
    if (bolp()) {
      if (!edit_navigate_up_line()) {
        if (level != 0)
          minibuf_error("Scan error: \"Unbalanced parentheses\"");
        break;
      }
      cur_bp->pt.o = astr_len(cur_bp->pt.p->item);
    }
    while (!bolp()) {
      int c = preceding_char();

      /* Jump quotes that doesn't are sexp separators.  */
      if (((c == '\'') || (c == '\"'))
          && cur_bp->pt.o-1 > 0
          && (*astr_char(cur_bp->pt.p->item, (ptrdiff_t)(cur_bp->pt.o - 2))
              == '\\')) {
        cur_bp->pt.o--;
        c = 'a'; /* Convert \' and \" like a
                    word char */
      }

      CONTROL_SEXP_LEVEL(ISCLOSEBRACKETCHAR,
                         ISOPENBRACKETCHAR);

      cur_bp->pt.o--;

      if (!ISSEXPCHAR(c)) {
        if (gotsexp && level == 0) {
          if (!ISSEXPSEPARATOR(c))
            cur_bp->pt.o++;
          return TRUE;
        }
      } else
        gotsexp = TRUE;
    }
    if (gotsexp && level == 0)
      return TRUE;
  }
  return FALSE;
}

DEFUN_INT("backward-sexp", backward_sexp)
/*+
Move backward across one balanced expression (sexp).
With argument, do it that many times.  Negative arg -N means
move forward across N balanced expressions.
+*/
{
  int uni;

  if (uniarg < 0)
    ok = FUNCALL_ARG(forward_sexp, -uniarg);
  else
    for (uni = 0; uni < uniarg; ++uni)
      if (!backward_sexp()) {
        ok = FALSE;
        break;
      }
}
END_DEFUN

DEFUN_INT("mark-word", mark_word)
/*+
Set mark ARG words away from point.
+*/
{
  FUNCALL(set_mark_command);
  if ((ok = FUNCALL_ARG(forward_word, uniarg)))
    FUNCALL(exchange_point_and_mark);
}
END_DEFUN

DEFUN_INT("mark-sexp", mark_sexp)
/*+
Set mark ARG sexps from point.
The place mark goes is the same place C-M-f would
move to with the same argument.
+*/
{
  FUNCALL(set_mark_command);
  if ((ok = FUNCALL_ARG(forward_sexp, uniarg)))
    FUNCALL(exchange_point_and_mark);
}
END_DEFUN

DEFUN_INT("forward-line", forward_line)
/*+
Move N lines forward (backward if N is negative).
Precisely, if point is on line I, move to the start of line I + N.
+*/
{
  FUNCALL(beginning_of_line);

  if (uniarg < 0) {
    while (uniarg++)
      if (!edit_navigate_up_line()) {
        ok = FALSE;
        break;
      }
  } else {
    if (uniarg == 0)
      uniarg = 1;

    while (uniarg--)
      if (!edit_navigate_down_line()) {
        ok = FALSE;
        break;
      }
  }
}
END_DEFUN

DEFUN_INT("backward-paragraph", backward_paragraph)
/*+
Move backward to start of paragraph.  With argument N, do it N times.
+*/
{
  if (uniarg < 0)
    ok = FUNCALL_ARG(forward_paragraph, -uniarg);
  else {
    do {
      while (edit_navigate_up_line() && is_empty_line());
      while (edit_navigate_up_line() && !is_empty_line());
    } while (--uniarg > 0);

    FUNCALL(beginning_of_line);
  }
}
END_DEFUN

DEFUN_INT("forward-paragraph", forward_paragraph)
/*+
Move forward to end of paragraph.  With argument N, do it N times.
+*/
{
  if (uniarg < 0)
    ok = FUNCALL_ARG(backward_paragraph, -uniarg);
  else {
    do {
      while (edit_navigate_down_line() && is_empty_line());
      while (edit_navigate_down_line() && !is_empty_line());
    } while (--uniarg > 0);

    if (is_empty_line())
      FUNCALL(beginning_of_line);
    else
      FUNCALL(end_of_line);
  }
}
END_DEFUN

DEFUN_INT("mark-paragraph", mark_paragraph)
/*+
Put point at beginning of this paragraph, mark at end.
The paragraph marked is the one that contains point or follows point.
+*/
{
  if (last_command() == F_mark_paragraph) {
    FUNCALL(exchange_point_and_mark);
    FUNCALL_ARG(forward_paragraph, uniarg);
    FUNCALL(exchange_point_and_mark);
  } else {
    FUNCALL_ARG(forward_paragraph, uniarg);
    FUNCALL(set_mark_command);
    FUNCALL_ARG(backward_paragraph, uniarg);
  }
}
END_DEFUN

DEFUN_INT("fill-paragraph", fill_paragraph)
/*+
Fill paragraph at or after point.
+*/
{
  int i, start, end;
  Marker *m = point_marker();

  assert(cur_bp);
  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0, FALSE);

  FUNCALL(forward_paragraph);
  end = cur_bp->pt.n;
  if (is_empty_line())
    end--;

  FUNCALL(backward_paragraph);
  start = cur_bp->pt.n;
  if (is_empty_line()) {  /* Move to next line if between two paragraphs. */
    edit_navigate_down_line();
    start++;
  }

  for (i = start; i < end; i++) {
    FUNCALL(end_of_line);
    delete_char();
    FUNCALL(delete_horizontal_space);
    insert_char(' ');
  }

  FUNCALL(end_of_line);
  while (get_goalc() > (size_t)get_variable_number("fill-column") + 1)
    fill_break_line();

  thisflag &= ~FLAG_DONE_CPCN;

  cur_bp->pt = m->pt;
  free_marker(m);

  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
}
END_DEFUN

#define UPPERCASE		1
#define LOWERCASE		2
#define CAPITALIZE		3

static int setcase_word(int rcase)
{
  int gotword;
  size_t i, size;

  assert(cur_bp);

  if (!ISWORDCHAR(following_char())) {
    if (!forward_word())
      return FALSE;
    if (!backward_word())
      return FALSE;
  }

  i = cur_bp->pt.o;
  while (i < astr_len(cur_bp->pt.p->item)) {
    if (!ISWORDCHAR(*astr_char(cur_bp->pt.p->item, (ptrdiff_t)i)))
      break;
    ++i;
  }
  size = i-cur_bp->pt.o;
  if (size > 0)
    undo_save(UNDO_REPLACE_BLOCK, cur_bp->pt, size, size, FALSE);

  gotword = FALSE;
  while (cur_bp->pt.o < astr_len(cur_bp->pt.p->item)) {
    char *p = astr_char(cur_bp->pt.p->item, (ptrdiff_t)cur_bp->pt.o);
    if (!ISWORDCHAR(*p))
      break;
    if (isalpha(*p)) {
      int oldc = *p, newc;
      if (rcase == UPPERCASE)
        newc = toupper(oldc);
      else if (rcase == LOWERCASE)
        newc = tolower(oldc);
      else { /* rcase == CAPITALIZE */
        if (!gotword)
          newc = toupper(oldc);
        else
          newc = tolower(oldc);
      }
      if (oldc != newc) {
        *p = newc;
      }
    }
    gotword = TRUE;
    cur_bp->pt.o++;
  }

  cur_bp->flags |= BFLAG_MODIFIED;

  return TRUE;
}

DEFUN_INT("downcase-word", downcase_word)
/*+
Convert following word (or argument N words) to lower case, moving over.
+*/
{
  int uni;

  assert(cur_bp);

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
  for (uni = 0; uni < uniarg; ++uni)
    if (!setcase_word(LOWERCASE)) {
      ok = FALSE;
      break;
    }
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
}
END_DEFUN

DEFUN_INT("upcase-word", upcase_word)
/*+
Convert following word (or argument N words) to upper case, moving over.
+*/
{
  int uni;

  assert(cur_bp);

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
  for (uni = 0; uni < uniarg; ++uni)
    if (!setcase_word(UPPERCASE)) {
      ok = FALSE;
      break;
    }
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
}
END_DEFUN

DEFUN_INT("capitalize-word", capitalize_word)
/*+
Capitalize the following word (or argument N words), moving over.
This gives the word(s) a first character in upper case and the rest
lower case.
+*/
{
  int uni;

  assert(cur_bp);

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
  for (uni = 0; uni < uniarg; ++uni)
    if (!setcase_word(CAPITALIZE)) {
      ok = FALSE;
      break;
    }
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
}
END_DEFUN

DEFUN_INT("shell-command", shell_command)
/*+
Reads a line of text using the minibuffer and creates an inferior shell
to execute the line as a command; passes the contents of the region as
input to the shell command.
If the shell command produces any output, it is inserted into the
current buffer, overwriting the current region.
+*/
{
  astr ms;

  assert(cur_bp);

  if ((ms = minibuf_read("Shell command: ", "")) == NULL)
    ok = cancel();
  else if (astr_len(ms) == 0 || warn_if_no_mark())
    ok = FALSE;
  else {
    char tempfile[] = P_tmpdir "/" PACKAGE_NAME "XXXXXX";
    int fd = mkstemp(tempfile);

    if (fd == -1) {
      minibuf_error("Cannot open temporary file");
      ok = FALSE;
    } else {
      FILE *pipe;
      Region r;
      astr cmd = astr_new(), as;

      calculate_the_region(&r);
      as = copy_text_block(r.start, r.size);
      write(fd, astr_cstr(as), r.size);
      astr_delete(as);

      close(fd);

      astr_afmt(cmd, "%s 2>&1 <%s", astr_cstr(ms), tempfile);

      if ((pipe = popen(astr_cstr(cmd), "r")) == NULL) {
        minibuf_error("Cannot open pipe to process");
        ok = FALSE;
      } else {
        astr out = astr_new(), s;

        while (astr_len(s = astr_fgets(pipe)) > 0) {
          astr_cat_delete(out, s);
          astr_cat_cstr(out, "\n");
        }
        astr_delete(s);
        pclose(pipe);
        remove(tempfile);

#ifdef CURSES
        /* We have no way of knowing whether a sub-process caught a
           SIGWINCH, so raise one. */
        raise(SIGWINCH);
#endif

        undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
        calculate_the_region(&r);
        if (cur_bp->pt.p != r.start.p
            || r.start.o != cur_bp->pt.o)
          FUNCALL(exchange_point_and_mark);
        FUNCALL_ARG(delete_char, (int)r.size);
        ok = insert_nstring(out, "\n", FALSE);
        undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0, FALSE);

        astr_delete(out);
      }

      astr_delete(cmd);
    }
  }
}
END_DEFUN
