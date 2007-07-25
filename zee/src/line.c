/* Line-oriented editing functions
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

#include <ctype.h>
#include <stdbool.h>

#include "main.h"
#include "extern.h"


/*--------------------------------------------------------------------------
 * Markers
 *--------------------------------------------------------------------------*/

void remove_marker(Marker *marker)
{
  Marker *m, *next, *prev = NULL;

  for (m = buf->markers; m; m = next) {
    next = m->next;
    if (m == marker) {
      if (prev)
        prev->next = next;
      else
        buf->markers = next;
      break;
    }
    prev = m;
  }
}

Marker *marker_new(Point pt)
{
  Marker *marker = zmalloc(sizeof(Marker));
  marker->next = buf->markers;
  buf->markers = marker;
  marker->pt = pt;
  return marker;
}

Marker *point_marker(void)
{
  return marker_new(buf->pt);
}

static void adjust_markers(size_t line, size_t pointo, ssize_t offset)
{
  Marker *m = point_marker(), *marker;

  for (marker = buf->markers; marker; marker = marker->next)
    if (marker->pt.n == line && (marker->pt.o >= pointo + (offset < 0)))
      marker->pt.o += offset;

  buf->pt = m->pt;
  remove_marker(m);
}

/*
 * Return the current mark
 */
Marker *get_mark(void)
{
  assert(buf->mark);
  return marker_new(buf->mark->pt);
}

/*
 * Set the current mark
 */
void set_mark(Marker *m)
{
  buf->mark->pt = m->pt;
}

/*
 * Set the mark to point
 */
void set_mark_to_point(void)
{
  buf->mark->pt = buf->pt;
}


/*
 * Return a flag indicating whether the current line is empty
 */
bool is_empty_line(void)
{
  return rblist_line_length(buf->lines, buf->pt.n) == 0;
}

/*
 * Return a flag indicating whether the current line is blank
 */
bool is_blank_line(void)
{
  RBLIST_FOR(c, rblist_line(buf->lines, buf->pt.n))
    if (!isspace(c))
      return false;
  RBLIST_END
  return true;
}

/*
 * Return the character following point in the current buffer
 */
int following_char(void)
{
  if (eobp())
    return '\0';
  else if (eolp())
    return '\n';
  else
    return rblist_get(buf->lines, rblist_line_to_start_pos(buf->lines, buf->pt.n) + buf->pt.o);
}

/*
 * Return the character preceding point in the current buffer
 */
int preceding_char(void)
{
  if (bobp())
    return '\0';
  else if (bolp())
    return '\n';
  else
    return rblist_get(buf->lines, rblist_line_to_start_pos(buf->lines, buf->pt.n) + buf->pt.o - 1);
}

/*
 * Return flag indicating whether point is at the beginning of the buffer
 */
bool bobp(void)
{
  return (bolp() && buf->pt.n == 0);
}

/*
 * Return a flag indicating whether point is at the end of the buffer
 */
bool eobp(void)
{
  return (eolp() && buf->pt.n == rblist_nl_count(buf->lines) - 1);
}

/*
 * Return a flag indicating whether point is at the beginning of a line
 */
bool bolp(void)
{
  return buf->pt.o == 0;
}

/*
 * Return a flag indicating whether point is at the end of a line
 */
bool eolp(void)
{
  return buf->pt.o == rblist_line_length(buf->lines, buf->pt.n);
}

/*
 * Insert the character `c' at the current point position
 * into the current buffer.
 */
bool insert_char(int c)
{
  return replace_nstring(0, NULL, rblist_singleton(c));
}

DEF(edit_insert_tab,
"\
Indent to next multiple of `indent_width'.\
")
{
  if (warn_if_readonly_buffer())
    ok = false;
  else {
    size_t c = get_goalc();
    size_t t = indent_width();

    for (c = t - c % t; c > 0; c--)
      insert_char(' ');
  }
}
END_DEF

/*
 * Check the case of a string.
 * Returns 2 if it is all upper case, 1 if the first character is
 * upper case, or 0 otherwise.
 */
static unsigned check_case(rblist as)
{
  if (!isupper(rblist_get(as, 0)))
    return 0;

  RBLIST_FOR(c, rblist_sub(as, 1, rblist_length(as)))
    if (!isupper(c))
      return 1;
  RBLIST_END

  return 2;
}

/*
 * Recase str according to case of tmpl.
 */
static rblist recase(rblist str, rblist tmpl)
{
  int c;
  unsigned tmpl_case = check_case(tmpl);
  rblist ret = rblist_empty;

  assert(rblist_length(str) > 0);
  assert(rblist_length(str) == rblist_length(tmpl));

  c = rblist_get(str, 0);
  if (tmpl_case >= 1)
    c = toupper(c);
  ret = rblist_append(ret, c);

  RBLIST_FOR(c, rblist_sub(str, 1, rblist_length(str)))
    if (tmpl_case == 2)
      c = toupper(c);
    ret = rblist_append(ret, c);
  RBLIST_END

  return ret;
}

