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
  return marker_new(&buf, buf.pt);
}

static void adjust_markers(Line *newlp, Line *oldlp, size_t pointo, int dir, int offset)
{
  Marker *pt = point_marker(), *marker;

  for (marker = buf.markers; marker != NULL; marker = marker->next)
    if (marker->pt.p == oldlp &&
        (dir == -1 || marker->pt.o >= pointo + dir + (offset < 0))) {
      marker->pt.p = newlp;
      marker->pt.o -= pointo * dir - offset;
      marker->pt.n += dir;
    } else if (marker->pt.n > buf.pt.n)
      marker->pt.n += dir;

  buf.pt = pt->pt;
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
  assert(buf.mark);
  list_append(mark_ring, marker_new(buf.mark->bp, buf.mark->pt));
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
  assert(buf.mark);
  move_marker(buf.mark, &buf, buf.pt);
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
  return astr_len(buf.pt.p->item) == 0;
}

/*
 * Return a flag indicating whether the current line is blank
 */
int is_blank_line(void)
{
  size_t c;

  for (c = 0; c < astr_len(buf.pt.p->item); c++)
    if (!isspace(*astr_char(buf.pt.p->item, (ptrdiff_t)c)))
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
    return *astr_char(buf.pt.p->item, (ptrdiff_t)buf.pt.o);
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
    return *astr_char(buf.pt.p->item, (ptrdiff_t)(buf.pt.o - 1));
}

/*
 * Return flag indicating whether point is at the beginning of the buffer
 */
int bobp(void)
{
  return (bolp() && list_prev(buf.pt.p) == buf.lines);
}

/*
 * Return a flag indicating whether point is at the end of the buffer
 */
int eobp(void)
{
  return (eolp() && list_next(buf.pt.p) == buf.lines);
}

/*
 * Return a flag indicating whether point is at the beginning of a line
 */
int bolp(void)
{
  return buf.pt.o == 0;
}

/*
 * Return a flag indicating whether point is at the end of a line
 */
int eolp(void)
{
  return buf.pt.o == astr_len(buf.pt.p->item);
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
    int t = tab_width();

    for (c = t - c % t; c > 0; c--)
      insert_char(' ');

    return TRUE;
  }
}

DEFUN("tab-to-tab-stop", tab_to_tab_stop)
/*+
Insert spaces or tabs to next defined tab-stop column.
Convert the tabulation into spaces.
+*/
{
  ok = insert_tab();
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

  if (warn_if_readonly_buffer())
    return FALSE;

  undo_save(UNDO_REPLACE_BLOCK, buf.pt, 0, 1, FALSE);

  /* Calculate the two line lengths. */
  lp1len = buf.pt.o;
  lp2len = astr_len(buf.pt.p->item) - lp1len;

  lp1 = buf.pt.p;

  /* Update line linked list. */
  list_prepend(lp1, astr_new());
  lp2 = list_next(lp1);
  ++buf.num_lines;

  /* Move the text after the point into the new line. */
  as = astr_substr(lp1->item, (ptrdiff_t)lp1len, lp2len);
  astr_cpy_delete(lp2->item, as);
  astr_truncate(lp1->item, (ptrdiff_t)lp1len);

  adjust_markers(lp2, lp1, lp1len, 1, 0);

  buf.flags |= BFLAG_MODIFIED;

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
    if (replace_case && get_variable_bool("case-replace"))
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

  /* If we're not beyond fill-column, stop now. */
  if (get_goalc() <= fillcol)
    return;

  /* Move cursor back to fill column */
  old_col = buf.pt.o;
  while (get_goalc() > fillcol + 1) {
    buf.pt.o--;
    excess++;
  }

  /* Find break point moving left from fill-column. */
  for (i = buf.pt.o; i > 0; i--) {
    int c = *astr_char(buf.pt.p->item, (ptrdiff_t)(i - 1));
    if (isspace(c)) {
      break_col = i;
      break;
    }
  }

  /* If no break point moving left from fill-column, find first
     possible moving right. */
  if (break_col == 0) {
    for (i = buf.pt.o + 1; i < astr_len(buf.pt.p->item); i++) {
      int c = *astr_char(buf.pt.p->item, (ptrdiff_t)(i - 1));
      if (isspace(c)) {
        break_col = i;
        break;
      }
    }
  }

  if (break_col >= 1) {
    /* Break line. */
    size_t last_col = buf.pt.o - break_col;
    buf.pt.o = break_col;
    FUNCALL(delete_horizontal_space);
    insert_char('\n');
    buf.pt.o = last_col + excess;
  } else
    /* Undo fiddling with point. */
    buf.pt.o = old_col;
}

DEFUN("newline", newline)
/*+
Insert a newline, and move to left margin of the new line if it's blank.
+*/
{
  undo_save(UNDO_START_SEQUENCE, buf.pt, 0, 0, FALSE);
  if (buf.flags & BFLAG_AUTOFILL &&
      get_goalc() > (size_t)get_variable_number("fill-column"))
    fill_break_line();
  ok = insert_char('\n');
  undo_save(UNDO_END_SEQUENCE, buf.pt, 0, 0, FALSE);
}
END_DEFUN

