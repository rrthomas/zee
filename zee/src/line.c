/* Line-oriented editing functions
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

static void adjust_markers(Line *newlp, Line *oldlp, size_t pointo, int dir, int offset)
{
  Marker *m = point_marker(), *marker;

  for (marker = buf->markers; marker; marker = marker->next)
    if (marker->pt.p == oldlp &&
        (dir == -1 || marker->pt.o >= pointo + dir + (offset < 0))) {
      marker->pt.p = newlp;
      marker->pt.o -= pointo * dir - offset;
      marker->pt.n += dir;
    } else if (marker->pt.n > buf->pt.n)
      marker->pt.n += dir;

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
 * Create a Line list.
 */
Line *line_new(void)
{
  Line *lp = list_new();
  list_append(lp, rblist_empty);
  return lp;
}

/*
 * Read an rblist into a Line list.
 * FIXME: Since the introduction of rblists, this is never necessary.
 */
Line *string_to_lines(rblist as, size_t *lines)
{
  size_t p, q, end = rblist_length(as);
  Line *lp = line_new();

  for (p = 0, *lines = 1;
       p < end && (q = (astr_str(as, p, rblist_singleton('\n')))) != SIZE_MAX;
       p = q + 1) {
    list_last(lp)->item = rblist_concat(list_last(lp)->item, rblist_sub(as, p, q));
    list_append(lp, rblist_empty);
    ++*lines;
  }

  /* Add the rest of the string, if any */
  list_last(lp)->item = rblist_concat(list_last(lp)->item, rblist_sub(as, p, end));

  return lp;
}

/*
 * Return a flag indicating whether the current line is empty
 */
bool is_empty_line(void)
{
  return rblist_length(buf->pt.p->item) == 0;
}

/*
 * Return a flag indicating whether the current line is blank
 */
bool is_blank_line(void)
{
  size_t c;

  for (c = 0; c < rblist_length(buf->pt.p->item); c++)
    if (!isspace(rblist_get(buf->pt.p->item, c)))
      return false;
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
    return rblist_get(buf->pt.p->item, buf->pt.o);
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
    return rblist_get(buf->pt.p->item, (buf->pt.o - 1));
}

/*
 * Return flag indicating whether point is at the beginning of the buffer
 */
bool bobp(void)
{
  return (bolp() && list_prev(buf->pt.p) == buf->lines);
}

/*
 * Return a flag indicating whether point is at the end of the buffer
 */
bool eobp(void)
{
  return (eolp() && list_next(buf->pt.p) == buf->lines);
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
  return buf->pt.o == rblist_length(buf->pt.p->item);
}

/*
 * Insert the character `c' at the current point position
 * into the current buffer.
 */
bool insert_char(int c)
{
  rblist as = astr_afmt("%c", c);
  return replace_nstring(0, NULL, as);
}

DEF(edit_insert_tab,
"\
Indent to next multiple of `indent_width'.\
")
{
  if (warn_if_readonly_buffer())
    ok = false;
  else {
    int c = get_goalc();
    int t = indent_width();

    for (c = t - c % t; c > 0; c--)
      insert_char(' ');
  }
}
END_DEF

/*
 * Insert a newline at the current position without moving the cursor.
 * Update markers that point to the splitted line.
 */
static bool intercalate_newline(void)
{
  Line *new_lp;

  if (warn_if_readonly_buffer())
    return false;

  undo_save(UNDO_REPLACE_BLOCK, buf->pt, 0, 1);

  /* Update line linked list. */
  list_prepend(buf->pt.p, rblist_empty);
  new_lp = list_first(buf->pt.p);
  ++buf->num_lines;

  /* Move the text after the point into the new line. */
  new_lp->item = rblist_sub(buf->pt.p->item, buf->pt.o, rblist_length(buf->pt.p->item));
  buf->pt.p->item = rblist_sub(buf->pt.p->item, 0, buf->pt.o);

  adjust_markers(new_lp, buf->pt.p, buf->pt.o, 1, 0);

  buf->flags |= BFLAG_MODIFIED;
  thisflag |= FLAG_NEED_RESYNC;

  return true;
}

