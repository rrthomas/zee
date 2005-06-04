/* Windows-handling functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2005 Reuben Thomas.
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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"

Window *window_new(void)
{
  return zmalloc(sizeof(Window));
}

/*
 * Free the window allocated memory.
 */
void free_window(Window *wp)
{
  if (wp->saved_pt)
    free_marker(wp->saved_pt);

  free(wp);
}

/*
 * Free all the allocated windows (used at exit).
 */
void free_windows(void)
{
  Window *wp, *next;

  for (wp = head_wp; wp != NULL; wp = next) {
    next = wp->next;
    free_window(wp);
  }
}

/*
 * Set the current window. Also, make its buffer the current buffer.
 */
void set_current_window(Window *wp)
{
  /* Save buffer's point in a new marker.  */
  if (cur_wp->saved_pt)
    free_marker(cur_wp->saved_pt);

  cur_wp->saved_pt = point_marker();

  /* Change the current window.  */
  cur_wp = wp;

  /* Change the current buffer.  */
  cur_bp = wp->bp;

  /* Update the buffer point with the window's saved point
     marker.  */
  if (cur_wp->saved_pt) {
    cur_bp->pt = cur_wp->saved_pt->pt;
    free_marker(cur_wp->saved_pt);
    cur_wp->saved_pt = NULL;
  }
}

DEFUN_INT("window-split", window_split)
/*+
Split current window into two windows, one above the other.
Both windows display the same buffer now current.
+*/
{
  Window *newwp;

  /* Windows smaller than 4 lines cannot be split. */
  if (cur_wp->fheight < 4) {
    minibuf_error("Window height %d too small for splitting", cur_wp->fheight);
    ok = FALSE;
  } else {
    newwp = window_new();
    newwp->fwidth = cur_wp->fwidth;
    newwp->ewidth = cur_wp->ewidth;
    newwp->fheight = (cur_wp->fheight + 1) / 2;
    newwp->eheight = newwp->fheight - 1;
    cur_wp->fheight = cur_wp->fheight / 2;
    cur_wp->eheight = cur_wp->fheight - 1;
    if (cur_wp->topdelta >= cur_wp->eheight)
      recenter(cur_wp);
    newwp->bp = cur_wp->bp;
    newwp->saved_pt = point_marker();
    newwp->next = cur_wp->next;
    cur_wp->next = newwp;
  }
}
END_DEFUN

void delete_window(Window *del_wp)
{
  Window *wp;

  if (del_wp == head_wp)
    wp = head_wp = head_wp->next;
  else
    for (wp = head_wp; wp != NULL; wp = wp->next)
      if (wp->next == del_wp) {
        wp->next = wp->next->next;
        break;
      }

  wp->fheight += del_wp->fheight;
  wp->eheight += del_wp->eheight + 1;

  set_current_window(wp);
  free_window(del_wp);
}

DEFUN_INT("window-close", window_close)
/*+
Remove the current window from the screen.
+*/
{
  if (cur_wp == head_wp && cur_wp->next == NULL) {
    minibuf_error("Attempt to close sole ordinary window");
    ok = FALSE;
  } else
    delete_window(cur_wp);
}
END_DEFUN

Window *popup_window(void)
{
  if (head_wp->next == NULL) {
    /* There is only one window on the screen, so split it. */
    FUNCALL(window_split);
    return cur_wp->next;
  }

  /* Use the window after the current one. */
  if (cur_wp->next != NULL)
    return cur_wp->next;

  /* Use the first window. */
  return head_wp;
}

DEFUN_INT("window-close-others", window_close_others)
/*+
Make the current window fill the screen.
+*/
{
  Window *wp, *nextwp;

  for (wp = head_wp; wp != NULL; wp = nextwp) {
    nextwp = wp->next;
    if (wp != cur_wp)
      free_window(wp);
  }

  cur_wp->fwidth = cur_wp->ewidth = term_width();
  /* Save space for minibuffer. */
  cur_wp->fheight = term_height() - 1;
  /* Save space for status line. */
  cur_wp->eheight = cur_wp->fheight - 1;
  cur_wp->next = NULL;
  head_wp = cur_wp;
}
END_DEFUN

DEFUN_INT("window-next", window_next)
/*+
Select the first different window on the screen.
All windows are arranged in a cyclic order.
This command selects the window one step away in that order.
+*/
{
  set_current_window((cur_wp->next != NULL) ? cur_wp->next : head_wp);
}
END_DEFUN

Window *find_window(const char *name)
{
  Window *wp;

  for (wp = head_wp; wp != NULL; wp = wp->next)
    if (!strcmp(wp->bp->name, name))
      return wp;

  return NULL;
}

Point window_pt(Window *wp)
{
  assert(cur_bp); /* FIXME: Remove this assumption. */

  /* The current window uses the current buffer point; all other
     windows have a saved point.  */
  assert(wp != NULL);
  if (wp == cur_wp) {
    assert(wp->bp == cur_bp);
    assert(wp->saved_pt == NULL);
    return cur_bp->pt;
  } else {
    assert(wp->saved_pt != NULL);
    return wp->saved_pt->pt;
  }
}
