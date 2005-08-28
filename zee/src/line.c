/* Line-oriented editing functions
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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"


/*--------------------------------------------------------------------------
 * Markers
 *--------------------------------------------------------------------------*/

static void unchain_marker(Marker *marker)
{
  Marker *m, *next, *prev = NULL;

  if (!marker->bp)
    return;

  for (m=marker->bp->markers; m; m=next) {
    next = m->next;
    if (m == marker) {
      if (prev)
        prev->next = next;
      else
        m->bp->markers = next;

      m->bp = NULL;
      break;
    }
    prev = m;
  }
}

void free_marker(Marker *marker)
{
  unchain_marker(marker);
  free(marker);
}

void move_marker(Marker *marker, Buffer *bp, Point pt)
{
  if (bp != marker->bp) {
    /* Unchain with the previous pointed buffer. */
    unchain_marker(marker);

    /* Change the buffer. */
    marker->bp = bp;

    /* Chain with the new buffer. */
    marker->next = bp->markers;
    bp->markers = marker;
  }

  /* Change the point. */
  marker->pt = pt;
}

Marker *marker_new(Buffer *bp, Point pt)
{
  Marker *marker = zmalloc(sizeof(Marker));
  move_marker(marker, bp, pt);
  return marker;
}

Marker *point_marker(void)
{
  assert(cur_bp);
  return marker_new(cur_bp, cur_bp->pt);
}

static void adjust_markers(Line *newlp, Line *oldlp, size_t pointo, int dir, int offset)
{
  Marker *pt = point_marker(), *marker;

  assert(cur_bp);
  for (marker = cur_bp->markers; marker != NULL; marker = marker->next)
    if (marker->pt.p == oldlp &&
        (dir == -1 || marker->pt.o >= pointo + dir + (offset < 0))) {
      marker->pt.p = newlp;
      marker->pt.o -= pointo * dir - offset;
      marker->pt.n += dir;
    } else if (marker->pt.n > cur_bp->pt.n)
      marker->pt.n += dir;

  cur_bp->pt = pt->pt;
  free_marker(pt);
}


/*-----------------------------------------------------------------------
  Mark ring
  -----------------------------------------------------------------------*/
static list mark_ring = NULL;	/* Mark ring */

/*
 * Push the current mark on to the the mark-ring
 */
void push_mark(void)
{
  if (!mark_ring)
    mark_ring = list_new();

  /* Save the mark */
  assert(cur_bp);
  assert(cur_bp->mark);
  list_append(mark_ring, marker_new(cur_bp->mark->bp, cur_bp->mark->pt));
}

/*
 * Pop a mark from the mark-ring and make it the current mark
 */
void pop_mark(void)
{
  Marker *m = list_last(mark_ring)->item;

  /* Replace the mark */
  assert(m->bp->mark);
  free_marker(m->bp->mark);

  m->bp->mark = (m->pt.p) ? marker_new(m->bp, m->pt) : NULL;

  list_betail(mark_ring);
  free_marker(m);
}

/*
 * Set the mark to point
 */
void set_mark(void)
{
  assert(cur_bp);
  assert(cur_bp->mark);
  move_marker(cur_bp->mark, cur_bp, cur_bp->pt);
}


/*
 * Create a Line list.
 */
Line *line_new(void)
{
  Line *lp = list_new();
  list_append(lp, astr_new());
  return lp;
}

/*
 * Free a Line list.
 */
void line_delete(Line *lp)
{
  Line *lq;

  /* Free all the lines. */
  for (lq = list_first(lp); lq != lp; lq = list_next(lq))
    astr_delete(lq->item);
  list_delete(lp);
}

/*
 * Read an astr into a Line list.
 */