/*
 * Replace text in the line `line' with newtext. If replace_case is
 * true then the new characters will be the same case as the old.
 * Return flag indicating whether modifications have been made.
 */
bool line_replace_text(size_t line, size_t offset, size_t oldlen,
                      rblist newtext, bool replace_case)
{
  bool changed = false;

  if (oldlen > 0) {
    if (replace_case)
      newtext = recase(newtext, rblist_sub(buf->lines, rblist_line_to_start_pos(buf->lines, line) + offset, offset + oldlen));

    buf->lines = rblist_concat(rblist_sub(buf->lines, 0, rblist_line_to_start_pos(buf->lines, line) + offset),
                               rblist_concat(newtext,
                                             rblist_sub(buf->lines, rblist_line_to_start_pos(buf->lines, line) + offset + rblist_length(newtext), rblist_length(buf->lines))));
    changed = true;
  }

  return changed;
}

/*
 * If point is greater than wrap_column, then split the line at the
 * right-most space character at or before wrap_column, if there is
 * one, or at the left-most at or after wrap_column, if not. If the
 * line contains no spaces, no break is made.
 */
bool wrap_break_line(void)
{
  size_t i, break_col = 0, excess = 0, old_col;
  size_t wrap_col = get_variable_number(rblist_from_string("wrap_column"));

  // If we're not beyond wrap_column, stop now.
  if (get_goalc() <= wrap_col)
    return false;

  // Move cursor back to wrap_column
  old_col = buf->pt.o;
  while (get_goalc() > wrap_col + 1) {
    buf->pt.o--;
    excess++;
  }

  // Find break point moving left from wrap_column.
  for (i = buf->pt.o; i > 0; i--) {
    int c = rblist_get(buf->lines, rblist_line_to_start_pos(buf->lines, buf->pt.n) + i - 1);
    if (isspace(c)) {
      break_col = i;
      break;
    }
  }

  /* If no break point moving left from wrap_column, find first
     possible moving right. */
  if (break_col == 0) {
    for (i = buf->pt.o + 1; i < rblist_line_length(buf->lines, buf->pt.n); i++) {
      int c = rblist_get(buf->lines, rblist_line_length(buf->lines, buf->pt.n) + i - 1);
      if (isspace(c)) {
        break_col = i;
        break;
      }
    }
  }

  if (break_col >= 1) {
    // Break line.
    size_t last_col = buf->pt.o - break_col;
    buf->pt.o = break_col;
    CMDCALL(delete_horizontal_space);
    insert_char('\n');
    buf->pt.o = last_col + excess;
    return true;
  } else {
    // Undo fiddling with point.
    buf->pt.o = old_col;
    return false;
  }
}

DEF(edit_insert_newline,
"\
Insert a newline, wrapping if in Wrap mode.\
")
{
  undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);
  if (buf->flags & BFLAG_AUTOFILL &&
      get_goalc() > (size_t)get_variable_number(rblist_from_string("wrap_column")))
    wrap_break_line();
  ok = insert_char('\n');
  undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);
}
END_DEF

/*
 * Replace `size' characters at point with the string `repl'.
 * If `ret' is non-NULL, return the characters replaced in it.
 * Return `true' if the replacement is performed without incident, or
 * `false' if the buffer is read-only or if there are fewer than
 * `size' characters after point (in which case nothing is inserted).
 * FIXME: Check the contract by looking at the callers.
 */
bool replace_nstring(size_t size, rblist *ret, rblist repl)
{
  if (warn_if_readonly_buffer())
    return false;

  if (size || (repl && rblist_length(repl)))
    undo_save(UNDO_REPLACE_BLOCK, buf->pt, size, repl ? rblist_length(repl) : 0);

  if (size) {
    if (ret)
      *ret = rblist_empty;
    buf->flags &= ~BFLAG_ANCHORED;

    while (size--) {
      if (ret) {
        if (!eolp())
          *ret = rblist_append(*ret, following_char());
        else
          *ret = rblist_append(*ret, '\n');
      }

      if (eobp()) {
        minibuf_error(rblist_from_string("End of buffer"));
        return false;
      }

      buf->lines = rblist_concat(rblist_sub(buf->lines, 0, rblist_line_to_start_pos(buf->lines, buf->pt.n) + buf->pt.o),
                                 rblist_sub(buf->lines, rblist_line_to_start_pos(buf->lines, buf->pt.n) + buf->pt.o + 1,
                                                          rblist_length(buf->lines)));
      adjust_markers(buf->pt.n, buf->pt.o, -1);
      buf->flags |= BFLAG_MODIFIED;
    }
  }

  // Insert string.
  if (repl && rblist_length(repl)) {
    buf->lines = rblist_concat(rblist_concat(rblist_sub(buf->lines, 0, rblist_line_to_start_pos(buf->lines, buf->pt.n) + buf->pt.o), repl),
                               rblist_sub(buf->lines, rblist_line_to_start_pos(buf->lines, buf->pt.n) + buf->pt.o, rblist_length(buf->lines)));
    buf->flags |= BFLAG_MODIFIED;
    adjust_markers(buf->pt.n, buf->pt.o, (ssize_t)rblist_length(repl));
    for (size_t i = 0; i < rblist_nl_count(repl); i++)
      CMDCALL(move_next_character);
  }

  return true;
}

