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
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

#include <stdarg.h>
#include <assert.h>

#include "config.h"
#include "main.h"
#include "extern.h"

void resync_display(void)
{
  int delta;

  assert(cur_bp); /* FIXME: Remove this assumption. */

  delta = cur_bp->pt.n - cur_wp->lastpointn;

  if (delta) {
    if ((delta > 0 && cur_wp->topdelta + delta < cur_wp->eheight) ||
        (delta < 0 && cur_wp->topdelta >= (size_t)(-delta)))
      cur_wp->topdelta += delta;
    else if (cur_bp->pt.n > cur_wp->eheight / 2)
      cur_wp->topdelta = cur_wp->eheight / 2;
    else
      cur_wp->topdelta = cur_bp->pt.n;
  }
  cur_wp->lastpointn = cur_bp->pt.n;
}

void recenter(Window *wp)
{
  Point pt = window_pt(wp);

  if (pt.n > wp->eheight / 2)
    wp->topdelta = wp->eheight / 2;
  else
    wp->topdelta = pt.n;
}

DEFUN_INT("recenter", recenter)
/*+
Center point in window and redisplay screen.
The desired position of point is always relative to the current window.
+*/
{
  recenter(cur_wp);
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
