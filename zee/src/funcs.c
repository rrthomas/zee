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


DEFUN("suspend", suspend)
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

DEFUN("keyboard-quit", keyboard_quit)
/*+
Cancel current command.
+*/
{
  ok = cancel();
}
END_DEFUN

DEFUN("edit-toggle-read-only", edit_toggle_read_only)
/*+
Change whether this buffer is visiting its file read-only.
+*/
{
  buf.flags ^= BFLAG_READONLY;
}
END_DEFUN

DEFUN("auto-fill-mode", auto_fill_mode)
/*+
Toggle Auto Fill mode.
In Auto Fill mode, inserting a space at a column beyond `fill-column'
automatically breaks the line at a previous space.
+*/
{
  buf.flags ^= BFLAG_AUTOFILL;
}
END_DEFUN

DEFUN_INT("set-fill-column", set_fill_column)
/*+
Set the fill column.
If an argument value is passed, set the `fill-column' variable with
that value, otherwise with the current column value.
+*/
{
  char *s;

  zasprintf(&s, "%d", (argc > 0) ? uniarg : (int)(buf.pt.o + 1));
  set_variable("fill-column", s);
  free(s);
}
END_DEFUN

void set_mark_command(void)
{
  set_mark();
  minibuf_write("Mark set");
}

DEFUN("set-mark-command", set_mark_command)
/*+
Set mark at where point is.
+*/
{
  set_mark_command();
  anchor_mark();
}
END_DEFUN

DEFUN("exchange-point-and-mark", exchange_point_and_mark)
/*+
Put the mark where point is now, and point where the mark is now.
+*/
{
  assert(buf.mark);
  swap_point(&buf.pt, &buf.mark->pt);
  anchor_mark();
  thisflag |= FLAG_NEED_RESYNC;
}
END_DEFUN

DEFUN("mark-whole-buffer", mark_whole_buffer)
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

DEFUN("quoted-insert", quoted_insert)
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

