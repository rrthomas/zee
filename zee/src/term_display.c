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
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "config.h"
#include "extern.h"

static size_t width = 0, height = 0;
static size_t cur_tab_width;
static size_t cur_topline;
static size_t point_start_column;
static size_t point_screen_column;

size_t term_width(void)
{
  return width;
}

size_t term_height(void)
{
  return height;
}

void term_set_size(size_t cols, size_t rows)
{
  width = cols;
  height = rows;
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

/* Prints a printable representation of the character c on the screen in the
 * specified font, and updates the cursor position x. On exit, the font is set
 * to FONT_NORMAL.
 *
 * Printing is suppressed if x reaches term_width(); in this case x is set to
 * term_width.
 *
 * This function is implemented in terms of term_addch().
 */
static void outch(int c, Font font, size_t *x)
{
  size_t j, w, width = term_width();
  char *buf;

  if (*x >= width)
    return;

  term_attrset(1, font);

  if (c == '\t')
    for (w = cur_tab_width - *x % cur_tab_width; w > 0 && *x < width; w--)
      term_addch(' '), ++(*x);
  else if (isprint(c))
    term_addch(c), ++(*x);
  else {
    j = make_char_printable(&buf, (size_t)c);
    for (w = 0; w < j && *x < width; ++w)
      term_addch(buf[w]), ++(*x);
    free(buf);
  }

  term_attrset(1, FONT_NORMAL);
}

/*
 * Tests whether the specified line and offset is within the specified Region.
 * The offset is measured in characters, not in character positions.
 */
static int in_region(size_t lineno, size_t x, Region *r)
{
  Point pt;

  pt.n = lineno;
  pt.o = x;

  return cmp_point(r->start, pt) != 1 && cmp_point(pt, r->end) == -1;
}

/*
 * Sets 'r->start' to the lesser of the point and mark,
 * and sets 'r->end' to the greater. If the mark is not anchored, it is treated
 * as if it were at the point.
 */
static void calculate_highlight_region(Region *r)
{
  assert(cur_bp);
  assert(cur_bp->mark);
  r->start = cur_bp->pt;
  if (cur_bp->flags & BFLAG_ANCHORED) {
    r->end = cur_bp->mark->pt;
    if (cmp_point(r->end, r->start) < 0)
      swap_point(&r->end, &r->start);
  } else
    r->end = cur_bp->pt;
}

/*
 * Prints a line on the terminal.
 *  - 'line' is the line number on the terminal.
 *  - 'start_col' is the horizontal scroll offset: the character position (not
 *    cursor position) within 'lp' of the first character that should be
 *    displayed.
 *  - 'lp' is the line to display.
 *  - 'lineno' is the line number of 'lp' within the buffer.
 */
static void draw_line(size_t line, size_t startcol, Line *lp, size_t lineno)
{
  size_t x, i;
  Region r;

  calculate_highlight_region(&r);

  term_move(line, 0);
  for (x = 0, i = startcol; i < astr_len(lp->item) && x < win.ewidth; i++) {
    if (in_region(lineno, i, &r))
      outch(*astr_char(lp->item, (ptrdiff_t)i), FONT_REVERSE, &x);
    else
      outch(*astr_char(lp->item, (ptrdiff_t)i), FONT_NORMAL, &x);
  }

  if (x >= term_width()) {
    term_move(line, win.ewidth - 1);
    term_addch('$');
  } else
    for (; x < win.ewidth && in_region(lineno, i, &r); ++i)
      outch(' ', FONT_REVERSE, &x);
}

static void draw_window(size_t topline)
{
  size_t i, startcol, lineno;
  Line *lp;
  Point pt;

  assert(cur_bp);
  pt = cur_bp->pt;

  /* Find the first line to display on the first screen line. */
  for (lp = pt.p, lineno = pt.n, i = win.topdelta;
       i > 0 && list_prev(lp) != cur_bp->lines; lp = list_prev(lp), --i, --lineno);

  cur_tab_width = tab_width();

  /* Draw the window lines. */
  for (i = topline; i < win.eheight + topline; ++i, ++lineno) {
    /* Clear the line. */
    term_move(i, 0);
    term_clrtoeol();

    /* If at the end of the buffer, don't write any text. */
    if (lp == cur_bp->lines)
      continue;

    startcol = point_start_column;

    draw_line(i, startcol, lp, lineno);

    if (point_start_column > 0) {
      term_move(i, 0);
      term_addch('$');
    }

    lp = list_next(lp);
  }
}

static char *make_mode_line_flags(void)
{
  static char buf[3];

  if ((cur_bp->flags & (BFLAG_MODIFIED | BFLAG_READONLY)) == (BFLAG_MODIFIED | BFLAG_READONLY))
    buf[0] = '%', buf[1] = '*';
  else if (cur_bp->flags & BFLAG_MODIFIED)
    buf[0] = buf[1] = '*';
  else if (cur_bp->flags & BFLAG_READONLY)
    buf[0] = buf[1] = '%';
  else
    buf[0] = buf[1] = '-';

  return buf;
}

/*
 * Calculate the best start column to draw if the line needs to be
 * truncated.
 * Called only for the line where the point is.
 */
static void calculate_start_column(void)
{
  size_t col = 0, lastcol = 0, t = tab_width();
  int rpfact, lpfact;
  char *buf, *rp, *lp, *p;
  Point pt;

  assert(cur_bp);
  pt = cur_bp->pt;

  rp = astr_char(pt.p->item, (ptrdiff_t)pt.o);
  rpfact = pt.o / (win.ewidth / 3);

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

    lpfact = (lp - astr_cstr(pt.p->item)) / (win.ewidth / 3);

    if (col >= win.ewidth - 1 || lpfact < (rpfact - 2)) {
      point_start_column = lp + 1 - astr_cstr(pt.p->item);
      point_screen_column = lastcol;
      return;
    }

    lastcol = col;
  }

  point_start_column = 0;
  point_screen_column = col;
}

