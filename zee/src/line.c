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
#include <string.h>

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
  list_append(lp, astr_new(""));
  return lp;
}

/*
 * Read an astr into a Line list.
 */
Line *string_to_lines(astr as, size_t *lines)
{
  ptrdiff_t p, q, end = (ptrdiff_t)astr_len(as);
  Line *lp = line_new();

  for (p = 0, *lines = 1;
       p < end && (q = (astr_str(as, p, astr_new("\n")))) >= 0;
       p = q + 1) {
    astr_cat(list_last(lp)->item, astr_sub(as, p, q));
    list_append(lp, astr_new(""));
    ++*lines;
  }

  /* Add the rest of the string, if any */
  astr_cat(list_last(lp)->item, astr_sub(as, p, end));

  return lp;
}

/*
 * Return a flag indicating whether the current line is empty
 */
int is_empty_line(void)
{
  return astr_len(buf->pt.p->item) == 0;
}

/*
 * Return a flag indicating whether the current line is blank
 */
int is_blank_line(void)
{
  size_t c;

  for (c = 0; c < astr_len(buf->pt.p->item); c++)
    if (!isspace(*astr_char(buf->pt.p->item, (ptrdiff_t)c)))
      return FALSE;
  return TRUE;
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
    return *astr_char(buf->pt.p->item, (ptrdiff_t)buf->pt.o);
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
    return *astr_char(buf->pt.p->item, (ptrdiff_t)(buf->pt.o - 1));
}

/*
 * Return flag indicating whether point is at the beginning of the buffer
 */
int bobp(void)
{
  return (bolp() && list_prev(buf->pt.p) == buf->lines);
}

/*
 * Return a flag indicating whether point is at the end of the buffer
 */
int eobp(void)
{
  return (eolp() && list_next(buf->pt.p) == buf->lines);
}

/*
 * Return a flag indicating whether point is at the beginning of a line
 */
int bolp(void)
{
  return buf->pt.o == 0;
}

/*
 * Return a flag indicating whether point is at the end of a line
 */
int eolp(void)
{
  return buf->pt.o == astr_len(buf->pt.p->item);
}

/*
 * Insert the character `c' at the current point position
 * into the current buffer.
 */
int insert_char(int c)
{
  astr as = astr_afmt("%c", c);
  return insert_nstring(as);
}

DEF(tab_to_tab_stop,
"\
Insert spaces or tabs to next defined tab-stop column.\n\
Convert the tabulation into spaces.\
")
{
  if (warn_if_readonly_buffer())
    ok = FALSE;
  else {
    int c = get_goalc();
    int t = tab_width();

    for (c = t - c % t; c > 0; c--)
      insert_char(' ');
  }
}
END_DEF

/*
 * Insert a newline at the current position without moving the cursor.
 * Update markers that point to the splitted line.
 */
static int intercalate_newline(void)
{
  Line *new_lp;

  if (warn_if_readonly_buffer())
    return FALSE;

  undo_save(UNDO_REPLACE_BLOCK, buf->pt, 0, 1);

  /* Update line linked list. */
  list_prepend(buf->pt.p, astr_new(""));
  new_lp = list_first(buf->pt.p);
  ++buf->num_lines;

  /* Move the text after the point into the new line. */
  new_lp->item = astr_sub(buf->pt.p->item, (ptrdiff_t)buf->pt.o, (ptrdiff_t)astr_len(buf->pt.p->item));
  buf->pt.p->item = astr_sub(buf->pt.p->item, 0, (ptrdiff_t)buf->pt.o);

  adjust_markers(new_lp, buf->pt.p, buf->pt.o, 1, 0);

  buf->flags |= BFLAG_MODIFIED;
  thisflag |= FLAG_NEED_RESYNC;

  return TRUE;
}

/*
 * Check the case of a string.
 * Returns 2 if it is all upper case, 1 if just the first letter is,
 * and 0 otherwise.
 */
static int check_case(astr as)
{
  size_t i;

  if (!isupper(*astr_char(as, 0)))
    return 0;

  for (i = 1; i < astr_len(as); i++)
    if (!isupper(*astr_char(as, (ptrdiff_t)i)))
      return 1;

  return 2;
}

/*
 * Recase str according to case of tmpl.
 */