Line *string_to_lines(astr as, const char *eol, size_t *lines)
{
  const char *p, *end = astr_cstr(as) + astr_len(as);
  Line *lp = line_new();

  for (p = astr_cstr(as), *lines = 1; p < end;) {
    const char *q;
    if ((q = strstr(p, eol)) != NULL) {
      astr_ncat(list_last(lp)->item, p, (size_t)(q - p));
      list_append(lp, astr_new());
      ++*lines;
      p = q + 1;
    } else {                    /* End of string, or embedded NUL */
      size_t len = strlen(p);
      astr_ncat(list_last(lp)->item, p, len);
      p += strlen(p);
      if (p < end) {            /* Deal with embedded NULs */
        astr_cat_char(as, '\0');
        p++;            /* Strictly can only increment p if p < end */
      }
    }
  }

  return lp;
}

/*
 * Return a flag indicating whether the current line is empty
 */
int is_empty_line(void)
{
  assert(cur_bp);
  return astr_len(cur_bp->pt.p->item) == 0;
}

/*
 * Return a flag indicating whether the current line is blank
 */
int is_blank_line(void)
{
  size_t c;
  assert(cur_bp);
  for (c = 0; c < astr_len(cur_bp->pt.p->item); c++)
    if (!isspace(*astr_char(cur_bp->pt.p->item, (ptrdiff_t)c)))
      return FALSE;
  return TRUE;
}

/*
 * Return the character following point in the current buffer
 */
int following_char(void)
{
  assert(cur_bp);
  if (eobp())
    return '\0';
  else if (eolp())
    return '\n';
  else
    return *astr_char(cur_bp->pt.p->item, (ptrdiff_t)cur_bp->pt.o);
}

/*
 * Return the character preceding point in the current buffer
 */
int preceding_char(void)
{
  assert(cur_bp);
  if (bobp())
    return '\0';
  else if (bolp())
    return '\n';
  else
    return *astr_char(cur_bp->pt.p->item, (ptrdiff_t)(cur_bp->pt.o - 1));
}

/*
 * Return flag indicating whether point is at the beginning of the buffer
 */
int bobp(void)
{
  assert(cur_bp);
  return (bolp() && list_prev(cur_bp->pt.p) == cur_bp->lines);
}

/*
 * Return a flag indicating whether point is at the end of the buffer
 */
int eobp(void)
{
  assert(cur_bp);
  return (eolp() && list_next(cur_bp->pt.p) == cur_bp->lines);
}

/*
 * Return a flag indicating whether point is at the beginning of a line
 */
int bolp(void)
{
  assert(cur_bp);
  return cur_bp->pt.o == 0;
}

/*
 * Return a flag indicating whether point is at the end of a line
 */
int eolp(void)
{
  assert(cur_bp);
  return cur_bp->pt.o == astr_len(cur_bp->pt.p->item);
}

/*
 * Insert the character `c' at the current point position
 * into the current buffer.
 */
int insert_char(int c)
{
  astr as = astr_cat_char(astr_new(), (char)c);
  int ret = insert_nstring(as, "\n", FALSE);

  astr_delete(as);
  return ret;
}

static int insert_tab(void)
{
  if (warn_if_readonly_buffer())
    return FALSE;
  else {
    int c = get_goalc();
    int t = tab_width(cur_bp);

    for (c = t - c % t; c > 0; c--)
      insert_char(' ');

    return TRUE;
  }
}

DEFUN_INT("tab-to-tab-stop", tab_to_tab_stop)
/*+
Insert spaces or tabs to next defined tab-stop column.
Convert the tabulation into spaces.
+*/
{
  int i;

  assert(cur_bp);

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
  for (i = 0; i < uniarg; ++i)
    if (!insert_tab()) {
      ok = FALSE;
      break;
    }
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
}
END_DEFUN

/*
 * Insert a newline at the current position without moving the cursor.
 * Update all other cursors if they point on the splitted line.
 */
