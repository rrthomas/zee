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
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

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
  return TRUE;
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
  return cancel();
}
END_DEFUN

static char *make_buffer_flags(Buffer *bp, int iscurrent)
{
  static char buf[4];

  buf[0] = iscurrent ? '.' : ' ';
  buf[1] = (bp->flags & BFLAG_MODIFIED) ? '*' : ' ';
  /* Display the readonly flag if it is set or the buffer is
     the current buffer, i.e. the `*Buffer List*' buffer. */
  buf[2] = (bp->flags & BFLAG_READONLY || bp == cur_bp) ? '%' : ' ';
  buf[3] = '\0';

  return buf;
}

static astr make_buffer_modeline(Buffer *bp)
{
  astr as = astr_new();

  if (bp->flags & BFLAG_AUTOFILL)
    astr_cat_cstr(as, " Fill");

  return as;
}

static void print_buf(Buffer *old_bp, Buffer *bp)
{
  astr mode = make_buffer_modeline(bp);

  if (bp->name[0] == ' ')
    return;

  bprintf("%3s %-16s %6u  %-13s",
          make_buffer_flags(bp, old_bp == bp),
          bp->name,
          calculate_buffer_size(bp),
          astr_cstr(mode));
  astr_delete(mode);
  if (bp->filename != NULL) {
    astr shortname = shorten_string(bp->filename, 40);
    insert_string(astr_cstr(shortname));
    astr_delete(shortname);
  }
  insert_newline();
}

void write_temp_buffer(const char *name, void (*func)(va_list ap), ...)
{
  Window *wp, *old_wp = cur_wp;
  Buffer *new_bp;
  va_list ap;

  /* Popup a window with the buffer "name". */
  if ((wp = find_window(name)))
    set_current_window(wp);
  else {
    set_current_window(popup_window());
    switch_to_buffer(find_buffer(name, TRUE));
  }

  /* Remove all the content of that buffer. */
  new_bp = create_buffer(cur_bp->name);
  kill_buffer(cur_bp);
  cur_bp = cur_wp->bp = new_bp;

  /* Make the buffer like a temporary one. */
  cur_bp->flags = BFLAG_NEEDNAME | BFLAG_NOSAVE | BFLAG_NOUNDO;
  set_temporary_buffer(cur_bp);

  /* Use the "callback" routine. */
  va_start(ap, func);
  func(ap);
  va_end(ap);

  gotobob();
  cur_bp->flags |= BFLAG_READONLY;

  /* Restore old current window. */
  set_current_window(old_wp);
}

static void write_buffers_list(va_list ap)
{
  Window *old_wp = va_arg(ap, Window *);
  Buffer *bp;

  bprintf(" MR Buffer           Size    Mode         File\n");
  bprintf(" -- ------           ----    ----         ----\n");

  /* Print buffers. */
  bp = old_wp->bp;
  do {
    /* Print all buffer less this one (the *Buffer List*). */
    if (cur_bp != bp)
      print_buf(old_wp->bp, bp);
    if ((bp = bp->next) == NULL)
      bp = head_bp;
  } while (bp != old_wp->bp);
}

DEFUN_INT("list-buffers", list_buffers)
/*+
Display a list of names of existing buffers.
The list is displayed in a buffer named `*Buffer List*'.
Note that buffers with names starting with spaces are omitted.

The M column contains a * for buffers that are modified.
The R column contains a % for buffers that are read-only.
+*/
{
  write_temp_buffer("*Buffer List*", write_buffers_list, cur_wp);
  return TRUE;
}
END_DEFUN

DEFUN_INT("toggle-read-only", toggle_read_only)
/*+
Change whether this buffer is visiting its file read-only.
+*/
{
  cur_bp->flags ^= BFLAG_READONLY;
  return TRUE;
}
END_DEFUN

DEFUN_INT("auto-fill-mode", auto_fill_mode)
/*+
Toggle Auto Fill mode.
In Auto Fill mode, inserting a space at a column beyond `fill-column'
automatically breaks the line at a previous space.
+*/
{
  cur_bp->flags ^= BFLAG_AUTOFILL;
  return TRUE;
}
END_DEFUN

