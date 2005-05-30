/* Display engine
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2005 Reuben Thomas.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "config.h"
#include "extern.h"

static int initted = FALSE;
static size_t width = 0, height = 0;
static size_t cur_tab_width;
static size_t cur_topline;
static size_t point_start_column;
static size_t point_screen_column;

int term_initted(void)
{
  return initted;
}

void term_set_initted(void)
{
  initted = TRUE;
}

size_t term_width(void)
{
  return width;
}

void term_set_width(size_t n)
{
  width = n;
}

size_t term_height(void)
{
  return height;
}

void term_set_height(size_t n)
{
  height = n;
}

static int make_char_printable(char **buf, size_t c)
{
  if (c == '\0')
    return zasprintf(buf, "^@");
  else if (c <= '\32')
    return zasprintf(buf, "^%c", 'A' + c - 1);
  else
    return zasprintf(buf, "\\%o", c & 0xff);
}

/* Prints a printable representation of the character 'c' on the screen in the
 * specified font, and updates the cursor position 'x'. On exit, the font is set
 * to FONT_NORMAL.
 * Printing is suppressed if 'x' reaches SCREEN_COLS; in this case 'x' is set to
 * SCREEN_COLS.
 * This function is implemented in terms of 'term_addch()'.
 */
static void outch(int c, Font font, size_t *x)
{
  int j, w;
  char *buf;

  if (*x >= screen_cols)
    return;

  term_attrset(1, font);

  if (c == '\t')
    for (w = cur_tab_width - *x % cur_tab_width; w > 0 && *x < screen_cols; w--)
      term_addch(' '), ++(*x);
  else if (isprint(c))
    term_addch(c), ++(*x);
  else {
    j = make_char_printable(&buf, (size_t)c);
    for (w = 0; w < j && *x < screen_cols; ++w)
      term_addch(buf[w]), ++(*x);
    free(buf);
  }

  term_attrset(1, FONT_NORMAL);
}

/** Tests whether the specified line and offset is within the specified Region.
 * The offset is measured in characters, not in character positions.
 * (Question: this code is horrible. Why not just use cmp_point()?)
 */
static int in_region(size_t lineno, size_t x, Region *r)
{
  if (lineno >= r->start.n && lineno <= r->end.n) {
    if (r->start.n == r->end.n) {
      if (x >= r->start.o && x < r->end.o)
        return TRUE;
    } else if (lineno == r->start.n) {
      if (x >= r->start.o)
        return TRUE;
    } else if (lineno == r->end.n) {
      if (x < r->end.o)
        return TRUE;
    } else
      return TRUE;
  }

  return FALSE;
}

/* Prints a line on the terminal.
 *  - Guess: 'line' is the line number measured on the terminal.
 *    (Question: why not within window?)
 *  - 'start_col' is the horizontal scroll offset: the character position (not
 *    cursor position) within 'lp' of the first character that should be
 *    displayed.
 *  - 'wp' is the window in which to display. Only used to find the display
 *    width.
 *  - 'lp' is the line to display.
 *  - 'lineno' is the line number of 'lp' within the buffer.
 *  - 'r' is a region to highlight: the current selection.
 */
static void draw_line(size_t line, size_t startcol, Window *wp, Line *lp,
		      size_t lineno, Region *r)
{
  size_t x, i;

  term_move(line, 0);
  for (x = 0, i = startcol; i < astr_len(lp->item) && x < wp->ewidth; i++) {
    if (in_region(lineno, i, r))
      outch(*astr_char(lp->item, (ptrdiff_t)i), FONT_REVERSE, &x);
    else
      outch(*astr_char(lp->item, (ptrdiff_t)i), FONT_NORMAL, &x);
  }

  if (x >= screen_cols) {
    /* (Question: why do we use screen_cols instead of wp->ewidth?) */
    term_move(line, screen_cols - 1);
    term_addch('$');
  } else {
    for (; x < wp->ewidth; ++i) {
      if (in_region(lineno, i, r))
        outch(' ', FONT_REVERSE, &x);
      else
        x++;
    }
  }
}

/* Sets 'r->start' to the lesser of the point and mark of the specified window,
 * and sets 'r->end' to the greater. If the mark is not anchored, it is treated
 * as if it were at the point.
 */
static void calculate_highlight_region(Window *wp, Region *r)
{
  assert(wp->bp->mark);
  r->start = window_pt(wp);
  if (wp->bp->mark_anchored) {
    r->end = wp->bp->mark->pt;
    if (cmp_point(r->end, r->start) < 0)
      swap_point(&r->end, &r->start);
  } else
    r->end = window_pt(wp);
}

static void draw_window(size_t topline, Window *wp)
{
  size_t i, startcol, lineno;
  Line *lp;
  Region r;
  Point pt = window_pt(wp);

  calculate_highlight_region(wp, &r);

  /* Find the first line to display on the first screen line. */
  for (lp = pt.p, lineno = pt.n, i = wp->topdelta;
       i > 0 && list_prev(lp) != wp->bp->lines; lp = list_prev(lp), --i, --lineno);

  cur_tab_width = tab_width(wp->bp);

  /* Draw the window lines. */
  for (i = topline; i < wp->eheight + topline; ++i, ++lineno) {
    /* Clear the line. */
    term_move(i, 0);
    term_clrtoeol();

    /* If at the end of the buffer, don't write any text. */
    if (lp == wp->bp->lines)
      continue;

    startcol = point_start_column;

    draw_line(i, startcol, wp, lp, lineno, &r);

    if (point_start_column > 0) {
      term_move(i, 0);
      term_addch('$');
    }

    lp = list_next(lp);
  }
}

