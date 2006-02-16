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

#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"

void add_history_element(History *hp, astr string)
{
  const char *last;

  if (!hp->elements)
    hp->elements = list_new();

  last = list_last(hp->elements)->item;
  if (!last || astr_cmp(last, string) != 0)
    list_append(hp->elements, astr_dup(string));
}

void prepare_history(History *hp)
{
  hp->sel = NULL;
}

astr previous_history_element(History *hp)
{
  astr as = NULL;

  if (hp->elements) {
    if (!hp->sel) { /* First call for this history. */
      /* Select last element. */
      if (list_last(hp->elements) != hp->elements) {
        hp->sel = list_last(hp->elements);
        as = hp->sel->item;
      }
    }
    /* Is there another element? */
    else if (list_prev(hp->sel) != hp->elements) {
      /* Select it. */
      hp->sel = list_prev(hp->sel);
      as = hp->sel->item;
    }
  }

  return as ? as : NULL;
}

astr next_history_element(History *hp)
{
  astr as = NULL;

  if (hp->elements && hp->sel) {
    /* Next element. */
    if (list_next(hp->sel) != hp->elements) {
      hp->sel = list_next(hp->sel);
      as = hp->sel->item;
    }
    /* No more elements (back to original status). */
    else
      hp->sel = NULL;
  }

  return as ? as : NULL;
}
