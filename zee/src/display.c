/* Terminal-independent display routines
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2007 Reuben Thomas.
   Copyright (c) 2007 Alistair Turnbull.
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
#include "term.h"
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
    lastpointn = buf->pt.n;
  }
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


// Contents of popup window.
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


// Window properties

static size_t width = 0, height = 0;
static size_t start_column; // Column of screen on page (i.e. scroll offset).
static size_t point_screen_column; // Column of cursor on screen.

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

static rblist make_char_printable(int c)
{
  const char ctrls[] = "@ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  c &= 0xff;

  if ((size_t)c < sizeof(ctrls))
    return rblist_fmt("^%c", ctrls[c]);
  else if (isprint(c))
    return rblist_singleton(c); // FIXME: Won't work for double-width characters.
  else
    return rblist_fmt("\\%o", c);
}

/*
 * Prints character c (which must be printable and single-width) provided
 * the column x is visible (i.e. at least 'start_col and less than term_width())
 * and returns the next column to the right.
 */
static size_t outch_printable(int c, size_t x)
{
  if (x >= start_column && x < start_column + term_width())
    term_addch(c);
  return x + 1;
}

/*
 * Prints a printable representation of the character `c' on the
 * screen in font `font' at column `x', where the tab width is `tab',
 * and returns the new column. On exit, the font is set to
 * FONT_NORMAL.
 *
 * Printing is suppressed if x is less than start_column or at least
 * start_column + term_width().
 */
static size_t outch(int c, Font font, size_t x, size_t tab)
{
  term_attrset(1, font);

  if (c == '\t')
    for (size_t w = tab - (x % tab); w > 0; w--)
      x = outch_printable(' ', x);
  else if (isprint(c))
    x = outch_printable(c, x);
  else
    RBLIST_FOR(d, make_char_printable(c))
      x = outch_printable(d, x);
    RBLIST_END

  term_attrset(1, FONT_NORMAL);

  return x;
}

/*
 * Tests whether the specified line and offset is within the specified
 * Region. The offset is measured in characters, not in character
 * positions.
 */
static bool in_region(size_t line, size_t x, Region *r)
{
  Point pt;

  pt.n = line;
  pt.o = x;

  return point_dist(r->start, pt) <= 0 && point_dist(pt, r->end) < 0;
}

/*
 * Sets 'r->start' to the lesser of the point and mark,
 * and sets 'r->end' to the greater. If the mark is not anchored, it
 * is treated as if it were at the point.
 */
static void calculate_highlight_region(Region *r)
{
  assert(buf->mark);
  r->start = buf->pt;
  if (buf->flags & BFLAG_ANCHORED) {
    r->end = buf->mark->pt;
    if (point_dist(r->end, r->start) < 0)
      swap_point(&r->end, &r->start);
  } else
    r->end = buf->pt;
}

/*
 * Prints a line on the terminal.
 *  - `row' is the line number on the terminal.
 *  - `line' is the line number within the buffer.
 *  - `tab' is the display width of a tab character.
 *
 * This function reads the static variable start_column: the horizontal
 * scroll offset.
 *
 * If any part of the line is off the left-hand side of the screen,
 * prints a '$' character in the left-hand column. If any part is off
 * the right, prints a '$' character in the right-hand column. Any
 * part that is within the highlight region is highlighted. If the
 * final position is within the highlight region then the remainder of
 * the line is also highlighted.
 */
static void draw_line(size_t row, size_t line, size_t tab)
{
  Region r;
  calculate_highlight_region(&r);

  term_move(row, 0);
  size_t x = 0, i = 0;
  RBLIST_FOR(c, rblist_line(buf->lines, line))
    if (x >= start_column + term_width())
      break; // No point in printing the rest of the line.
    Font font = in_region(line, i, &r) ? FONT_REVERSE : FONT_NORMAL;
    x = outch(c, font, x, tab);
    i++;
  RBLIST_END

  if (start_column > 0) {
    term_move(row, 0);
    term_addch('$');
  }
  if (x >= start_column + term_width()) {
    term_move(row, win.ewidth - 1);
    term_addch('$');
  } else if (in_region(line, i, &r))
    while (x < win.ewidth)
      x = outch(' ', FONT_REVERSE, x, tab);
}

/*
 * Find the character position in `rbl' corresponding to display
 * width `goal'.
 */
// FIXME: At the moment, display widths could overflow (even size_t!).
size_t column_to_character(rblist rbl, size_t goal)
{
  size_t i = 0, col = 0;

  RBLIST_FOR(c, rbl)
    // FIXME: What should we do if we overrun (col > goal)?
    if (col >= goal)
      break;
    col += rblist_length(make_char_printable(c));
    i++;
  RBLIST_END

  return i;
}

/*
 * Calculate the display width of a string in screen columns
 */
size_t string_display_width(rblist rbl)
{
  size_t col_count = 0, t = tab_width();

  RBLIST_FOR(c, rbl)
    if (c == '\t')
      col_count += t - (col_count % t);
    else
      col_count += rblist_length(make_char_printable(c));
  RBLIST_END

  return col_count;
}

