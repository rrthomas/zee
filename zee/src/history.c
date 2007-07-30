/* History facility functions
   Copyright (c) 2004 David A. Capello.
   Copyright (c) 2007 Reuben Thomas.
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

#include <string.h>

#include "config.h"

#include "main.h"
#include "extern.h"


void add_history_element(History *hp, rblist string)
{
  if (hp->elements == 0)
    hp->elements = list_new();

  if (list_length(hp->elements) == 0 ||
      strcmp(list_get_string(hp->elements, list_length(hp->elements)), rblist_to_string(string)) != 0)
    list_set_string(hp->elements, list_length(hp->elements) + 1, rblist_to_string(string));
}

void prepare_history(History *hp)
{
  hp->sel = 0;
}

rblist previous_history_element(History *hp)
{
  rblist rbl = NULL;

  if (hp->elements) {
    if (hp->sel == 0) { // First call for this history.
      // Select last element.
      if (list_length(hp->elements) > 0) {
        hp->sel = list_length(hp->elements);
        rbl = rblist_from_string(list_get_string(hp->elements, hp->sel));
      }
    } else if (hp->sel > 1) {
      // If there is there another element, select it.
      hp->sel--;
      rbl = rblist_from_string(list_get_string(hp->elements, hp->sel));
    }
  }

  return rbl;
}

rblist next_history_element(History *hp)
{
  rblist rbl = NULL;

  if (hp->elements && hp->sel) {
    // Next element.
    if (hp->sel < list_length(hp->elements)) {
      hp->sel++;
      rbl = rblist_from_string(list_get_string(hp->elements, hp->sel));
    } else              // No more elements (back to original status).
      hp->sel = 0;
  }

  return rbl;
}