/*
 * Check the case of a string.
 * Returns 2 if it is all upper case, 1 if just the first letter is,
 * and 0 otherwise.
 */
static int check_case(rblist as)
{
  size_t i;

  if (!isupper(rblist_get(as, 0)))
    return 0;

  for (i = 1; i < rblist_length(as); i++)
    if (!isupper(rblist_get(as, i)))
      return 1;

  return 2;
}

/*
 * Recase str according to case of tmpl.
 */
static rblist recase(rblist str, rblist tmpl)
{
  size_t i;
  int tmpl_case = check_case(tmpl), c;
  rblist ret = rblist_empty;

  assert(rblist_length(str) > 0);
  assert(rblist_length(str) == rblist_length(tmpl));

  c = rblist_get(str, 0);
  if (tmpl_case >= 1)
    c = toupper(c);
  ret = rblist_append(ret, c);

  for (i = 1; i < rblist_length(str); i++) {
    c = rblist_get(str, i);
    if (tmpl_case == 2)
      c = toupper(c);
    ret = rblist_append(ret, c);
  }

  return ret;
}

/*
 * Replace text in the line lp with newtext. If replace_case is
 * true then the new characters will be the same case as the old.
 * Return flag indicating whether modifications have been made.
 */
bool line_replace_text(Line **lp, size_t offset, size_t oldlen,
                      rblist newtext, bool replace_case)
{
  bool changed = false;

  if (oldlen > 0) {
    if (replace_case && get_variable_bool(rblist_from_string("case_replace")))
      newtext = recase(newtext, rblist_sub((*lp)->item, offset, (offset + oldlen)));

    if (rblist_length(newtext) != oldlen) {
      (*lp)->item = rblist_concat(rblist_concat(rblist_sub((*lp)->item, 0, offset), newtext),
                             rblist_sub((*lp)->item, (offset + oldlen), rblist_length((*lp)->item)));
      adjust_markers(*lp, *lp, offset, 0, (int)(rblist_length(newtext) - oldlen));
      changed = true;
    } else if (rblist_ncompare(rblist_sub((*lp)->item, offset, rblist_length((*lp)->item)),
                                  newtext, rblist_length(newtext)) != 0) {
      (*lp)->item = rblist_concat(rblist_sub((*lp)->item, 0, (offset - 1)),
                             rblist_concat(newtext, rblist_sub((*lp)->item, (offset + rblist_length(newtext)), rblist_length((*lp)->item))));
/*       memcpy(rblist_get((*lp)->item, offset), */
/*              astr_to_string(newtext), rblist_length(newtext)); */
      changed = true;
    }
  }

  return changed;
}

/*
 * If point is greater than wrap_column, then split the line at the
 * right-most space character at or before wrap_column, if there is
 * one, or at the left-most at or after wrap_column, if not. If the
 * line contains no spaces, no break is made.
 */
