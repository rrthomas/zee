/* History facility functions
   Copyright (c) 2004 David A. Capello.  All rights reserved.

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

#include "main.h"
#include "extern.h"

void add_history_element(History *hp, rblist string)
{
  rblist last;

  if (!hp->elements)
    hp->elements = list_new();

  last = list_last(hp->elements)->item;
  if (!last || rblist_compare(last, string) != 0)
    list_append(hp->elements, string);
}

void prepare_history(History *hp)
{
  hp->sel = NULL;
}

rblist previous_history_element(History *hp)
{
  rblist rbl = NULL;

  if (hp->elements) {
    if (!hp->sel) { // First call for this history.
      // Select last element.
      if (list_last(hp->elements) != hp->elements) {
        hp->sel = list_last(hp->elements);
        rbl = hp->sel->item;
      }
    } else if (list_prev(hp->sel) != hp->elements) {
      // If there is there another element, select it.
      hp->sel = list_prev(hp->sel);
      rbl = hp->sel->item;
    }
  }

  return rbl;
}

rblist next_history_element(History *hp)
{
  rblist rbl = NULL;

  if (hp->elements && hp->sel) {
    // Next element.
    if (list_next(hp->sel) != hp->elements) {
      hp->sel = list_next(hp->sel);
      rbl = hp->sel->item;
    } else              // No more elements (back to original status).
      hp->sel = NULL;
  }

  return rbl;
}