DEFUN_INT("set-fill-column", set_fill_column)
/*+
Set the fill column.
If an argument value is passed, set the `fill-column' variable with
that value, otherwise with the current column value.
+*/
{
  if (lastflag & FLAG_SET_UNIARG)
    variableSetNumber(&cur_bp->vars, "fill-column", uniarg);
  else
    variableSetNumber(&cur_bp->vars, "fill-column", (int)(cur_bp->pt.o + 1));

  return TRUE;
}
END_DEFUN

int set_mark_command(void)
{
  set_mark();
  minibuf_write("Mark set");
  return TRUE;
}

DEFUN_INT("set-mark-command", set_mark_command)
/*+
Set mark at where point is.
+*/
{
  int ret = set_mark_command();
  anchor_mark();
  return ret;
}
END_DEFUN

int exchange_point_and_mark(void)
{
  /* No mark? */
  assert(cur_bp->mark);

  /* Swap the point with the mark.  */
  swap_point(&cur_bp->pt, &cur_bp->mark->pt);
  return TRUE;
}


DEFUN_INT("exchange-point-and-mark", exchange_point_and_mark)
/*+
Put the mark where point is now, and point where the mark is now.
+*/
{
  if (!exchange_point_and_mark())
    return FALSE;

  anchor_mark();

  thisflag |= FLAG_NEED_RESYNC;

  return TRUE;
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
  return TRUE;
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
  c = term_xgetkey(GETKEY_UNFILTERED, 0);

  if (isdigit(c) && c - '0' < 8)
    quoted_insert_octal(c);
  else
    insert_char(c);

  minibuf_clear();

  return TRUE;
}
END_DEFUN

int universal_argument(int keytype, int xarg)
{
  int i = 0, arg = 4, sgn = 1, digit;
  size_t key;
  astr as = astr_new();

  if (keytype == KBD_META) {
    astr_cpy_cstr(as, "ESC");
    term_ungetkey((size_t)(xarg + '0'));
  } else
    astr_cpy_cstr(as, "C-u");

  for (;;) {
    astr_cat_cstr(as, "-"); /* Add the '-' character. */
    key = do_completion(as);
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
    } else if (key == (KBD_CTL | 'u')) {
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
        /* If i == 0 do nothing (the Emacs behavior is a little
           strange in this case, it waits for one more key that is
           eaten, and then goes back to the normal state). */
        term_ungetkey(key);
        break;
      }
    } else {
      term_ungetkey(key);
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
  return universal_argument(KBD_CTL | 'u', 0);
}
END_DEFUN

DEFUN_INT("back-to-indentation", back_to_indentation)
/*+
Move point to the first non-whitespace character on this line.
+*/
{
  cur_bp->pt = line_beginning_position(0);
  while (!eolp()) {
    if (!isspace(following_char()))
      break;
    forward_char();
  }
  return TRUE;
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
      cur_bp->pt.o++;
    }
    if (gotword)
      return TRUE;
    cur_bp->pt.o = astr_len(cur_bp->pt.p->item);
    if (!next_line())
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
    return FUNCALL_ARG(backward_word, -uniarg);

  for (uni = 0; uni < uniarg; ++uni)
    if (!forward_word())
      return FALSE;

  return TRUE;
}
END_DEFUN

