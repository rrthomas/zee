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
 * Calculate the maximum length of the astrs in `l'.
 */
static size_t calculate_max_length(list l)
{
  size_t maxlen = 0;

  for (list p = list_first(l); p != l; p = list_next(p))
    maxlen = max(maxlen, astr_len(p->item));

  return maxlen;
}

/*
 * Write the astrs in `l' in a set of columns. The width of the
 * columns is chosen to be big enough for the longest astr, with a
 * COLUMN_GAP-character gap between each column.
 */
#define COLUMN_GAP 5
static astr completion_write(list l)
{
  astr as = astr_new("Possible completions are:\n");
  size_t maxlen = calculate_max_length(l) + COLUMN_GAP;
  size_t numcols = (win.ewidth + COLUMN_GAP - 1) / maxlen;

  size_t i = 0, col = 0;
  for (list p = list_first(l);
       p != l && i < list_length(l);
       p = list_next(p), i++) {
    if (col >= numcols) {
      col = 0;
      astr_cat(as, astr_new("\n"));
    }
    astr_cat(as, p->item);
    if (++col < numcols)
      for (size_t i = maxlen - astr_len(p->item); i > 0; i--)
        astr_cat(as, astr_new(" "));
  }

  return as;
}

/*
 * Popup the completion window
 * cp - the completions to show
 */
static void popup_completion(Completion *cp)
{
  cp->flags |= COMPLETION_POPPEDUP;
  popup_set(astr_cat(astr_new("Completions\n\n"), completion_write(cp->matches)));
  term_display();
}

static int hcompar(const void *p1, const void *p2)
{
  return astr_cmp(*(const astr *)p1, *(const astr *)p2);
}

/*
 * Match completions
 * cp - the completions
 * search - the prefix to search for
 * Returns COMPLETION_NOTMATCHED if `search' is not a prefix of any
 * completion, and COMPLETION_MATCHED otherwise.
 */
int completion_try(Completion *cp, astr search)
{
  size_t fullmatches = 0;
  cp->matches = list_new();
  list_sort(cp->completions, hcompar);

  for (list p = list_first(cp->completions); p != cp->completions; p = list_next(p))
    if (!astr_ncmp(p->item, search, astr_len(search))) {
      list_append(cp->matches, p->item);
      if (!astr_cmp(p->item, search))
        ++fullmatches;
    }

  if (list_empty(cp->matches))
    return COMPLETION_NOTMATCHED;

  size_t i;
  cp->match = list_first(cp->matches)->item;
  for (i = astr_len(search); i < astr_len(cp->match); i++) {
    char c = *astr_char(cp->match, (ptrdiff_t)i); /* FIXME: Broken if first match is a prefix of all other matches. */
    for (list p = list_first(cp->matches); p != cp->matches; p = list_next(p)) {
      /* FIXME: Broken if p->item is a prefix of all other matches. */
      if (*astr_char(p->item, (ptrdiff_t)i) != c)
        break;
    }
  }

  cp->match = astr_sub(cp->match, 0, (ptrdiff_t)i);
  popup_completion(cp);
  return COMPLETION_MATCHED;
}
