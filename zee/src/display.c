/* Terminal-independent display routines
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2006 Reuben Thomas.
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

#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>

#include "main.h"
#include "rblist.h"
#include "extern.h"


void resync_display(void)
{
  static size_t lastpointn = 0;
  int delta = buf->pt.n - lastpointn;

  if (delta) {
    if ((delta > 0 && win.topdelta + delta < win.eheight) ||
        (delta < 0 && win.topdelta >= (size_t)(-delta)))
      win.topdelta += delta;
    else if (buf->pt.n > win.eheight / 2)
      win.topdelta = win.eheight / 2;
    else
      win.topdelta = buf->pt.n;
  }
  lastpointn = buf->pt.n;
}

DEF(recenter,
"\
Center point in window and redisplay screen.\n\
The desired position of point is always relative to the current window.\
")
{
  if (buf) {
    if (buf->pt.n > win.eheight / 2)
      win.topdelta = win.eheight / 2;
    else
      win.topdelta = buf->pt.n;
  }
  term_clear();
  term_display();
}
END_DEF


/* Contents of popup window. */
static rblist popup_text = NULL;
static size_t popup_line = 0;

/*
 * Set the popup string to as, which should not have a trailing newline.
 * Passing NULL for as clears the popup string.
 */
void popup_set(rblist as)
{
  popup_text = as;
  popup_line = 0;
}

/*
 * Clear the popup string.
 */
void popup_clear(void)
{
  popup_set(NULL);
}

/*
 * Scroll the popup text and loop having reached the bottom.
 */
void popup_scroll_down_and_loop(void)
{
  const size_t h = term_height() - 3;
  const size_t inc = h;
  popup_line += inc;
  if (popup_line > rblist_nl_count(popup_text))
    popup_line = 0;
  term_display();
}

/*
 * Scroll down the popup text.
 */
void popup_scroll_down(void)
{
  const size_t h = term_height() - 3;
  const size_t inc = h;
  popup_line = min(popup_line + inc, rblist_nl_count(popup_text) + 1 - h);
  term_display();
}

/*
 * Scroll up the popup text.
 */
void popup_scroll_up(void)
{
  const size_t h = term_height() - 3;
  const size_t inc = h;
  popup_line -= min(inc, popup_line);
  term_display();
}


/* Window properties */

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

static rblist make_char_printable(size_t c)
{
  if (c == '\0')
    return rblist_from_string("^@");
  else if (c <= '\32')
    return astr_afmt("^%c", 'A' + c - 1);
  else
    return astr_afmt("\\%o", c & 0xff);
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
  size_t w, width = term_width();

  if (*x >= width)
    return;

  term_attrset(1, font);

  if (c == '\t')
    for (w = cur_tab_width - *x % cur_tab_width; w > 0 && *x < width; w--)
      term_addch(' '), ++(*x);
  else if (isprint(c))
    term_addch(c), ++(*x);
  else
    RBLIST_FOR(c, make_char_printable((size_t)c))
      if (*x >= width) break;
      term_addch(c);
      ++(*x);
    RBLIST_END

  term_attrset(1, FONT_NORMAL);
}

/*
 * Tests whether the specified line and offset is within the specified Region.
 * The offset is measured in characters, not in character positions.
 */
static bool in_region(size_t lineno, size_t x, Region *r)
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
  assert(buf->mark);
  r->start = buf->pt;
  if (buf->flags & BFLAG_ANCHORED) {
    r->end = buf->mark->pt;
    if (cmp_point(r->end, r->start) < 0)
      swap_point(&r->end, &r->start);
  } else
    r->end = buf->pt;
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
  for (x = 0, i = startcol; i < rblist_length(lp->item) && x < win.ewidth; i++) {
    if (in_region(lineno, i, &r))
      outch(rblist_get(lp->item, i), FONT_REVERSE, &x);
    else
      outch(rblist_get(lp->item, i), FONT_NORMAL, &x);
  }

  if (x >= term_width()) {
    term_move(line, win.ewidth - 1);
    term_addch('$');
  } else
    /* In the following loop is in_region(lineno, i, &r) constant?
       If so, replace with an if and a while. */
    for (; x < win.ewidth && in_region(lineno, i, &r); i++)
      outch(' ', FONT_REVERSE, &x);
}

