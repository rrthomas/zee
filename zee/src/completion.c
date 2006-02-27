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
 * Calculate the maximum length of the first 'size' astrs in 'l'.
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
 * Write the first 'size' astrs in 'l' in a set of columns. The width of the
 * columns is chosen to be big enough for the longest astr, plus 5.
 */
static astr completion_write(list l, size_t size)
{
  size_t i, j, col, max, numcols;
  list p;
  astr as = astr_new("Possible completions are:\n");

  max = calculate_max_length(l, size) + 5;
  numcols = (win.ewidth + 5 - 1) / max;

  col = 0;
  for (p = list_first(l), i = 0; p != l && i < size; p = list_next(p), i++) {
    if (col >= numcols) {
      col = 0;
      astr_cat(as, astr_new("\n"));
    }
    astr_cat(as, p->item);
    ++col;
    if (col < numcols)
      for (j = max - astr_len(p->item); j > 0; --j)
        astr_cat(as, astr_new(" "));
  }

  return as;
}

/*
 * Popup the completion window.
 * cp - the completions to show.
 * allflag - true to show completions, false to show matches.
 * Suggestion: always show matches.
 * num - ignored if allflag, otherwise the number of matches to show.
 * Question: It looks like 'num' is always 'list_length(cp->matches)'. Remove?
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
 * cp - the completions.
 * search - the prefix to search for.
 * popup_when_complete - if true, and there is more than one match,
 *   call popup_completion().
 * Returns:
 * COMPLETION_NOTMATCHED if 'search' is not a prefix of any completion.
 * COMPLETION_MATCHED if 'search' is a prefix of exactly one completion.
 * COMPLETION_NONUNIQUE if 'search' is a prefix of more than one completion but
 * not equal to any completion.
 * COMPLETION_MATCHEDNONUNIQUE if 'search' is a prefix of more than one
 * completion and equal to one of them.
 */
int completion_try(Completion *cp, astr search, int popup_when_complete)
{
  size_t j;
  size_t fullmatches = 0;
  char c;
  list p;

  cp->matches = list_new();
  list_sort(cp->completions, hcompar);

  if (astr_len(search) == 0) {
    /* Question: Why is this a special case? */
    cp->match = list_first(cp->completions)->item; /* FIXME: Fails if cp->completions is an empty list. */

    if (list_length(cp->completions) > 1) {
      cp->matchsize = 0; /* FIXME: Wrong if all completions have a common prefix. */
      popup_completion(cp, TRUE, 0); /* FIXME: popup_when_complete ignored. */
      return COMPLETION_NONUNIQUE; /* FIXME: Broken if one of the completions is the empty string. */
    } else {
      cp->matchsize = astr_len(cp->match);
      return COMPLETION_MATCHED;
    }
  }

  for (p = list_first(cp->completions); p != cp->completions; p = list_next(p))
    if (!astr_ncmp(p->item, search, astr_len(search))) {
      list_append(cp->matches, p->item);
      if (!astr_cmp(p->item, search))
        ++fullmatches;
    }

  if (list_empty(cp->matches))
    return COMPLETION_NOTMATCHED;

  cp->match = list_first(cp->matches)->item;
  cp->matchsize = astr_len(cp->match);

  if (list_length(cp->matches) == 1)
    return COMPLETION_MATCHED;

  if (fullmatches == 1 && list_length(cp->matches) > 1) {
    if (popup_when_complete)
      popup_completion(cp, FALSE, list_length(cp->matches));
    return COMPLETION_MATCHEDNONUNIQUE;
  }


  for (j = astr_len(search); ; j++) {
    cp->matchsize = j;
    c = *astr_char(cp->match, (ptrdiff_t)j); /* FIXME: Broken if first match is a prefix of all other matches. */
    for (p = list_first(cp->matches); p != cp->matches; p = list_next(p)) {
      /* FIXME: Broken if p->item is a prefix of all other matches. */
      if (*astr_char(p->item, (ptrdiff_t)j) != c) {
        /* FIXME: Ignores popup_when_complete. */
        popup_completion(cp, FALSE, list_length(cp->matches));
        return COMPLETION_NONUNIQUE;
      }
    }
  }

  assert(0);
  return COMPLETION_NOTMATCHED;
}