static int backward_word(void)
{
  int gotword = FALSE;
  for (;;) {
    if (bolp()) {
      if (!previous_line())
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
    return FUNCALL_ARG(forward_word, -uniarg);

  for (uni = 0; uni < uniarg; ++uni)
    if (!backward_word())
      return FALSE;

  return TRUE;
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
    if (!next_line()) {
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
    return FUNCALL_ARG(backward_sexp, -uniarg);

  for (uni = 0; uni < uniarg; ++uni)
    if (!forward_sexp())
      return FALSE;

  return TRUE;
}
END_DEFUN

int backward_sexp(void)
{
  int gotsexp = FALSE;
  int level = 0;
  int double_quote = 1;
  int single_quote = 1;

  for (;;) {
    if (bolp()) {
      if (!previous_line()) {
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
    return FUNCALL_ARG(forward_sexp, -uniarg);

  for (uni = 0; uni < uniarg; ++uni)
    if (!backward_sexp())
      return FALSE;

  return TRUE;
}
END_DEFUN

DEFUN_INT("mark-word", mark_word)
/*+
Set mark ARG words away from point.
+*/
{
  int ret;
  FUNCALL(set_mark_command);
  ret = FUNCALL_ARG(forward_word, uniarg);
  if (ret)
    FUNCALL(exchange_point_and_mark);
  return ret;
}
END_DEFUN

DEFUN_INT("mark-sexp", mark_sexp)
/*+
Set mark ARG sexps from point.
The place mark goes is the same place C-M-f would
move to with the same argument.
+*/
{
  int ret;
  FUNCALL(set_mark_command);
  ret = FUNCALL_ARG(forward_sexp, uniarg);
  if (ret)
    FUNCALL(exchange_point_and_mark);
  return ret;
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
      if (!previous_line())
        return FALSE;
  } else {
    if (uniarg == 0)
      uniarg = 1;

    while (uniarg--)
      if (!next_line())
        return FALSE;
  }

  return TRUE;
}
END_DEFUN

DEFUN_INT("backward-paragraph", backward_paragraph)
/*+
Move backward to start of paragraph.  With argument N, do it N times.
+*/
{
  if (uniarg < 0)
    return FUNCALL_ARG(forward_paragraph, -uniarg);

  do {
    while (previous_line() && is_empty_line());
    while (previous_line() && !is_empty_line());
  } while (--uniarg > 0);

  FUNCALL(beginning_of_line);

  return TRUE;
}
END_DEFUN

DEFUN_INT("forward-paragraph", forward_paragraph)
/*+
Move forward to end of paragraph.  With argument N, do it N times.
+*/
{
  if (uniarg < 0)
    return FUNCALL_ARG(backward_paragraph, -uniarg);

  do {
    while (next_line() && is_empty_line());
    while (next_line() && !is_empty_line());
  } while (--uniarg > 0);

  if (is_empty_line())
    FUNCALL(beginning_of_line);
  else
    FUNCALL(end_of_line);

  return TRUE;
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

  return TRUE;
}
END_DEFUN

DEFUN_INT("fill-paragraph", fill_paragraph)
/*+
Fill paragraph at or after point.
+*/
{
  int i, start, end;
  Marker *m = point_marker();

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);

  FUNCALL(forward_paragraph);
  end = cur_bp->pt.n;
  if (is_empty_line())
    end--;

  FUNCALL(backward_paragraph);
  start = cur_bp->pt.n;
  if (is_empty_line()) {  /* Move to next line if between two paragraphs. */
    next_line();
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

  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

  return TRUE;
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

  i = cur_bp->pt.o;
  while (i < astr_len(cur_bp->pt.p->item)) {
    if (!ISWORDCHAR(*astr_char(cur_bp->pt.p->item, (ptrdiff_t)i)))
      break;
    ++i;
  }
  size = i-cur_bp->pt.o;
  if (size > 0)
    undo_save(UNDO_REPLACE_BLOCK, cur_bp->pt, size, size);

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
  int uni, ret = TRUE;

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
  for (uni = 0; uni < uniarg; ++uni)
    if (!setcase_word(LOWERCASE)) {
      ret = FALSE;
      break;
    }
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

  return ret;
}
END_DEFUN

DEFUN_INT("upcase-word", upcase_word)
/*+
Convert following word (or argument N words) to upper case, moving over.
+*/
{
  int uni, ret = TRUE;

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
  for (uni = 0; uni < uniarg; ++uni)
    if (!setcase_word(UPPERCASE)) {
      ret = FALSE;
      break;
    }
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

  return ret;
}
END_DEFUN

DEFUN_INT("capitalize-word", capitalize_word)
/*+
Capitalize the following word (or argument N words), moving over.
This gives the word(s) a first character in upper case and the rest
lower case.
+*/
{
  int uni, ret = TRUE;

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
  for (uni = 0; uni < uniarg; ++uni)
    if (!setcase_word(CAPITALIZE)) {
      ret = FALSE;
      break;
    }
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);

  return ret;
}
END_DEFUN

static void write_shell_output(va_list ap)
{
  astr out = va_arg(ap, astr);

  insert_string((char *)astr_cstr(out));
}

DEFUN_INT("shell-command", shell_command)
/*+
Reads a line of text using the minibuffer and creates an inferior shell.
to execute the line as a command.
Standard input from the command comes from the null device.  If the
shell command produces any output, the output goes to a buffer
named `*Shell Command Output*', which is displayed in another window
but not selected.
If the output is one line, it is displayed in the echo area.
A numeric argument, as in `M-1 M-!' or `C-u M-!', directs this
command to insert any output into the current buffer.
+*/
{
  char *ms;
  FILE *pipe;
  astr out, s;
  int lines = 0;
  astr cmd;

  if ((ms = minibuf_read("Shell command: ", "")) == NULL)
    return cancel();
  if (ms[0] == '\0')
    return FALSE;

  cmd = astr_new();
  astr_afmt(cmd, "%s 2>&1 </dev/null", ms);
  if ((pipe = popen(astr_cstr(cmd), "r")) == NULL) {
    minibuf_error("Cannot open pipe to process");
    return FALSE;
  }
  astr_delete(cmd);

  out = astr_new();
  while ((s = astr_fgets(pipe)) != NULL) {
    ++lines;
    astr_cat(out, s);
    astr_cat_cstr(out, "\n");
    astr_delete(s);
  }
  pclose(pipe);

  if (lines == 0)
    minibuf_write("(Shell command succeeded with no output)");
  else { /* lines >= 1 */
    if (lastflag & FLAG_SET_UNIARG)
      insert_string((char *)astr_cstr(out));
    else {
      if (lines > 1)
        write_temp_buffer("*Shell Command Output*",
                          write_shell_output, out);
      else /* lines == 1 */
        minibuf_write("%s", astr_cstr(out));
    }
  }
  astr_delete(out);

  return TRUE;
}
END_DEFUN

DEFUN_INT("shell-command-on-region", shell_command_on_region)
/*+
Reads a line of text using the minibuffer and creates an inferior shell.
to execute the line as a command; passes the contents of the region as
input to the shell command.
If the shell command produces any output, the output goes to a buffer
named `*Shell Command Output*', which is displayed in another window
but not selected.
If the output is one line, it is displayed in the echo area.
A numeric argument, as in `M-1 M-|' or `C-u M-|', directs output to the
current buffer, then the old region is deleted first and the output replaces
it as the contents of the region.
+*/
{
  char *ms;
  FILE *pipe;
  astr out, s;
  int lines = 0;
  astr cmd;
  char tempfile[] = P_tmpdir "/" BIN_NAME "XXXXXX";

  if ((ms = minibuf_read("Shell command: ", "")) == NULL)
    return cancel();
  if (ms[0] == '\0')
    return FALSE;

  if (warn_if_no_mark())
    return FALSE;

  cmd = astr_new();

  {
    Region r;
    char *p;
    int fd;

    fd = mkstemp(tempfile);
    if (fd == -1) {
      minibuf_error("Cannot open temporary file");
      return FALSE;
    }

    calculate_the_region(&r);
    p = copy_text_block(r.start.n, r.start.o, r.size);
    write(fd, p, r.size);
    free(p);

    close(fd);

    astr_afmt(cmd, "%s 2>&1 <%s", ms, tempfile);
  }

  if ((pipe = popen(astr_cstr(cmd), "r")) == NULL) {
    minibuf_error("Cannot open pipe to process");
    return FALSE;
  }
  astr_delete(cmd);

  out = astr_new();
  while (astr_len(s = astr_fgets(pipe)) > 0) {
    ++lines;
    astr_cat(out, s);
    astr_cat_cstr(out, "\n");
    astr_delete(s);
  }
  astr_delete(s);
  pclose(pipe);
  remove(tempfile);

  if (lines == 0)
    minibuf_write("(Shell command succeeded with no output)");
  else { /* lines >= 1 */
    if (lastflag & FLAG_SET_UNIARG) {
      undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
      {
        Region r;
        calculate_the_region(&r);
        if (cur_bp->pt.p != r.start.p
            || r.start.o != cur_bp->pt.o)
          FUNCALL(exchange_point_and_mark);
        undo_save(UNDO_INSERT_BLOCK, cur_bp->pt, r.size, 0);
        undo_nosave = TRUE;
        while (r.size--)
          FUNCALL(delete_char);
        undo_nosave = FALSE;
      }
      insert_string((char *)astr_cstr(out));
      undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
    } else {
      if (lines > 1)
        write_temp_buffer("*Shell Command Output*",
                          write_shell_output, out);
      else /* lines == 1 */
        minibuf_write("%s", astr_cstr(out));
    }
  }
  astr_delete(out);

  return TRUE;
}
END_DEFUN
