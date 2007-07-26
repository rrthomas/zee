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

#include <stdbool.h>

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
 * Write the rblists in `l' in a set of columns. The width of the
 * columns is chosen to be big enough for the longest rblist, with a
 * COLUMN_GAP-character gap between each column.
 */
#define COLUMN_GAP 5
static rblist completion_write(list l)
{
  size_t maxlen = 0;
  for (list p = list_first(l); p != l; p = list_next(p))
    maxlen = max(maxlen, rblist_length(p->item));
  maxlen += COLUMN_GAP;
  size_t numcols = (win.ewidth + COLUMN_GAP - 1) / maxlen;

  size_t i = 0, col = 0;
  rblist rbl = rblist_empty;
  for (list p = list_first(l);
       p != l && i < list_length(l);
       p = list_next(p), i++) {
    if (col >= numcols) {
      col = 0;
      rbl = rblist_concat(rbl, rblist_from_string("\n"));
    }
    rbl = rblist_concat(rbl, p->item);
    if (++col < numcols)
      for (size_t i = maxlen - rblist_length(p->item); i > 0; i--)
        rbl = rblist_concat(rbl, rblist_from_string(" "));
  }

  return rbl;
}

/*
 * Pop up the completion window, showing cp->matches. If there are no
 * completions, the popup just says "No completions". If there is exactly
 * one completion the popup says "Sole completion: %r".
 */
void completion_popup(Completion *cp)
{
  assert(cp);
  assert(cp->matches);
  rblist rbl = rblist_from_string("Completions\n\n");
  switch (list_length(cp->matches)) {
    case 0:
      rbl = rblist_concat(rbl, rblist_from_string("No completions"));
      break;
    case 1:
      rbl = rblist_concat(rbl, rblist_from_string("Sole completion: "));
      rbl = rblist_concat(rbl, list_first(cp->matches)->item);
      break;
    default:
      rbl = rblist_concat(rbl, rblist_from_string("Possible completions are:\n"));
      rbl = rblist_concat(rbl, completion_write(cp->matches));
  }
  popup_set(rbl);
  term_display();
}

/*
 * Returns the length of the longest string that is a prefix of
 * both rbl1 and rbl2.
 */
static size_t common_prefix_length(rblist rbl1, rblist rbl2)
{
  size_t i, len = min(rblist_length(rbl1), rblist_length(rbl2));
  for (i = 0; i < len; i++)
    if (rblist_get(rbl1, i) != rblist_get(rbl2, i))
      break;
  return i;
}

static int hcompar(const void *p1, const void *p2)
{
  return rblist_compare(*(const rblist *)p1, *(const rblist *)p2);
}

/*
 * Match completions
 * cp - the completions
 * search - the prefix to search for (not modified).
 * Returns false if `search' is not a prefix of any completion, and true
 * otherwise. The effect on cp is as follows:
 * cp->completions - not modified.
 * cp->matches - replaced with the list of matching completions, sorted.
 * cp->match - replaced with the longest common prefix of the matches, if the
 * function returns true, otherwise not modified.
 *
 * To see the completions in a popup, you should call completion_popup
 * after this method. You may want to call completion_remove_suffix and/or
 * completion_remove_prefix in between to keep the list manageable.
 */
bool completion_try(Completion *cp, rblist search)
{
  size_t fullmatches = 0;

  cp->matches = list_new();
  for (list p = list_first(cp->completions); p != cp->completions; p = list_next(p))
    if (!rblist_ncompare(p->item, search, rblist_length(search))) {
      list_append(cp->matches, p->item);
      if (!rblist_compare(p->item, search))
        ++fullmatches;
    }
  list_sort(cp->matches, hcompar);

  if (list_empty(cp->matches))
    return false;

  cp->match = list_first(cp->matches)->item;
  size_t prefix_len = rblist_length(cp->match);
  for (list p = list_first(cp->matches); p != cp->matches; p = list_next(p))
    prefix_len = min(prefix_len, common_prefix_length(cp->match, p->item));
  cp->match = rblist_sub(cp->match, 0, prefix_len);

  return true;
}

/*
 * Find the last occurrence of character `c' before `before_pos' in
 * `rbl'. Returns the offset into `rbl' of the character after `c', or
 * 0 if not found.
 */
static size_t last_occurrence(rblist rbl, size_t before_pos, int c)
{
  while (before_pos > 0 && rblist_get(rbl, before_pos - 1) != c)
    before_pos--;
  return before_pos;
}

/*
 * If two or more `cp->matches' have a common prefix that is longer than
 * `cp->match' and ends in `_', replaces them with the longest such prefix.
 * Repeats as often as possible.
 */
void completion_remove_suffix(Completion *cp)
{
  if (list_empty(cp->matches))
    return;
  list ans = list_new();
  list p = list_first(cp->matches);
  rblist previous = p->item;
  for (p = list_next(p); p != cp->matches; p = list_next(p)) {
    size_t length = last_occurrence(previous, common_prefix_length(previous, p->item), '_');
    if (length > rblist_length(cp->match))
      previous = rblist_sub(previous, 0, length);
    else {
      list_append(ans, previous);
      previous = p->item;
    }
  }
  cp->matches = list_append(ans, previous);
}

/*
 * Finds the longest prefix of `search' that ends in an underscore, and removes
 * it from all `cp->matches'. Does nothing if there is no such prefix.
 * Returns the length of the removed prefix.
 */
size_t completion_remove_prefix(Completion *cp, rblist search)
{
  size_t pos = last_occurrence(search, rblist_length(search), '_');
  if (pos > 0)
    for (list p = list_first(cp->matches); p != cp->matches; p = list_next(p))
      p->item = rblist_sub(p->item, pos, rblist_length(p->item));
  return pos;
}