static int intercalate_newline(void)
{
  Line *lp1, *lp2;
  size_t lp1len, lp2len;
  astr as;

  assert(cur_bp);

  if (warn_if_readonly_buffer())
    return FALSE;

  undo_save(UNDO_REPLACE_BLOCK, cur_bp->pt, 0, 1, FALSE);

  /* Calculate the two line lengths. */
  lp1len = cur_bp->pt.o;
  lp2len = astr_len(cur_bp->pt.p->item) - lp1len;

  lp1 = cur_bp->pt.p;

  /* Update line linked list. */
  list_prepend(lp1, astr_new());
  lp2 = list_next(lp1);
  ++cur_bp->num_lines;

  /* Move the text after the point into the new line. */
  as = astr_substr(lp1->item, (ptrdiff_t)lp1len, lp2len);
  astr_cpy(lp2->item, as);
  astr_delete(as);
  astr_truncate(lp1->item, (ptrdiff_t)lp1len);

  adjust_markers(lp2, lp1, lp1len, 1, 0);

  cur_bp->flags |= BFLAG_MODIFIED;

  thisflag |= FLAG_NEED_RESYNC;

  return TRUE;
}

/*
 * Check the case of a string.
 * Returns 2 if it is all upper case, 1 if just the first letter is,
 * and 0 otherwise.
 */
static int check_case(const char *s, size_t len)
{
  size_t i;

  if (!isupper(*s))
    return 0;

  for (i = 1; i < len; i++)
    if (!isupper(s[i]))
      return 1;

  return 2;
}

/*
 * Recase str according to case of tmpl.
 */
static void recase(char *str, size_t len, const char *tmpl, size_t tmpl_len)
{
  size_t i;
  int tmpl_case = check_case(tmpl, tmpl_len);

  if (tmpl_case >= 1)
    *str = toupper(*str);

  if (tmpl_case == 2)
    for (i = 1; i < len; i++)
      str[i] = toupper(str[i]);
}

/*
 * Replace text in the line "lp" with "newtext". If "replace_case" is
 * TRUE then the new characters will be the same case as the old.
 * Return flag indicating whether modifications have been made.
 */
int line_replace_text(Line **lp, size_t offset, size_t oldlen,
                       const char *newtext, size_t newlen, int replace_case)
{
  int changed = FALSE;
  char *newcopy = zstrdup(newtext);

  if (oldlen > 0) {
    if (replace_case && lookup_bool_variable("case-replace"))
      recase(newcopy, newlen, astr_char((*lp)->item, (ptrdiff_t)offset),
             oldlen);

    if (newlen != oldlen) {
      astr_replace_cstr((*lp)->item, (ptrdiff_t)offset, oldlen, newcopy);
      adjust_markers(*lp, *lp, offset, 0, (int)(newlen - oldlen));
      changed = TRUE;
    } else if (memcmp(astr_char((*lp)->item, (ptrdiff_t)offset),
                      newcopy, newlen) != 0) {
      memcpy(astr_char((*lp)->item, (ptrdiff_t)offset), newcopy, newlen);
      changed = TRUE;
    }
  }

  free(newcopy);

  return changed;
}

/*
 * If point is greater than fill-column, then split the line at the
 * right-most space character at or before fill-column, if there is
 * one, or at the left-most at or after fill-column, if not. If the
 * line contains no spaces, no break is made.
 */
void fill_break_line(void)
{
  size_t i, break_col = 0, excess = 0, old_col;
  size_t fillcol = get_variable_number("fill-column");

  assert(cur_bp);

  /* If we're not beyond fill-column, stop now. */
  if (get_goalc() <= fillcol)
    return;

  /* Move cursor back to fill column */
  old_col = cur_bp->pt.o;
  while (get_goalc() > fillcol + 1) {
    cur_bp->pt.o--;
    excess++;
  }

  /* Find break point moving left from fill-column. */
  for (i = cur_bp->pt.o; i > 0; i--) {
    int c = *astr_char(cur_bp->pt.p->item, (ptrdiff_t)(i - 1));
    if (isspace(c)) {
      break_col = i;
      break;
    }
  }

  /* If no break point moving left from fill-column, find first
     possible moving right. */
  if (break_col == 0) {
    for (i = cur_bp->pt.o + 1; i < astr_len(cur_bp->pt.p->item); i++) {
      int c = *astr_char(cur_bp->pt.p->item, (ptrdiff_t)(i - 1));
      if (isspace(c)) {
        break_col = i;
        break;
      }
    }
  }

  if (break_col >= 1) {
    /* Break line. */
    size_t last_col = cur_bp->pt.o - break_col;
    cur_bp->pt.o = break_col;
    FUNCALL(delete_horizontal_space);
    insert_char('\n');
    cur_bp->pt.o = last_col + excess;
  } else
    /* Undo fiddling with point. */
    cur_bp->pt.o = old_col;
}

