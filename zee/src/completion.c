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
 * Write the astrs in `l' in a set of columns. The width of the
 * columns is chosen to be big enough for the longest astr, with a
 * COLUMN_GAP-character gap between each column.
 */
#define COLUMN_GAP 5
static astr completion_write(list l)
{
  size_t maxlen = 0;
  for (list p = list_first(l); p != l; p = list_next(p))
    maxlen = max(maxlen, astr_len(p->item));
  maxlen += COLUMN_GAP;
  size_t numcols = (win.ewidth + COLUMN_GAP - 1) / maxlen;

  size_t i = 0, col = 0;
  astr as = astr_new("Possible completions are:\n");
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

/*
 * Returns the length of the longest string that is a prefix of
 * both as and bs.
 */
static size_t common_prefix_length(astr as, astr bs) {
  size_t len = min(astr_len(as), astr_len(bs));
  for (size_t i = 0; i < len; i++)
    if (*astr_char(as, (ptrdiff_t)i) != *astr_char(bs, (ptrdiff_t)i))
      return (size_t)i;
  return len;
}

static int hcompar(const void *p1, const void *p2)
{
  return astr_cmp(*(const astr *)p1, *(const astr *)p2);
}

/*
 * Match completions
 * cp - the completions (the list gets sorted)
 * search - the prefix to search for (not modified)
 * Returns COMPLETION_NOTMATCHED (== FALSE) if `search' is not a prefix of any
 * completion, and COMPLETION_MATCHED (== TRUE) otherwise. Never returns
 * COMPLETION_NOTMATCHING.
 *
 * To determine if the match was exact, first check for COMPLETION_MATCHED,
 * then check whether
 *   astr_length(cp->match) == astr_length(list_first(cp->matches)->item)
 * This works because the exact match is first in sorted order.
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

  cp->match = list_first(cp->matches)->item;
  size_t prefix_len = astr_len(cp->match);
  for (list p = list_first(cp->matches); p != cp->matches; p = list_next(p))
    prefix_len = min(prefix_len, common_prefix_length(cp->match, p->item));
  cp->match = astr_sub(cp->match, 0, (ptrdiff_t)prefix_len);

  popup_completion(cp); /* FIXME: Incorrectly leaves popup on screen. Move to callers and handle case-by-case. */
  return COMPLETION_MATCHED;
}