static void recase(astr str, astr tmpl)
{
  size_t i;
  int tmpl_case = check_case(tmpl);

  if (tmpl_case >= 1)
    *astr_char(str, 0) = toupper(*astr_char(str, 0));

  if (tmpl_case == 2)
    for (i = 1; i < astr_len(str); i++)
      *astr_char(str, (ptrdiff_t)i) = toupper(*astr_char(str, (ptrdiff_t)i));
}

/*
 * Replace text in the line lp with newtext. If replace_case is
 * TRUE then the new characters will be the same case as the old.
 * Return flag indicating whether modifications have been made.
 */
int line_replace_text(Line **lp, size_t offset, size_t oldlen,
                      astr newtext, int replace_case)
{
  int changed = FALSE;

  if (oldlen > 0) {
    if (replace_case && get_variable_bool(astr_new("case_replace")))
      recase(newtext, astr_sub((*lp)->item, (ptrdiff_t)offset, (ptrdiff_t)(offset + oldlen)));

    if (astr_len(newtext) != oldlen) {
      (*lp)->item = astr_cat(astr_cat(astr_sub((*lp)->item, 0, (ptrdiff_t)offset), newtext),
                             astr_sub((*lp)->item, (ptrdiff_t)(offset + oldlen), (ptrdiff_t)astr_len((*lp)->item)));
      adjust_markers(*lp, *lp, offset, 0, (int)(astr_len(newtext) - oldlen));
      changed = TRUE;
    } else if (memcmp(astr_char((*lp)->item, (ptrdiff_t)offset),
                      astr_cstr(newtext), astr_len(newtext)) != 0) {
      memcpy(astr_char((*lp)->item, (ptrdiff_t)offset),
             astr_cstr(newtext), astr_len(newtext));
      changed = TRUE;
    }
  }

  return changed;
}

/*
 * If point is greater than fill_column, then split the line at the
 * right-most space character at or before fill_column, if there is
 * one, or at the left-most at or after fill_column, if not. If the
 * line contains no spaces, no break is made.
 */
