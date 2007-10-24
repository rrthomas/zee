/* Completion facility functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2006-2007 Reuben Thomas.
   Copyright (c) 2006 Alistair Turnbull.
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
#include <string.h>

#include "config.h"
#include "main.h"
#include "extern.h"
#include "rbacc.h"


/*
 * Allocate a new completion structure.
 */
Completion *completion_new(void)
{
  return zmalloc(sizeof(Completion));
}

/*
 * Write the rblists in `l' in a set of columns. The width of the
 * columns is chosen to be big enough for the longest rblist, with a
 * COLUMN_GAP-character gap between each column.
 */
#define COLUMN_GAP 5
static rblist completion_write(int l)
{
  size_t maxlen = 0;
  for (size_t p = 1; p <= list_length(l); p++)
    maxlen = max(maxlen, strlen(list_get_string(l, p)));
  maxlen += COLUMN_GAP;
  size_t numcols = (win.ewidth + COLUMN_GAP - 1) / maxlen;

  size_t col = 0;
  rbacc rba = rbacc_new();
  for (size_t p = 1; p <= list_length(l); p++) {
    if (col >= numcols) {
      col = 0;
      rbacc_add_char(rba, '\n');
    }
    rbacc_add_string(rba, list_get_string(l, p));
    if (++col < numcols)
      for (size_t i = maxlen - strlen(list_get_string(l, p)); i > 0; i--)
        rbacc_add_char(rba, ' ');
  }

  return rbacc_to_rblist(rba);
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
      rbl = rblist_concat(rbl, rblist_fmt("Sole completion: %s",
                                          list_get_string(cp->matches, 1)));
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
  lua_pushnil(L); // initial key
  while (lua_next(L, cp->completions) != 0) {
    lua_pop(L, 1); // remove value; keep key for next iteration
    if (lua_isstring(L, -1)) {
      rblist rbl = rblist_from_string(lua_tostring(L, -1));
      if (!rblist_ncompare(rbl, search, rblist_length(search))) {
        list_set_string(cp->matches, list_length(cp->matches) + 1, lua_tostring(L, -1));
        if (!rblist_compare(rbl, search))
          ++fullmatches;
      }
    }
  }

  if (list_length(cp->matches) == 0)
    return false;

  // FIXME: sort(cp->matches);
  cp->match = rblist_from_string(list_get_string(cp->matches, 1));
  size_t prefix_len = rblist_length(cp->match);
  for (size_t p = 1; p <= list_length(cp->matches); p++)
    prefix_len = min(prefix_len, common_prefix_length(cp->match, rblist_from_string(list_get_string(cp->matches, p))));
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
  if (list_length(cp->matches) == 0)
    return;
  int ans = list_new();
  rblist previous = rblist_from_string(list_get_string(cp->matches, 1));
  for (size_t p = 2; p <= list_length(cp->matches); p++) {
    size_t length = last_occurrence(previous, common_prefix_length(previous, rblist_from_string(list_get_string(cp->matches, p))), '_');
    if (length > rblist_length(cp->match))
      previous = rblist_sub(previous, 0, length);
    else {
      list_set_string(ans, list_length(ans) + 1, rblist_to_string(previous));
      previous = rblist_from_string(list_get_string(cp->matches, p));
    }
  }
  list_set_string(ans, list_length(ans) + 1, rblist_to_string(previous));
  list_free(cp->matches);
  cp->matches = ans;
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
    for (size_t p = 1; p <= list_length(cp->matches); p++)
      list_set_string(cp->matches, p, list_get_string(cp->matches, p) + pos);
  return pos;
}