DEFUN_INT("newline", newline)
/*+
Insert a newline, and move to left margin of the new line if it's blank.
+*/
{
  int i;

  assert(cur_bp);

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
  for (i = 0; i < uniarg; ++i) {
    if (cur_bp->flags & BFLAG_AUTOFILL &&
        get_goalc() > (size_t)get_variable_number("fill-column"))
      fill_break_line();
    if (!insert_char('\n')) {
      ok = FALSE;
      break;
    }
  }
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
}
END_DEFUN

int insert_nstring(astr as, const char *eolstr, int intercalate)
{
  size_t i, eollen = strlen(eolstr);

  assert(cur_bp);

  if (warn_if_readonly_buffer())
    return FALSE;

  undo_save(UNDO_REPLACE_BLOCK, cur_bp->pt, 0, astr_len(as), FALSE);

  for (i = 0; i < astr_len(as); i++) {
    char *s = astr_char(as, (ptrdiff_t)i);
    if (strncmp(s, eolstr, eollen) == 0) {
      intercalate_newline();
      if (!intercalate)
        edit_navigate_forward_char();
    } else {
      astr_insert_char(cur_bp->pt.p->item, (ptrdiff_t)cur_bp->pt.o, *s);
      cur_bp->flags |= BFLAG_MODIFIED;
      if (!intercalate)
        adjust_markers(cur_bp->pt.p, cur_bp->pt.p, cur_bp->pt.o, 0, 1);
    }
  }

  return TRUE;
}

int delete_nstring(size_t size, astr *as)
{
  weigh_mark();

  if (warn_if_readonly_buffer())
    return FALSE;

  *as = astr_new();

  undo_save(UNDO_REPLACE_BLOCK, cur_bp->pt, size, 0, FALSE);
  cur_bp->flags |= BFLAG_MODIFIED;
  while (size--) {
    if (!eolp())
      astr_cat_char(*as, following_char());
    else
      astr_cat_char(*as, '\n');

    if (eobp()) {
      minibuf_error("End of buffer");
      return FALSE;
    }

    if (eolp()) {
      Line *lp1 = cur_bp->pt.p;
      Line *lp2 = list_next(lp1);
      size_t lp1len = astr_len(lp1->item);

      /* Move the next line of text into the current line. */
      lp2 = list_next(cur_bp->pt.p);
      astr_cat(lp1->item, list_next(cur_bp->pt.p)->item);
      astr_delete(list_behead(lp1));
      --cur_bp->num_lines;

      adjust_markers(lp1, lp2, lp1len, -1, 0);

      thisflag |= FLAG_NEED_RESYNC;
    } else {
      /* Move the text one position backward after the point. */
      astr_remove(cur_bp->pt.p->item, (ptrdiff_t)cur_bp->pt.o, 1);
      adjust_markers(cur_bp->pt.p, cur_bp->pt.p, cur_bp->pt.o, 0, -1);
    }
  }

  return TRUE;
}

int self_insert_command(size_t key)
{
  assert(cur_bp);

  weigh_mark();

  if (key <= 255) {
    if (isspace(key) && cur_bp->flags & BFLAG_AUTOFILL &&
        get_goalc() > (size_t)get_variable_number("fill-column"))
      fill_break_line();
    insert_char((int)key);
    return TRUE;
  } else {
    ding();
    return FALSE;
  }
}

DEFUN_INT("self-insert-command", self_insert_command)
/*+
Insert the character you type.
Whichever character you type to run this command is inserted.
+*/
{
  int i;
  size_t key = getkey();

  assert(cur_bp);

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
  for (i = 0; i < uniarg; ++i)
    if (!self_insert_command(key)) {
      ok = FALSE;
      break;
    }
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
}
END_DEFUN