void fill_break_line(void)
{
  size_t i, break_col = 0, excess = 0, old_col;
  size_t fillcol = get_variable_number(astr_new("fill_column"));

  /* If we're not beyond fill_column, stop now. */
  if (get_goalc() <= fillcol)
    return;

  /* Move cursor back to fill column */
  old_col = buf->pt.o;
  while (get_goalc() > fillcol + 1) {
    buf->pt.o--;
    excess++;
  }

  /* Find break point moving left from fill_column. */
  for (i = buf->pt.o; i > 0; i--) {
    int c = *astr_char(buf->pt.p->item, (ptrdiff_t)(i - 1));
    if (isspace(c)) {
      break_col = i;
      break;
    }
  }

  /* If no break point moving left from fill_column, find first
     possible moving right. */
  if (break_col == 0) {
    for (i = buf->pt.o + 1; i < astr_len(buf->pt.p->item); i++) {
      int c = *astr_char(buf->pt.p->item, (ptrdiff_t)(i - 1));
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

DEF(newline,
"\
Insert a newline, and move to left margin of the new line if it's blank.\
")
{
  undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);
  if (buf->flags & BFLAG_AUTOFILL &&
      get_goalc() > (size_t)get_variable_number(astr_new("fill_column")))
    fill_break_line();
  ok = insert_char('\n');
  undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);
}
END_DEF

int insert_nstring(astr as)
{
  size_t i;

  if (warn_if_readonly_buffer())
    return FALSE;

  undo_save(UNDO_REPLACE_BLOCK, buf->pt, 0, astr_len(as));

  for (i = 0; i < astr_len(as); i++) {
    if (!astr_cmp(astr_sub(as, (ptrdiff_t)i, (ptrdiff_t)astr_len(as)), astr_new("\n"))) {
      intercalate_newline();
      CMDCALL(edit_navigate_forward_char);
    } else {
      buf->pt.p->item = astr_cat(astr_cat(astr_sub(buf->pt.p->item, 0, (ptrdiff_t)buf->pt.o),
                                         astr_sub(as, (ptrdiff_t)i, (ptrdiff_t)i + 1)),
                                astr_sub(buf->pt.p->item, (ptrdiff_t)buf->pt.o, (ptrdiff_t)astr_len(buf->pt.p->item)));
      buf->flags |= BFLAG_MODIFIED;
      adjust_markers(buf->pt.p, buf->pt.p, buf->pt.o, 0, 1);
    }
  }

  return TRUE;
}

/*
 * Delete a string of the given length from point, returning it
 */
int delete_nstring(size_t size, astr *as)
{
  weigh_mark();

  if (warn_if_readonly_buffer())
    return FALSE;

  *as = astr_new("");

  undo_save(UNDO_REPLACE_BLOCK, buf->pt, size, 0);
  buf->flags |= BFLAG_MODIFIED;
  while (size--) {
    if (!eolp())
      astr_cat_char(*as, following_char());
    else
      astr_cat(*as, astr_new("\n"));

    if (eobp()) {
      minibuf_error(astr_new("End of buffer"));
      return FALSE;
    }

    if (eolp()) {
      Line *lp1 = buf->pt.p;
      Line *lp2 = list_next(lp1);
      size_t lp1len = astr_len(lp1->item);

      /* Move the next line of text into the current line. */
      lp2 = list_next(buf->pt.p);
      astr_cat(lp1->item, list_next(buf->pt.p)->item);
      list_behead(lp1);
      --buf->num_lines;

      adjust_markers(lp1, lp2, lp1len, -1, 0);

      thisflag |= FLAG_NEED_RESYNC;
    } else {
      /* Move the text one position backward after the point. */
      buf->pt.p->item = astr_cat(astr_sub(buf->pt.p->item, 0, (ptrdiff_t)buf->pt.o),
                                astr_sub(buf->pt.p->item, (ptrdiff_t)buf->pt.o + 1,
                                         (ptrdiff_t)astr_len(buf->pt.p->item)));
      adjust_markers(buf->pt.p, buf->pt.p, buf->pt.o, 0, -1);
    }
  }

  return TRUE;
}

DEF_ARG(self_insert_command,
"\
Insert the character you type.\n\
Whichever character you type to run this command is inserted.\
",
INT(c))
{
  weigh_mark();

  if (c <= 255) {
    if (isspace(c) && buf->flags & BFLAG_AUTOFILL &&
        get_goalc() > (size_t)get_variable_number(astr_new("fill_column")))
      fill_break_line();
    insert_char(c);
  } else {
    ding();
    ok = FALSE;
  }
}
END_DEF

DEF(delete_char,
"\
Delete the following character.\n\
Join lines if the character is a newline.\
")
{
  astr as;
  ok = delete_nstring(1, &as);
}
END_DEF

DEF(backward_delete_char,
"\
Delete the previous character.\n\
Join lines if the character is a newline.\
")
{
  weigh_mark();

  if (CMDCALL(edit_navigate_backward_char))
    CMDCALL(delete_char);
  else {
    minibuf_error(astr_new("Beginning of buffer"));
    ok = FALSE;
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
    CMDCALL(delete_char);

  while (!bolp() && isspace(preceding_char()))
    CMDCALL(backward_delete_char);

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
  while (CMDCALL(edit_navigate_up_line) && is_blank_line());

  /* Go to `cur_goalc' in that non-blank line. */
  while (!eolp() && get_goalc() < cur_goalc)
    CMDCALL(edit_navigate_forward_char);
}

DEF(indent_relative,
"\
Indent line or insert a tab.\
")
{
  if (warn_if_readonly_buffer())
    ok = FALSE;
  else {
    size_t cur_goalc = get_goalc(), target_goalc = 0;
    Marker *old_point = point_marker();

    weigh_mark();
    previous_nonblank_goalc();

    /* Now find the next blank char. */
    if (!(preceding_char() == '\t' && get_goalc() > cur_goalc))
      while (!eolp() && !isspace(following_char()))
        CMDCALL(edit_navigate_forward_char);

    /* Find next non-blank char. */
    while (!eolp() && isspace(following_char()))
      CMDCALL(edit_navigate_forward_char);

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
        /* If already at EOL on target line, insert a tab. */
        insert_char(' ');
    else
      ok = CMDCALL(tab_to_tab_stop);
    undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);
  }
}
END_DEF

DEF(newline_and_indent,
"\
Insert a newline, then indent.\n\
Indentation is done using the `indent_relative' command, except\n\
that if there is a character in the first column of the line above,\n\
no indenting is performed.\
")
{
  if (warn_if_readonly_buffer())
    ok = FALSE;
  else {
    weigh_mark();

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