DEF_ARG(edit_insert_character,
"\
Insert the character you type.\n\
Whichever character you type to run this command is inserted.\
",
UINT(c, "Insert character: "))
{
  if (ok) {
    undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);

    buf->flags &= ~BFLAG_ANCHORED;

    if (c <= 255) {
      if (isspace(c) && buf->flags & BFLAG_AUTOFILL &&
          get_goalc() > (size_t)get_variable_number(rblist_from_string("wrap_column")))
        wrap_break_line();
      insert_char((int)c);
    } else {
      ding();
      ok = false;
    }

    undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);
  }
}
END_DEF

DEF(edit_delete_next_character,
"\
Delete the following character.\n\
Join lines if the character is a newline.\
")
{
  ok = replace_nstring(1, NULL, NULL);
}
END_DEF

DEF(edit_delete_previous_character,
"\
Delete the previous character.\n\
Join lines if the character is a newline.\
")
{
  buf->flags &= ~BFLAG_ANCHORED;

  if (CMDCALL(move_previous_character))
    CMDCALL(edit_delete_next_character);
  else {
    minibuf_error(rblist_from_string("Beginning of buffer"));
    ok = false;
  }
}
END_DEF

DEF(delete_horizontal_space,
"\
Delete all spaces and tabs around point.\
")
{
  undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);

  while (!eolp() && isspace(following_char()))
    CMDCALL(edit_delete_next_character);

  while (!bolp() && isspace(preceding_char()))
    CMDCALL(edit_delete_previous_character);

  undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);
}
END_DEF

/*---------------------------------------------------------------------
			 Indentation command
  ---------------------------------------------------------------------*/

/*
 * Go to cur_goalc() in the previous non-blank line.
 */
static void previous_nonblank_goalc(void)
{
  size_t cur_goalc = get_goalc();

  // Find previous non-blank line.
  while (CMDCALL(move_previous_line) && is_blank_line());

  // Go to `cur_goalc' in that non-blank line.
  while (!eolp() && get_goalc() < cur_goalc)
    CMDCALL(move_next_character);
}

DEF(indent_relative,
"\
Indent line or insert a tab.\
")
{
  if (warn_if_readonly_buffer())
    ok = false;
  else {
    size_t cur_goalc = get_goalc(), target_goalc = 0;
    Marker *old_point = point_marker();

    buf->flags &= ~BFLAG_ANCHORED;
    previous_nonblank_goalc();

    // Now find the next blank char.
    if (!(preceding_char() == '\t' && get_goalc() > cur_goalc))
      while (!eolp() && !isspace(following_char()))
        CMDCALL(move_next_character);

    // Find next non-blank char.
    while (!eolp() && isspace(following_char()))
      CMDCALL(move_next_character);

    // Record target column.
    if (!eolp())
      target_goalc = get_goalc();

    buf->pt = old_point->pt;
    remove_marker(old_point);

    // Indent.
    undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);
    if (target_goalc > 0)
      /* If not at EOL on target line, insert spaces up to
         target_goalc. */
      while (get_goalc() < target_goalc)
        insert_char(' ');
    else
      // If already at EOL on target line, insert a tab.
      ok = CMDCALL(edit_insert_tab);
    undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);
  }
}
END_DEF

DEF(edit_insert_newline_and_indent,
"\
Insert a newline, then indent.\n\
Indentation is done using the `indent_relative' command, except\n\
that if there is a character in the first column of the line above,\n\
no indenting is performed.\
")
{
  if (warn_if_readonly_buffer())
    ok = false;
  else {
    buf->flags &= ~BFLAG_ANCHORED;

    if ((ok = insert_char('\n'))) {
      Marker *old_point = point_marker();

      undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);

      // Check where last non-blank goalc is.
      previous_nonblank_goalc();
      size_t pos = get_goalc();
      bool indent = pos > 0 || (!eolp() && isspace(following_char()));
      buf->pt = old_point->pt;
      remove_marker(old_point);
      /* Only indent if we're in column > 0 or we're in column 0 and
         there is a space character there in the last non-blank line. */
      if (indent)
        CMDCALL(indent_relative);

      undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);
    }
  }
}
END_DEF