static char *make_screen_pos(char **buf)
{
  Point pt;

  assert(cur_bp);
  pt = cur_bp->pt;

  if (cur_bp->num_lines <= win.eheight && win.topdelta == pt.n)
    zasprintf(buf, "All");
  else if (pt.n == win.topdelta)
    zasprintf(buf, "Top");
  else if (pt.n + (win.eheight - win.topdelta) > cur_bp->num_lines)
    zasprintf(buf, "Bot");
  else
    zasprintf(buf, "%2d%%", (int)((float)pt.n / cur_bp->num_lines * 100));

  return *buf;
}

static void draw_border(void)
{
  size_t i;
  term_attrset(1, FONT_REVERSE);
  for (i = 0; i < win.ewidth; ++i)
    term_addch('-');
  term_attrset(1, FONT_NORMAL);
}

static void draw_status_line(size_t line)
{
  int someflag = 0;
  char *buf;
  Point pt;

  assert(cur_bp);
  pt = cur_bp->pt;

  term_move(line, 0);
  draw_border();

  term_attrset(1, FONT_REVERSE);

  term_move(line, 0);
  term_printf("--%2s- %-18s (", make_mode_line_flags(), cur_bp->name);

  if (cur_bp->flags & BFLAG_AUTOFILL) {
    term_printf("Fill");
    someflag = 1;
  }
  if (thisflag & FLAG_DEFINING_MACRO) {
    term_printf("%sDef", someflag ? " " : "");
    someflag = 1;
  }
  if (cur_bp->flags & BFLAG_ISEARCH)
    term_printf("%sIsearch", someflag ? " " : "");

  term_printf(")--L%d--C%d--%s",
              pt.n + 1, get_goalc(), make_screen_pos(&buf));
  free(buf);

  term_attrset(1, FONT_NORMAL);
}

/*
 * Draw the popup window.
 */
static void draw_popup(void)
{
  Line *popup = popup_get();

  if (popup) {
    Line *lp;
    size_t i, y = 0;

    /* Add 3 to popup_lines for the border above, and minibuffer and
       status line below. */
    if (term_height() - 3 > popup_lines())
      y = term_height() - 3 - popup_lines();
    term_move(y++, 0);
    draw_border();

    /* Skip down to first line to display. */
    for (i = 0, lp = list_first(popup); i < popup_pos(); i++, lp = list_next(lp))
      ;

    /* Draw lines. */
    for (; i < popup_lines() && y < term_height() - 2;
         i++, y++, lp = list_next(lp))
      term_printf("%.*s\n", term_width(), astr_cstr(lp->item));

    /* Draw blank lines to bottom of window. */
    for (; y < term_height() - 2; y++)
      term_printf("\n");
  }
}

/*
 * Draw all the windows in turn, and draw the status line if any space
 * remains. Finally, move the cursor to the correct position.
 */
void term_display(void)
{
  size_t topline;
  cur_topline = topline = 0;

  if (cur_bp)
    calculate_start_column();

  cur_topline = topline;

  if (cur_bp) {
    draw_window(topline);

    /* Draw the status line only if there is available space after the
       buffer text space. */
    if (win.fheight - win.eheight > 0)
      draw_status_line(topline + win.eheight);
  }

  topline += win.fheight;

  /* Draw the popup window. */
  draw_popup();

  /* Redraw cursor. */
  term_move(cur_topline + win.topdelta, point_screen_column);
}

void term_full_redisplay(void)
{
  term_clear();
  term_display();
}

/*
 * Tidy up the term ready to suspend or quit.
 */
void term_tidy(void)
{
  term_move(term_height() - 1, 0);
  term_clrtoeol();
  term_attrset(1, FONT_NORMAL);
  term_refresh();
}

/*
 * Print a string on the terminal.
 */
static void term_print(const char *s)
{
  size_t i;

  for (i = 0; *s != '\0'; s++)
    if (*s != '\n')
      term_addch(*s);
    else {
      term_clrtoeol();
      term_nl();
    }
}

/*
 * printf on the terminal
 */
int term_printf(const char *fmt, ...)
{
  char *buf;
  int res = 0;
  va_list ap;
  va_start(ap, fmt);
  res = zvasprintf(&buf, fmt, ap);
  va_end(ap);
  term_print(buf);
  free(buf);
  return res;
}

void resize_window(void)
{
  int hdelta;

  /* Resize window horizontally. */
  win.fwidth = win.ewidth = term_width();

  /* Work out difference in window height; window may be taller than
     terminal if the terminal was very short. */
  hdelta = term_height() - 1;
  hdelta -= win.fheight;

  /* Resize window vertically. */
  if (hdelta > 0) { /* Increase window height. */
    while (hdelta > 0) {
      ++win.fheight;
      ++win.eheight;
      --hdelta;
    }
  } else { /* Decrease window height. */
    int decreased = TRUE;
    while (decreased) {
      decreased = FALSE;
      while (hdelta < 0) {
        if (win.fheight > 2) {
          --win.fheight;
          --win.eheight;
          ++hdelta;
          decreased = TRUE;
        } else
          /* Can't erase windows any longer. */
          assert(0);
      }
    }
  }

  if (hdelta != 0)
    FUNCALL(recenter);
}