int insert_nstring(astr as, const char *eolstr, int intercalate)
{
  size_t i, eollen = strlen(eolstr);

  if (warn_if_readonly_buffer())
    return FALSE;

  undo_save(UNDO_REPLACE_BLOCK, buf.pt, 0, astr_len(as), FALSE);

  for (i = 0; i < astr_len(as); i++) {
    char *s = astr_char(as, (ptrdiff_t)i);
    if (strncmp(s, eolstr, eollen) == 0) {
      intercalate_newline();
      if (!intercalate)
        edit_navigate_forward_char();
    } else {
      astr_insert_char(buf.pt.p->item, (ptrdiff_t)buf.pt.o, *s);
      buf.flags |= BFLAG_MODIFIED;
      if (!intercalate)
        adjust_markers(buf.pt.p, buf.pt.p, buf.pt.o, 0, 1);
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

  undo_save(UNDO_REPLACE_BLOCK, buf.pt, size, 0, FALSE);
  buf.flags |= BFLAG_MODIFIED;
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
      Line *lp1 = buf.pt.p;
      Line *lp2 = list_next(lp1);
      size_t lp1len = astr_len(lp1->item);

      /* Move the next line of text into the current line. */
      lp2 = list_next(buf.pt.p);
      astr_cat(lp1->item, list_next(buf.pt.p)->item);
      astr_delete(list_behead(lp1));
      --buf.num_lines;

      adjust_markers(lp1, lp2, lp1len, -1, 0);

      thisflag |= FLAG_NEED_RESYNC;
    } else {
      /* Move the text one position backward after the point. */
      astr_remove(buf.pt.p->item, (ptrdiff_t)buf.pt.o, 1);
      adjust_markers(buf.pt.p, buf.pt.p, buf.pt.o, 0, -1);
    }
  }

  return TRUE;
}

int self_insert_command(size_t key)
{
  weigh_mark();

  if (key <= 255) {
    if (isspace(key) && buf.flags & BFLAG_AUTOFILL &&
        get_goalc() > (size_t)get_variable_number("fill-column"))
      fill_break_line();
    insert_char((int)key);
    return TRUE;
  } else {
    ding();
    return FALSE;
  }
}

DEFUN("self-insert-command", self_insert_command)
/*+
Insert the character you type.
Whichever character you type to run this command is inserted.
+*/
{
  ok = self_insert_command(getkey());
}
END_DEFUN

int delete_char(void)
{
  astr as;
  int ok = delete_nstring(1, &as);
  astr_delete(as);
  return ok;
}

DEFUN("delete-char", delete_char)
/*+
Delete the following character.
Join lines if the character is a newline.
+*/
{
  ok = delete_char();
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

DEFUN("backward-delete-char", backward_delete_char)
/*+
Delete the previous character.
Join lines if the character is a newline.
+*/
{
  ok = backward_delete_char();
}
END_DEFUN

DEFUN("delete-horizontal-space", delete_horizontal_space)
/*+
Delete all spaces and tabs around point.
+*/
{
  undo_save(UNDO_START_SEQUENCE, buf.pt, 0, 0, FALSE);

  while (!eolp() && isspace(following_char()))
    delete_char();

  while (!bolp() && isspace(preceding_char()))
    backward_delete_char();

  undo_save(UNDO_END_SEQUENCE, buf.pt, 0, 0, FALSE);
}
END_DEFUN

DEFUN("just-one-space", just_one_space)
/*+
Delete all spaces and tabs around point, leaving one space.
+*/
{
  undo_save(UNDO_START_SEQUENCE, buf.pt, 0, 0, FALSE);
  FUNCALL(delete_horizontal_space);
  insert_char(' ');
  undo_save(UNDO_END_SEQUENCE, buf.pt, 0, 0, FALSE);
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
  while (edit_navigate_up_line() && is_blank_line());

  /* Go to `cur_goalc' in that non-blank line. */
  while (!eolp() && get_goalc() < cur_goalc)
    edit_navigate_forward_char();
}

DEFUN("indent-relative", indent_relative)
/*+
Indent line or insert a tab.
+*/
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
        edit_navigate_forward_char();

    /* Find next non-blank char. */
    while (!eolp() && isspace(following_char()))
      edit_navigate_forward_char();

    /* Record target column. */
    if (!eolp())
      target_goalc = get_goalc();

    buf.pt = old_point->pt;
    free_marker(old_point);

    /* Indent. */
    undo_save(UNDO_START_SEQUENCE, buf.pt, 0, 0, FALSE);
    if (target_goalc > 0)
      /* If not at EOL on target line, insert spaces up to
         target_goalc. */
      while (get_goalc() < target_goalc)
        /* If already at EOL on target line, insert a tab. */
        insert_char(' ');
    else
      ok = insert_tab();
    undo_save(UNDO_END_SEQUENCE, buf.pt, 0, 0, FALSE);
  }
}
END_DEFUN

DEFUN("newline-and-indent", newline_and_indent)
/*+
Insert a newline, then indent.
Indentation is done using the `indent-relative' command, except
that if there is a character in the first column of the line above,
no indenting is performed.
+*/
{
  if (warn_if_readonly_buffer())
    ok = FALSE;
  else {
    int indent;

    weigh_mark();

    if ((ok = insert_char('\n'))) {
      size_t pos;
      Marker *old_point = point_marker();

      undo_save(UNDO_START_SEQUENCE, buf.pt, 0, 0, FALSE);

      /* Check where last non-blank goalc is. */
      previous_nonblank_goalc();
      pos = get_goalc();
      indent = pos > 0 || (!eolp() && isspace(following_char()));
      buf.pt = old_point->pt;
      free_marker(old_point);
      /* Only indent if we're in column > 0 or we're in column 0 and
         there is a space character there in the last non-blank line. */
      if (indent)
        FUNCALL(indent_relative);

      undo_save(UNDO_END_SEQUENCE, buf.pt, 0, 0, FALSE);
    }
  }
}
END_DEFUN
