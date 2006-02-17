/* Completion facility functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2006 Reuben Thomas.
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
#include "main.h"
#include "extern.h"

/*
 * Allocate a new completion structure.
 */
Completion *completion_new(void)
{
  Completion *cp = zmalloc(sizeof(Completion));

  cp->completions = list_new();
  cp->matches = list_new();

  return cp;
}

/*
 * Calculate the maximum length of the completions.
 */
static size_t calculate_max_length(list l, size_t size)
{
  size_t i, maxlen = 0;
  list p;

  for (p = list_first(l), i = 0; p != l && i < size; p = list_next(p), i++)
    maxlen = max(maxlen, astr_len(p->item));

  return maxlen;
}

/*
 * Write the list of completions in a set of columns.
 */
static astr completion_write(list l, size_t size)
{
  size_t i, j, col, max, numcols;
  list p;
  astr as = astr_new("Possible completions are:\n");

  max = calculate_max_length(l, size) + 5;
  numcols = (win.ewidth - 1) / max;

  for (p = list_first(l), i = col = 0; p != l && i < size; p = list_next(p), i++) {
    if (col >= numcols) {
      col = 0;
      astr_cat_cstr(as, "\n");
    }
    astr_cat(as, p->item);
    for (j = max - astr_len(p->item); j > 0; --j)
      astr_cat_cstr(as, " ");
    ++col;
  }

  return as;
}

/*
 * Popup the completion window.
 */
static void popup_completion(Completion *cp, int allflag, size_t num)
{
  astr popup = astr_new("Completions\n\n");

  cp->flags |= COMPLETION_POPPEDUP;

  if (allflag)
    astr_cat(popup, completion_write(cp->completions, list_length(cp->completions)));
  else
    astr_cat(popup, completion_write(cp->matches, num));

  popup_set(popup);
  term_display();
}

static int hcompar(const void *p1, const void *p2)
{
  return astr_cmp(*(const astr *)p1, *(const astr *)p2);
}

/*
 * Match completions.
 */
int completion_try(Completion *cp, astr search, int popup_when_complete)
{
  size_t i, j;
  size_t fullmatches = 0, partmatches = 0;
  char c;
  list p;

  cp->matches = list_new();

  if (!cp->flags & COMPLETION_SORTED) {
    list_sort(cp->completions, hcompar);
    cp->flags |= COMPLETION_SORTED;
  }

  if (astr_len(search) == 0) {
    cp->match = list_first(cp->completions)->item;
    if (list_length(cp->completions) > 1) {
      cp->matchsize = 0;
      popup_completion(cp, TRUE, 0);
      return COMPLETION_NONUNIQUE;
    } else {
      cp->matchsize = astr_len(cp->match);
      return COMPLETION_MATCHED;
    }
  }

  for (p = list_first(cp->completions); p != cp->completions; p = list_next(p))
    if (!astr_cmp(p->item, search)) {
      ++partmatches;
      list_append(cp->matches, p->item);
      if (!astr_cmp(p->item, search))
        ++fullmatches;
    }

  if (partmatches == 0)
    return COMPLETION_NOTMATCHED;
  else if (partmatches == 1) {
    cp->match = list_first(cp->matches)->item;
    cp->matchsize = astr_len(cp->match);
    return COMPLETION_MATCHED;
  }

  if (fullmatches == 1 && partmatches > 1) {
    cp->match = list_first(cp->matches)->item;
    cp->matchsize = astr_len(cp->match);
    if (popup_when_complete)
      popup_completion(cp, FALSE, partmatches);
    return COMPLETION_MATCHEDNONUNIQUE;
  }

  for (j = astr_len(search); ; j++) {
    list p = list_first(cp->matches);

    c = *astr_char(p->item, (ptrdiff_t)j);
    for (i = 1; i < partmatches; ++i) {
      p = list_next(p);
      if (*astr_char(p->item, (ptrdiff_t)j) != c) {
        cp->match = list_first(cp->matches)->item;
        cp->matchsize = j;
        popup_completion(cp, FALSE, partmatches);
        return COMPLETION_NONUNIQUE;
      }
    }
  }

  assert(0);
  return COMPLETION_NOTMATCHED;
}
