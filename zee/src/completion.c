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
  astr as = astr_new("");
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
 * Pop up the completion window, showing cp->matches. If there are no
 * completions, the popup just says "No completions". If there is exactly
 * one completion the popup says "Sole completion: %s".
 */
void completion_popup(Completion *cp)
{
  assert(cp);
  assert(cp->matches);
  astr as = astr_new("Completions\n\n");
  switch (list_length(cp->matches)) {
    case 0:
      astr_cat(as, astr_new("No completions"));
      break;
    case 1:
      astr_cat(as, astr_new("Sole completion: "));
      astr_cat(as, list_first(cp->matches)->item);
      break;
    default:
      astr_cat(as, astr_new("Possible completions are:\n"));
      astr_cat(as, completion_write(cp->matches));
  }
  popup_set(as);
  term_display();
}

/*
 * Returns the length of the longest string that is a prefix of
 * both as and bs.
 */
static size_t common_prefix_length(astr as, astr bs)
{
  size_t len = min(astr_len(as), astr_len(bs));
  for (size_t i = 0; i < len; i++)
    if (*astr_char(as, (ptrdiff_t)i) != *astr_char(bs, (ptrdiff_t)i))
      return i;
  return len;
}

static int hcompar(const void *p1, const void *p2)
{
  return astr_cmp(*(const astr *)p1, *(const astr *)p2);
}

/*
 * Match completions
 * cp - the completions
 * search - the prefix to search for (not modified).
 * Returns FALSE if `search' is not a prefix of any completion, and TRUE
 * otherwise. The effect on cp is as follows:
 * cp->completions - not modified.
 * cp->matches - replaced with the list of matching completions, sorted.
 * cp->match - replaced with the longest common prefix of the matches, if the
 * function returns TRUE, otherwise not modified.
 *
 * To see the completions in a popup, you should call completion_popup
 * after this method. You may want to call completion_remove_suffix and/or
 * completion_remove_prefix in between to keep the list manageable.
 */
int completion_try(Completion *cp, astr search)
{
  size_t fullmatches = 0;

  cp->matches = list_new();
  for (list p = list_first(cp->completions); p != cp->completions; p = list_next(p))
    if (!astr_ncmp(p->item, search, astr_len(search))) {
      list_append(cp->matches, p->item);
      if (!astr_cmp(p->item, search))
        ++fullmatches;
    }
  list_sort(cp->matches, hcompar);

  if (list_empty(cp->matches))
    return FALSE;

  cp->match = list_first(cp->matches)->item;
  size_t prefix_len = astr_len(cp->match);
  for (list p = list_first(cp->matches); p != cp->matches; p = list_next(p))
    prefix_len = min(prefix_len, common_prefix_length(cp->match, p->item));
  cp->match = astr_sub(cp->match, 0, (ptrdiff_t)prefix_len);

  return TRUE;
}

/*
 * Tests whether there was an exact match.
 * Not currently used, but it's worth remembering how to do it.
 */
int completion_is_exact(Completion *cp, astr search)
{
  return
    !list_empty(cp->matches) &&
    astr_len(search) == astr_len(list_first(cp->matches)->item);
}

/*
 * Find the last occurrence of character 'c' before 'before_pos' in 'as'.
 * Returns the offset into 'as' of the character after 'c', or '0' if not found.
 */
static size_t last_occurrence(astr as, size_t before_pos, int c)
{
  while (before_pos > 0 && *astr_char(as, (ptrdiff_t)before_pos - 1) != c)
    before_pos--;
  return before_pos;
}

/*
 * If two or more 'cp->matches' have a common prefix that is longer than
 * 'cp->match' and ends in '_', replaces them with the longest such prefix.
 * Repeats as often as possible.
 */
void completion_remove_suffix(Completion *cp)
{
  if (list_empty(cp->matches))
    return;
  list ans = list_new();
  list p = list_first(cp->matches);
  astr previous = p->item;
  for (p = list_next(p); p != cp->matches; p = list_next(p)) {
    size_t length = last_occurrence(previous, common_prefix_length(previous, p->item), '_');
    if (length > astr_len(cp->match))
      previous = astr_sub(previous, 0, (ptrdiff_t)length);
    else {
      list_append(ans, previous);
      previous = p->item;
    }
  }
  cp->matches = list_append(ans, previous);
}

/*
 * Finds the longest prefix of 'search' that ends in an underscore, and removes
 * it from all 'cp->matches'. Does nothing if there is no such prefix.
 * Returns the length of the removed prefix.
 */
size_t completion_remove_prefix(Completion *cp, astr search)
{
  size_t pos = last_occurrence(search, astr_len(search), '_');
  if (pos > 0)
    for (list p = list_first(cp->matches); p != cp->matches; p = list_next(p))
      p->item = astr_sub(p->item, (ptrdiff_t)pos, (ptrdiff_t)astr_len(p->item));
  return pos;
}