DEFUN("universal-argument", universal_argument)
/*+
Begin a numeric argument for the following command.
Digits or minus sign following C-u make up the numeric argument.
C-u following the digits or minus sign ends the argument.
+*/
{
  size_t key, i = 0, arg = 0, digit;
  int sgn = 1;
  astr as = astr_new();

  ok = TRUE;

  for (;;) {
    astr_cat_cstr(as, "-"); /* Add the '-' character. */
    minibuf_write("%s", astr_cstr(as));
    key = getkey();
    minibuf_clear();
    astr_truncate(as, -1); /* Remove the '-' character. */

    if (key == KBD_CANCEL) {
      ok = cancel();
      break;
    } else if (isdigit(key & 0xff)) {
      /* Digit pressed. */
      digit = (key & 0xff) - '0';
      astr_afmt(as, " %d", digit);
      arg = arg * 10 + digit;
      i++;
    } else if (key == (KBD_CTRL | 'u'))
      break;
    else if (key == '-' && i == 0) {
      if (sgn > 0) {
        sgn = -sgn;
        astr_cat_cstr(as, " -");
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
}
END_DEFUN

DEFUN("back-to-indentation", back_to_indentation)
/*+
Move point to the first non-whitespace character on this line.
+*/
{
  buf.pt.o = 0;
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

  for (;;) {
    while (!eolp()) {
      int c = following_char();
      if (!ISWORDCHAR(c)) {
        if (gotword)
          return TRUE;
      } else
        gotword = TRUE;
      buf.pt.o++;
    }
    if (gotword)
      return TRUE;
    buf.pt.o = astr_len(buf.pt.p->item);
    if (!edit_navigate_down_line())
      break;
    buf.pt.o = 0;
  }
  return FALSE;
}

DEFUN("forward-word", forward_word)
/*+
Move point forward one word.
+*/
{
  ok = forward_word();
}
END_DEFUN

static int backward_word(void)
{
  int gotword = FALSE;

  for (;;) {
    if (bolp()) {
      if (!edit_navigate_up_line())
        break;
      buf.pt.o = astr_len(buf.pt.p->item);
    }
    while (!bolp()) {
      int c = preceding_char();
      if (!ISWORDCHAR(c)) {
        if (gotword)
          return TRUE;
      } else
        gotword = TRUE;
      buf.pt.o--;
    }
    if (gotword)
      return TRUE;
  }
  return FALSE;
}

DEFUN("backward-word", backward_word)
/*+
Move backward until encountering the beginning of a word.
+*/
{
  ok = backward_word();
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

static int forward_sexp(void)
{
  int gotsexp = FALSE;
  int level = 0;
  int double_quote = 0;
  int single_quote = 0;

  for (;;) {
    while (!eolp()) {
      int c = following_char();

      /* Jump quotes that aren't sexp separators. */
      if (c == '\\'
          && buf.pt.o+1 < astr_len(buf.pt.p->item)
          && ((*astr_char(buf.pt.p->item, (ptrdiff_t)(buf.pt.o + 1))
               == '\"') ||
              (*astr_char(buf.pt.p->item, (ptrdiff_t)(buf.pt.o + 1))
                  == '\''))) {
        buf.pt.o++;
        c = 'a'; /* Convert \' and \" into a word char. */
      }

      CONTROL_SEXP_LEVEL(ISOPENBRACKETCHAR,
                         ISCLOSEBRACKETCHAR);

      buf.pt.o++;

      if (!ISSEXPCHAR(c)) {
        if (gotsexp && level == 0) {
          if (!ISSEXPSEPARATOR(c))
            buf.pt.o--;
          return TRUE;
        }
      }
      else
        gotsexp = TRUE;
    }
    if (gotsexp && level == 0)
      return TRUE;
    buf.pt.o = astr_len(buf.pt.p->item);
    if (!edit_navigate_down_line()) {
      if (level != 0)
        minibuf_error("Scan error: \"Unbalanced parentheses\"");
      break;
    }
    buf.pt.o = 0;
  }
  return FALSE;
}

DEFUN("forward-sexp", forward_sexp)
/*+
Move forward across one balanced expression (sexp).
+*/
{
  ok = forward_sexp();
}
END_DEFUN

static int backward_sexp(void)
{
  int gotsexp = FALSE;
  int level = 0;
  int double_quote = 1;
  int single_quote = 1;

  for (;;) {
    if (bolp()) {
      if (!edit_navigate_up_line()) {
        if (level != 0)
          minibuf_error("Scan error: \"Unbalanced parentheses\"");
        break;
      }
      buf.pt.o = astr_len(buf.pt.p->item);
    }
    while (!bolp()) {
      int c = preceding_char();

      /* Jump quotes that doesn't are sexp separators.  */
      if (((c == '\'') || (c == '\"'))
          && buf.pt.o-1 > 0
          && (*astr_char(buf.pt.p->item, (ptrdiff_t)(buf.pt.o - 2))
              == '\\')) {
        buf.pt.o--;
        c = 'a'; /* Convert \' and \" like a
                    word char */
      }

      CONTROL_SEXP_LEVEL(ISCLOSEBRACKETCHAR,
                         ISOPENBRACKETCHAR);

      buf.pt.o--;

      if (!ISSEXPCHAR(c)) {
        if (gotsexp && level == 0) {
          if (!ISSEXPSEPARATOR(c))
            buf.pt.o++;
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

DEFUN("backward-sexp", backward_sexp)
/*+
Move backward across one balanced expression (sexp).
+*/
{
  ok = backward_sexp();
}
END_DEFUN

DEFUN("mark-word", mark_word)
/*+
Set mark ARG words away from point.
+*/
{
  FUNCALL(set_mark_command);
  if ((ok = FUNCALL(forward_word)))
    FUNCALL(exchange_point_and_mark);
}
END_DEFUN

DEFUN("mark-sexp", mark_sexp)
/*+
Set mark a sexp from point.
+*/
{
  FUNCALL(set_mark_command);
  if ((ok = FUNCALL(forward_sexp)))
    FUNCALL(exchange_point_and_mark);
}
END_DEFUN

DEFUN("backward-paragraph", backward_paragraph)
/*+
Move backward to start of paragraph.
+*/
{
  while (is_empty_line() && edit_navigate_up_line())
    ;
  while (!is_empty_line() && edit_navigate_up_line())
    ;

  FUNCALL(beginning_of_line);
}
END_DEFUN

DEFUN("forward-paragraph", forward_paragraph)
/*+
Move forward to end of paragraph.
+*/
{
  while (is_empty_line() && edit_navigate_down_line())
    ;
  while (!is_empty_line() && edit_navigate_down_line())
    ;

  if (is_empty_line())
    FUNCALL(beginning_of_line);
  else
    FUNCALL(end_of_line);
}
END_DEFUN

DEFUN("mark-paragraph", mark_paragraph)
/*+
Put point at beginning of this paragraph, mark at end.
The paragraph marked is the one that contains point or follows point.
+*/
{
  FUNCALL(forward_paragraph);
  FUNCALL(set_mark_command);
  FUNCALL(backward_paragraph);
}
END_DEFUN

DEFUN("fill-paragraph", fill_paragraph)
/*+
Fill paragraph at or after point.
+*/
{
  int i, start, end;
  Marker *m = point_marker();

  undo_save(UNDO_START_SEQUENCE, buf.pt, 0, 0, FALSE);

  FUNCALL(forward_paragraph);
  end = buf.pt.n;
  if (is_empty_line())
    end--;

  FUNCALL(backward_paragraph);
  start = buf.pt.n;
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

  buf.pt = m->pt;
  free_marker(m);

  undo_save(UNDO_END_SEQUENCE, buf.pt, 0, 0, FALSE);
}
END_DEFUN

#define UPPERCASE		1
#define LOWERCASE		2
#define CAPITALIZE		3

static int setcase_word(int rcase)
{
  int gotword;
  size_t i, size;

  if (!ISWORDCHAR(following_char())) {
    if (!forward_word())
      return FALSE;
    if (!backward_word())
      return FALSE;
  }

  i = buf.pt.o;
  while (i < astr_len(buf.pt.p->item)) {
    if (!ISWORDCHAR(*astr_char(buf.pt.p->item, (ptrdiff_t)i)))
      break;
    ++i;
  }
  size = i-buf.pt.o;
  if (size > 0)
    undo_save(UNDO_REPLACE_BLOCK, buf.pt, size, size, FALSE);

  gotword = FALSE;
  while (buf.pt.o < astr_len(buf.pt.p->item)) {
    char *p = astr_char(buf.pt.p->item, (ptrdiff_t)buf.pt.o);
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
    buf.pt.o++;
  }

  buf.flags |= BFLAG_MODIFIED;

  return TRUE;
}

DEFUN("downcase-word", downcase_word)
/*+
Convert following word to lower case, moving over.
+*/
{
  ok = setcase_word(LOWERCASE);
}
END_DEFUN

DEFUN("upcase-word", upcase_word)
/*+
Convert following word to upper case, moving over.
+*/
{
  ok = setcase_word(UPPERCASE);
}
END_DEFUN

DEFUN("capitalize-word", capitalize_word)
/*+
Capitalize the following word, moving over.
This gives the word a first character in upper case and the rest
lower case.
+*/
{
  ok = setcase_word(CAPITALIZE);
}
END_DEFUN

DEFUN("execute-command", execute_command)
/*+
Read command or macro name, then call it.
FIXME: Make it work non-interactively.
+*/
{
  astr name, msg = astr_new();
  Function func;
  Macro *mp;

  astr_cat_cstr(msg, "M-x ");

  name = minibuf_read_function_name(astr_cstr(msg));
  astr_delete(msg);
  if (name == NULL)
    return FALSE;

  if ((func = get_function(astr_cstr(name))))
    ok = func(0, 0, NULL);
  else if ((mp = get_macro(astr_cstr(name))))
    call_macro(mp);
  else
    ok = FALSE;

  astr_delete(name);
}
END_DEFUN

DEFUN("shell-command", shell_command)
/*+
Reads a line of text using the minibuffer and creates an inferior shell
to execute the line as a command; passes the contents of the region as
input to the shell command.
If the shell command produces any output, it is inserted into the
current buffer, overwriting the current region.
+*/
{
  astr ms;

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
        size_t i;

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

        undo_save(UNDO_START_SEQUENCE, buf.pt, 0, 0, FALSE);
        calculate_the_region(&r);
        if (buf.pt.p != r.start.p
            || r.start.o != buf.pt.o)
          FUNCALL(exchange_point_and_mark);
        /* FIXME: implement this with a repeat call */
        for (i = 0; i < r.size; i++)
          FUNCALL(delete_char);
        ok = insert_nstring(out, "\n", FALSE);
        undo_save(UNDO_END_SEQUENCE, buf.pt, 0, 0, FALSE);

        astr_delete(out);
      }

      astr_delete(cmd);
    }
  }
}
END_DEFUN