static char *make_mode_line_flags(Window *wp)
{
  static char buf[3];

  if ((wp->bp->flags & (BFLAG_MODIFIED | BFLAG_READONLY)) == (BFLAG_MODIFIED | BFLAG_READONLY))
    buf[0] = '%', buf[1] = '*';
  else if (wp->bp->flags & BFLAG_MODIFIED)
    buf[0] = buf[1] = '*';
  else if (wp->bp->flags & BFLAG_READONLY)
    buf[0] = buf[1] = '%';
  else
    buf[0] = buf[1] = '-';

  return buf;
}

/*
 * This function calculates the best start column to draw if the line
 * needs to get truncated.
 * Called only for the line where the point is.
 */
static void calculate_start_column(Window *wp)
{
  size_t col = 0, lastcol = 0, t = tab_width(wp->bp);
  int rpfact, lpfact;
  char *buf, *rp, *lp, *p;
  Point pt = window_pt(wp);

  rp = astr_char(pt.p->item, (ptrdiff_t)pt.o);
  rpfact = pt.o / (wp->ewidth / 3);

  for (lp = rp; lp >= astr_cstr(pt.p->item); --lp) {
    for (col = 0, p = lp; p < rp; ++p)
      if (*p == '\t') {
        col |= t - 1;
        ++col;
      } else if (isprint(*p))
        ++col;
      else {
        col += make_char_printable(&buf, (size_t)*p);
        free(buf);
      }

    lpfact = (lp - astr_cstr(pt.p->item)) / (wp->ewidth / 3);

    if (col >= wp->ewidth - 1 || lpfact < (rpfact - 2)) {
      point_start_column = lp + 1 - astr_cstr(pt.p->item);
      point_screen_column = lastcol;
      return;
    }

    lastcol = col;
  }

  point_start_column = 0;
  point_screen_column = col;
}

static char *make_screen_pos(Window *wp, char **buf)
{
  Point pt = window_pt(wp);

  if (wp->bp->num_lines <= wp->eheight && wp->topdelta == pt.n)
    zasprintf(buf, "All");
  else if (pt.n == wp->topdelta)
    zasprintf(buf, "Top");
  else if (pt.n + (wp->eheight - wp->topdelta) > wp->bp->num_lines)
    zasprintf(buf, "Bot");
  else
    zasprintf(buf, "%2d%%", (int)((float)pt.n / wp->bp->num_lines * 100));

  return *buf;
}

static void draw_status_line(size_t line, Window *wp)
{
  size_t i;
  int someflag = 0;
  char *buf;
  Point pt = window_pt(wp);

  term_attrset(1, FONT_REVERSE);

  term_move(line, 0);
  for (i = 0; i < wp->ewidth; ++i)
    term_addch('-');

  term_move(line, 0);
  term_printw("--%2s- %-18s (", make_mode_line_flags(wp), wp->bp->name);

  if (wp->bp->flags & BFLAG_AUTOFILL) {
    term_printw("Fill");
    someflag = 1;
  }
  if (thisflag & FLAG_DEFINING_MACRO) {
    term_printw("%sDef", someflag ? " " : "");
    someflag = 1;
  }
  if (wp->bp->flags & BFLAG_ISEARCH)
    term_printw("%sIsearch", someflag ? " " : "");

  term_printw(")--L%d--C%d--%s",
              pt.n+1, get_goalc_wp(wp),
              make_screen_pos(wp, &buf));
  free(buf);

  term_attrset(1, FONT_NORMAL);
}

/* Draws all the windows in turn, and draws the status line if any space
 * remains. Finally, moves the cursor to the correct position.
 */
void term_display(void)
{
  size_t topline;
  Window *wp;

  cur_topline = topline = 0;

  calculate_start_column(cur_wp);

  for (wp = head_wp; wp != NULL; wp = wp->next) {
    if (wp == cur_wp)
      cur_topline = topline;

    draw_window(topline, wp);

    /* Draw the status line only if there is available space after the
       buffer text space. */
    if (wp->fheight - wp->eheight > 0)
      draw_status_line(topline + wp->eheight, wp);

    topline += wp->fheight;
  }

  term_redraw_cursor();
}

void term_redraw_cursor(void)
{
  term_move(cur_topline + cur_wp->topdelta, point_screen_column);
}

void term_full_redisplay(void)
{
  term_clear();
  term_display();
}

void show_splash_screen(const char *splash)
{
  size_t i;
  const char *p;

  for (i = 0; i < screen_rows - 2; ++i) {
    term_move(i, 0);
    term_clrtoeol();
  }

  term_move(0, 0);
  for (i = 0, p = splash; *p != '\0' && i < term_height() - 2; ++p)
    if (*p == '\n')
      term_move(++i, 0);
    else
      term_addch(*p);
}

/*
 * Tidy up the term ready to suspend or quit.
 */
void term_tidy(void)
{
  term_move(screen_rows - 1, 0);
  term_clrtoeol();
  term_attrset(1, FONT_NORMAL);
  term_refresh();
}

/*
 * Add a string to the terminal
 */
void term_addnstr(const char *s, size_t len)
{
  size_t i;
  for (i = 0; i < len; i++)
    term_addch(*s++);
}

/*
 * printf on the terminal
 */
int term_printw(const char *fmt, ...)
{
  char *buf;
  int res = 0;
  va_list ap;
  va_start(ap, fmt);
  res = zvasprintf(&buf, fmt, ap);
  va_end(ap);
  term_addnstr(buf, strlen(buf));
  free(buf);
  return res;
}