int delete_char(void)
{
  astr as;
  int ok = delete_nstring(1, &as);
  astr_delete(as);
  return ok;
}

DEFUN_INT("delete-char", delete_char)
/*+
Delete the following character.
Join lines if the character is a newline.
+*/
{
  int i;

  assert(cur_bp);

  if (uniarg < 0)
    return FUNCALL_ARG(backward_delete_char, -uniarg);

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
  for (i = 0; i < uniarg; ++i)
    if (!delete_char()) {
      ok = FALSE;
      break;
    }
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
}
END_DEFUN

int backward_delete_char(void)
{
  weigh_mark();

  if (edit_navigate_backward_char()) {
    delete_char();
    return TRUE;
  } else {
    minibuf_error("Beginning of buffer");
    return FALSE;
  }
}

DEFUN_INT("backward-delete-char", backward_delete_char)
/*+
Delete the previous character.
Join lines if the character is a newline.
+*/
{
  int i;

  assert(cur_bp);

  if (uniarg < 0)
    return FUNCALL_ARG(delete_char, -uniarg);

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
  for (i = 0; i < uniarg; ++i)
    if (!backward_delete_char()) {
      ok = FALSE;
      break;
    }
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
}
END_DEFUN

DEFUN_INT("delete-horizontal-space", delete_horizontal_space)
/*+
Delete all spaces and tabs around point.
+*/
{
  assert(cur_bp);

  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0, FALSE);

  while (!eolp() && isspace(following_char()))
    delete_char();

  while (!bolp() && isspace(preceding_char()))
    backward_delete_char();

  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
}
END_DEFUN

DEFUN_INT("just-one-space", just_one_space)
/*+
Delete all spaces and tabs around point, leaving one space.
+*/
{
  assert(cur_bp);
  undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
  FUNCALL(delete_horizontal_space);
  insert_char(' ');
  undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
}
END_DEFUN

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
  while (FUNCALL_ARG(forward_line, -1) && is_blank_line());

  /* Go to `cur_goalc' in that non-blank line. */
  while (!eolp() && get_goalc() < cur_goalc)
    edit_navigate_forward_char();
}

DEFUN_INT("indent-relative", indent_relative)
/*+
Indent line or insert a tab.
+*/
{
  assert(cur_bp);

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
        edit_navigate_forward_char();

    /* Find next non-blank char. */
    while (!eolp() && isspace(following_char()))
      edit_navigate_forward_char();

    /* Record target column. */
    if (!eolp())
      target_goalc = get_goalc();

    cur_bp->pt = old_point->pt;
    free_marker(old_point);

    /* Indent. */
    undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
    if (target_goalc > 0)
      /* If not at EOL on target line, insert spaces up to
         target_goalc. */
      while (get_goalc() < target_goalc)
        /* If already at EOL on target line, insert a tab. */
        insert_char(' ');
    else
      ok = insert_tab();
    undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
  }
}
END_DEFUN

DEFUN_INT("newline-and-indent", newline_and_indent)
/*+
Insert a newline, then indent.
Indentation is done using the `indent-relative' command, except
that if there is a character in the first column of the line above,
no indenting is performed.
+*/
{
  assert(cur_bp);

  if (warn_if_readonly_buffer())
    ok = FALSE;
  else {
    int indent;

    weigh_mark();

    if ((ok = insert_char('\n'))) {
      size_t pos;
      Marker *old_point = point_marker();

      undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0, FALSE);

      /* Check where last non-blank goalc is. */
      previous_nonblank_goalc();
      pos = get_goalc();
      indent = pos > 0 || (!eolp() && isspace(following_char()));
      cur_bp->pt = old_point->pt;
      free_marker(old_point);
      /* Only indent if we're in column > 0 or we're in column 0 and
         there is a space character there in the last non-blank line. */
      if (indent)
        FUNCALL(indent_relative);

      undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0, FALSE);
    }
  }
}
END_DEFUN