/*
 * Calculate start_column (the horizontal scroll offset) and
 * point_screen_column (the horizontal position of the cursor on the
 * screen).
 *
 * The start_column is always a multiple of a third of a screen width. It is
 * chosen so as to put the cursor in the middle third, unless the cursor is near
 * one or other end of the line, in which case it is chosen to show as much of
 * the line as possible.
 */
static void calculate_start_column(void)
{
  const size_t width = term_width(), third_width = max(1, width / 3);
  
  // Calculate absolute column of cursor and of end of line.
  const rblist line = rblist_line(buf->lines, buf->pt.n);
  const size_t x = string_display_width(rblist_sub(line, 0, buf->pt.o));
  const size_t length = string_display_width(line);
  
  // Choose start_column.
  if (x < third_width || length < width) {
    // No-brainer cases: show left-hand end of line.
    start_column = 0;
  } else {
    // Put cursor in the middle third.
    start_column = x - (x % third_width) - third_width;
    // But scroll left if the right-hand end of the line stays on the screen.
    while (start_column + width >= length + third_width)
      start_column -= third_width;
  }

  // Consequently, calculate screen-relative column.
  point_screen_column = x - start_column;
}

/*
 * Draws the window.
 */
static void draw_window(void)
{
  // Find the first line to display on the first screen line.
  size_t line = max(buf->pt.n - win.topdelta, 0);
  size_t tab = tab_width();

  calculate_start_column();

  // Draw the window lines.
  for (size_t i = 0; i < win.eheight; ++i, ++line) {
    term_move(i, 0);
    term_clrtoeol();
    if (line < rblist_nl_count(buf->lines))
      draw_line(i, line, tab);
  }
}

/*
 * Print a string on the terminal.
 */
void term_print(rblist as)
{
  RBLIST_FOR(c, as)
    term_addch(c);
  RBLIST_END
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
  bool someflag = false;

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
    someflag = true;
  }
  if (thisflag & FLAG_DEFINING_MACRO) {
    if (someflag)
      term_print(rblist_from_string(" "));
    term_print(rblist_from_string("Def"));
    someflag = true;
  }
  if (buf->flags & BFLAG_ISEARCH) {
    if (someflag)
      term_print(rblist_from_string(" "));
    term_print(rblist_from_string("Isearch"));
  }

  term_print(rblist_fmt(")--L%d--C%d--", buf->pt.n + 1, get_goalc()));

  if (rblist_nl_count(buf->lines) <= win.eheight && win.topdelta == buf->pt.n)
    term_print(rblist_from_string("All"));
  else if (buf->pt.n == win.topdelta)
    term_print(rblist_from_string("Top"));
  else if (buf->pt.n + (win.eheight - win.topdelta) > rblist_nl_count(buf->lines))
    term_print(rblist_from_string("Bot"));
  else
    term_print(rblist_fmt("%d%%", (int)((float)buf->pt.n / rblist_nl_count(buf->lines) * 100)));

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
  // Number of lines is one more than number of newline characters.
  const size_t l = rblist_nl_count(popup_text) + 1;
  // Position of top of popup == number of lines not to use.
  const size_t y = h > l ? h - l : 0;
  // Number of lines to print.
  const size_t p = min(h - y, l - popup_line);

  term_move(y, 0);
  draw_border();

  // Draw popup text, and blank lines to bottom of window.
  for (size_t i = 0; i < h - y; i++) {
    if (i < p)
      term_print(rblist_sub(popup_text,
                            rblist_line_to_start_pos(popup_text, popup_line + i),
                            rblist_line_to_end_pos(popup_text, popup_line + i)));
    term_clrtoeol();
    term_nl();
  }
}

/*
 * Draw the window, and draw the status line if any space
 * remains. Finally, move the cursor to the correct position.
 */
void term_display(void)
{
  if (buf)
    draw_window();

  /* Draw the status line only if there is available space after the
     buffer text space. */
  if (win.fheight - win.eheight > 0)
    draw_status_line(win.eheight);

  // Draw the popup window.
  if (popup_text)
    draw_popup();

  // Redraw cursor.
  term_move(win.topdelta, point_screen_column);
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

  // Resize window horizontally.
  win.fwidth = win.ewidth = term_width();

  /* Work out difference in window height; window may be taller than
     terminal if the terminal was very short. */
  hdelta = term_height() - 1;
  hdelta -= win.fheight;

  // Resize window vertically.
  if (hdelta > 0) { // Increase window height.
    while (hdelta > 0) {
      ++win.fheight;
      ++win.eheight;
      --hdelta;
    }
  } else { // Decrease window height.
    bool decreased = true;
    while (decreased) {
      decreased = false;
      while (hdelta < 0) {
        if (win.fheight > 2) {
          --win.fheight;
          --win.eheight;
          ++hdelta;
          decreased = true;
        } else
          // FIXME: Window too small!
          assert(0);
      }
    }
  }

  if (hdelta != 0)
    CMDCALL(recenter);
}

/*
 * Emit an error sound.
 */
void ding(void)
{
  term_beep();
}