static void draw_window(size_t topline)
{
  size_t i, startcol, lineno;
  Line *lp;
  Point pt = buf->pt;

  /* Find the first line to display on the first screen line. */
  for (lp = pt.p, lineno = pt.n, i = win.topdelta;
       i > 0 && list_prev(lp) != buf->lines;
       lp = list_prev(lp), --i, --lineno);

  cur_tab_width = tab_width();

  /* Draw the window lines. */
  for (i = topline; i < win.eheight + topline; ++i, ++lineno) {
    /* Clear the line. */
    term_move(i, 0);
    term_clrtoeol();

    /* If at the end of the buffer, don't write any text. */
    if (lp == buf->lines)
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

/*
 * Print a string on the terminal.
 */
void term_print(rblist as)
{
  RBLIST_FOR(c, as)
    if (c != '\n')
      term_addch(c);
    else {
      term_clrtoeol();
      term_nl();
    }
  RBLIST_END
}

/*
 * Calculate the best start column to draw if the line needs to be
 * truncated.
 * Called only for the line where the point is.
 * FIXME: What the hell does this actually do?! And is it sensible?
 */
static void calculate_start_column(void)
{
  size_t col = 0, lastcol = 0;
  size_t rp, lp, p, rpfact, lpfact;
  Point pt = buf->pt;

  rp = pt.o;
  rpfact = pt.o / (win.ewidth / 3);

  for (lp = rp; ; lp--) {
    for (col = 0, p = lp; p < rp; ++p)
      if (rblist_get(pt.p->item, p) == '\t') {
        /* FIXME: Broken unless lp starts at a tab position. */
        size_t t = tab_width();
        col += t - (col % t);
      } else if (isprint(rblist_get(pt.p->item, p)))
        ++col;
      else
        col += rblist_length(make_char_printable((size_t)rblist_get(pt.p->item, p)));

    lpfact = lp / (win.ewidth / 3);

    if (col >= win.ewidth - 1 || lpfact + 2 < rpfact) {
      point_start_column = lp + 1;
      point_screen_column = lastcol;
      return;
    }

    lastcol = col;

    if (lp == 0)
      break;
  }

  point_start_column = 0;
  point_screen_column = col;
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

  term_move(line, 0);
  draw_border();

  term_attrset(1, FONT_REVERSE);

  term_move(line, 0);
  term_print(rblist_from_string("--"));

  if ((buf->flags & (BFLAG_MODIFIED | BFLAG_READONLY)) == (BFLAG_MODIFIED | BFLAG_READONLY))
    term_print(rblist_from_string("%*"));
  else if (buf->flags & BFLAG_MODIFIED)
    term_print(rblist_from_string("**"));
  else if (buf->flags & BFLAG_READONLY)
    term_print(rblist_from_string("%%"));
  else
    term_print(rblist_from_string("--"));

  term_print(rblist_from_string("-("));
  if (buf->flags & BFLAG_AUTOFILL) {
    term_print(rblist_from_string("Fill"));
    someflag = 1;
  }
  if (thisflag & FLAG_DEFINING_MACRO) {
    if (someflag)
      term_print(rblist_from_string(" "));
    term_print(rblist_from_string("Def"));
    someflag = 1;
  }
  if (buf->flags & BFLAG_ISEARCH) {
    if (someflag)
      term_print(rblist_from_string(" "));
    term_print(rblist_from_string("Isearch"));
  }

  term_print(astr_afmt(")--L%d--C%d--", buf->pt.n + 1, get_goalc()));

  if (buf->num_lines <= win.eheight && win.topdelta == buf->pt.n)
    term_print(rblist_from_string("All"));
  else if (buf->pt.n == win.topdelta)
    term_print(rblist_from_string("Top"));
  else if (buf->pt.n + (win.eheight - win.topdelta) > buf->num_lines)
    term_print(rblist_from_string("Bot"));
  else
    term_print(astr_afmt("%d%%", (int)((float)buf->pt.n / buf->num_lines * 100)));

  term_attrset(1, FONT_NORMAL);
}

/*
 * Draw the popup window.
 */
static void draw_popup(void)
{
  assert(popup_text);

  /* Number of lines of popup_text that will fit on the terminal.
   * Allow 3 for the border above, and minibuffer and status line below. */
  const size_t h = term_height() - 3;
  /* Number of lines is one more than number of newline characters. */
  const size_t l = rblist_nl_count(popup_text) + 1;
  /* Position of top of popup == number of lines not to use. */
  const size_t y = h > l ? h - l : 0;
  /* Number of lines to print. */
  const size_t p = min(h - y, l - popup_line);

  term_move(y, 0);
  draw_border();

  size_t start_pos = rblist_line_to_start_pos(popup_text, popup_line);
  size_t end_pos = rblist_line_to_end_pos(popup_text, popup_line + p - 1);
  term_print(rblist_sub(popup_text, start_pos, end_pos));
  term_print(astr_nl());

  /* Draw blank lines to bottom of window. */
  for (size_t i = h - y - p; i > 0; i--)
    term_print(astr_nl());
}

/*
 * Draw all the windows in turn, and draw the status line if any space
 * remains. Finally, move the cursor to the correct position.
 */
void term_display(void)
{
  size_t topline;

  cur_topline = topline = 0;
  calculate_start_column();
  cur_topline = topline;
  if (buf)
    draw_window(topline);

  /* Draw the status line only if there is available space after the
     buffer text space. */
  if (win.fheight - win.eheight > 0)
    draw_status_line(topline + win.eheight);

  topline += win.fheight;

  /* Draw the popup window. */
  if (popup_text)
    draw_popup();

  /* Redraw cursor. */
  term_move(cur_topline + win.topdelta, point_screen_column);
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
    int decreased = true;
    while (decreased) {
      decreased = false;
      while (hdelta < 0) {
        if (win.fheight > 2) {
          --win.fheight;
          --win.eheight;
          ++hdelta;
          decreased = true;
        } else
          /* FIXME: Window too small! */
          assert(0);
      }
    }
  }

  if (hdelta != 0)
    CMDCALL(recenter);
}

/*
 * Emit an error sound and cancel any macro definition.
 */
void ding(void)
{
  term_beep();

  if (thisflag & FLAG_DEFINING_MACRO)
    cancel_kbd_macro();
}