void wrap_break_line(void)
{
  size_t i, break_col = 0, excess = 0, old_col;
  size_t wrapcol = get_variable_number(rblist_from_string("wrap_column"));

  /* If we're not beyond wrap_column, stop now. */
  if (get_goalc() <= wrapcol)
    return;

  /* Move cursor back to wrap_column */
  old_col = buf->pt.o;
  while (get_goalc() > wrapcol + 1) {
    buf->pt.o--;
    excess++;
  }

  /* Find break point moving left from wrap_column. */
  for (i = buf->pt.o; i > 0; i--) {
    int c = rblist_get(buf->pt.p->item, (i - 1));
    if (isspace(c)) {
      break_col = i;
      break;
    }
  }

  /* If no break point moving left from wrap_column, find first
     possible moving right. */
  if (break_col == 0) {
    for (i = buf->pt.o + 1; i < rblist_length(buf->pt.p->item); i++) {
      int c = rblist_get(buf->pt.p->item, (i - 1));
      if (isspace(c)) {
        break_col = i;
        break;
      }
    }
  }

  if (break_col >= 1) {
    /* Break line. */
    size_t last_col = buf->pt.o - break_col;
    buf->pt.o = break_col;
    CMDCALL(delete_horizontal_space);
    insert_char('\n');
    buf->pt.o = last_col + excess;
  } else
    /* Undo fiddling with point. */
    buf->pt.o = old_col;
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

bool replace_nstring(size_t size, rblist *as, rblist bs)
{
  if (warn_if_readonly_buffer())
    return false;

  if (size || (bs && rblist_length(bs)))
    undo_save(UNDO_REPLACE_BLOCK, buf->pt, size, bs ? rblist_length(bs) : 0);

  if (size) {
    assert(as);
    *as = rblist_empty;
    buf->flags &= ~BFLAG_ANCHORED;

    while (size--) {
      if (!eolp())
        *as = rblist_append(*as, following_char());
      else
        *as = rblist_append(*as, '\n');

      if (eobp()) {
        minibuf_error(rblist_from_string("End of buffer"));
        return false;
      }

      if (eolp()) {
        Line *lp1 = buf->pt.p;
        Line *lp2 = list_next(lp1);
        size_t lp1len = rblist_length(lp1->item);

        /* Move the next line of text into the current line. */
        lp2 = list_next(buf->pt.p);
        lp1->item = rblist_concat(lp1->item, list_next(buf->pt.p)->item);
        list_behead(lp1);
        --buf->num_lines;

        adjust_markers(lp1, lp2, lp1len, -1, 0);

        thisflag |= FLAG_NEED_RESYNC;
      } else {
        /* Move the text one position backward after the point. */
        buf->pt.p->item = rblist_concat(rblist_sub(buf->pt.p->item, 0, buf->pt.o),
                                        rblist_sub(buf->pt.p->item, buf->pt.o + 1,
                                                   rblist_length(buf->pt.p->item)));
        adjust_markers(buf->pt.p, buf->pt.p, buf->pt.o, 0, -1);
      }
      buf->flags |= BFLAG_MODIFIED;
    }
  }

  /* Insert string. */
  /* FIXME: Inefficient. Could search as for \n's using astr_str. */
  if (bs && rblist_length(bs))
    for (size_t i = 0; i < rblist_length(bs); i++) {
      if (rblist_get(bs, i) == '\n') {
        intercalate_newline();
        CMDCALL(move_next_character);
      } else {
        buf->pt.p->item = rblist_concat(rblist_concat(rblist_sub(buf->pt.p->item, 0, buf->pt.o),
                                                      rblist_sub(bs, i, i + 1)),
                                        rblist_sub(buf->pt.p->item, buf->pt.o, rblist_length(buf->pt.p->item)));
        buf->flags |= BFLAG_MODIFIED;
        adjust_markers(buf->pt.p, buf->pt.p, buf->pt.o, 0, 1);
      }
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
  rblist as;
  ok = replace_nstring(1, &as, NULL);
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

  /* Find previous non-blank line. */
  while (CMDCALL(move_previous_line) && is_blank_line());

  /* Go to `cur_goalc' in that non-blank line. */
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

    /* Now find the next blank char. */
    if (!(preceding_char() == '\t' && get_goalc() > cur_goalc))
      while (!eolp() && !isspace(following_char()))
        CMDCALL(move_next_character);

    /* Find next non-blank char. */
    while (!eolp() && isspace(following_char()))
      CMDCALL(move_next_character);

    /* Record target column. */
    if (!eolp())
      target_goalc = get_goalc();

    buf->pt = old_point->pt;
    remove_marker(old_point);

    /* Indent. */
    undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);
    if (target_goalc > 0)
      /* If not at EOL on target line, insert spaces up to
         target_goalc. */
      while (get_goalc() < target_goalc)
        insert_char(' ');
    else
      /* If already at EOL on target line, insert a tab. */
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
      int indent;
      size_t pos;
      Marker *old_point = point_marker();

      undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);

      /* Check where last non-blank goalc is. */
      previous_nonblank_goalc();
      pos = get_goalc();
      indent = pos > 0 || (!eolp() && isspace(following_char()));
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
