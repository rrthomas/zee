/* Terminal-independent display routines
   Copyright (c) 1997-2004 Sandro Sigala.  All rights reserved.

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

#include <stdarg.h>
#include <assert.h>

#include "config.h"
#include "main.h"
#include "extern.h"

void resync_display(void)
{
  static size_t lastpointn = 0;
  int delta = buf.pt.n - lastpointn;

  if (delta) {
    if ((delta > 0 && win.topdelta + delta < win.eheight) ||
        (delta < 0 && win.topdelta >= (size_t)(-delta)))
      win.topdelta += delta;
    else if (buf.pt.n > win.eheight / 2)
      win.topdelta = win.eheight / 2;
    else
      win.topdelta = buf.pt.n;
  }
  lastpointn = buf.pt.n;
}

void recenter(void)
{
  if (buf.pt.n > win.eheight / 2)
    win.topdelta = win.eheight / 2;
  else
    win.topdelta = buf.pt.n;
}

DEFUN_INT("recenter", recenter)
/*+
Center point in window and redisplay screen.
The desired position of point is always relative to the current window.
+*/
{
  recenter();
  term_full_redisplay();
}
END_DEFUN


/* Contents of popup window. */
static Line *popup = NULL;
static size_t popup_num_lines = 0;
static size_t popup_pos_line = 0;

/*
 * Return the contents of the popup window.
 */
Line *popup_get(void)
{
  return popup;
}

/*
 * Return number of lines in popup.
 */
size_t popup_lines(void)
{
  return popup_num_lines;
}

/*
 * Set the popup string to as, which should not have a trailing newline.
 * Passing NULL for as clears the popup string.
 */
void popup_set(astr as)
{
  if (popup)
    line_delete(popup);

  if (as)
    popup = string_to_lines(as, "\n", &popup_num_lines);
  else {
    popup = NULL;
    popup_num_lines = 0;
  }
  popup_pos_line = 0;
}

/*
 * Clear the popup string.
 */
void popup_clear(void)
{
  popup_set(NULL);
}

/*
 * Return the popup position.
 */
size_t popup_pos(void)
{
  return popup_pos_line;
}

/*
 * Scroll the popup text up.
 */
void popup_scroll_up(void)
{
  if (popup_pos_line + term_height() - 3 < popup_num_lines)
    popup_pos_line += term_height() - 3;
  else
    popup_pos_line = 0;

  term_display();
}

/*
 * Scroll the popup text down.
 */
void popup_scroll_down(void)
{
  if (popup_pos_line >= (term_height() - 3))
    popup_pos_line -= term_height() - 3;
  else if (popup_pos_line > 0)
    popup_pos_line = 0;
  else
    popup_pos_line = popup_num_lines - (term_height() - 3);

  term_display();
}
