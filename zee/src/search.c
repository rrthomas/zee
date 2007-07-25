/* Search and replace functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2005-2007 Reuben Thomas.
   Copyright (c) 2004 David A. Capello.
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

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <pcre.h>

#include "main.h"
#include "extern.h"


static const char *find_err;

static size_t find_substr(rblist as1, rblist as2, bool bol, bool eol, bool backward)
{
  pcre *pattern;
  size_t ret = SIZE_MAX;
  int ovector[3], err_offset;

  if ((pattern = pcre_compile(rblist_to_string(as2), get_variable_bool(rblist_from_string("caseless_search")) ? PCRE_CASELESS : 0, &find_err, &err_offset, NULL))) {
    int options = 0;
    int index = 0;

    if (!bol)
      options |= PCRE_NOTBOL;
    if (!eol)
      options |= PCRE_NOTEOL;

    if (!backward)
      index = pcre_exec(pattern, NULL, rblist_to_string(as1), (int)rblist_length(as1), 0,
                        options, ovector, 3);
    else {
      for (int i = (int)rblist_length(as1); i >= 0; i--) {
        index = pcre_exec(pattern, NULL, rblist_to_string(as1), (int)rblist_length(as1), i,
                          options | PCRE_ANCHORED, ovector, 3);
      }
    }
    
    if (index >= 0) {
      if (!backward)
        ret = ovector[1];
      else
        ret = ovector[0];
    }

    pcre_free(pattern);
  }

  return ret;
}

static bool search_forward(Point start, rblist as)
{
  if (rblist_length(as) > 0) {
    // FIXME: BOL should not always be true!
    size_t off = find_substr(rblist_sub(buf->lines, rblist_line_to_start_pos(buf->lines, start.n) + start.o, rblist_length(buf->lines)), as, true, true, false);
    if (off != SIZE_MAX) {
      size_t n = rblist_pos_to_line(buf->lines, off);
      while (buf->pt.n != n)
        CMDCALL(move_next_line);
      buf->pt.o = off - rblist_line_to_start_pos(buf->lines, n);
      return true;
    }
  }

  return false;
}

static bool search_backward(Point start, rblist as)
{
  if (rblist_length(as) > 0) {
    // FIXME: EOL should not always be true!
    size_t off = find_substr(rblist_sub(buf->lines, rblist_line_to_start_pos(buf->lines, start.n) + start.o, rblist_length(buf->lines)), as, true, true, true);
    if (off != SIZE_MAX) {
      size_t n = rblist_pos_to_line(buf->lines, off);
      while (buf->pt.n != n)
        CMDCALL(move_previous_line);
      buf->pt.o = off - rblist_line_to_start_pos(buf->lines, n);
      return true;
    }
  }

  return false;
}

static rblist last_search = NULL;

#define ISEARCH_FORWARD		1
#define ISEARCH_BACKWARD	2

/*
 * Incremental search engine.
 *
 * FIXME: Once the search is underway, "find next" is hard-wired to C-s.
 * Having it hard-wired is obviously broken, but something neutral like RET
 * would be better.
 * The proposed meaning of ESC obviates the current behaviour of RET.
 */
static bool isearch(int dir)
{
  assert(buf->mark);
  Marker *old_mark = marker_new(buf->mark->pt);

  Point start = buf->pt;
  Point cur = buf->pt;

  buf->flags |= BFLAG_ISEARCH;

  bool last = true;
  rblist pattern = rblist_empty;
  for (;;) {
    // Make the minibuf message.
    rblist as = rblist_fmt("%sI-search%s: %r",
                          (last ? "" : "Failing "),
                          (dir == ISEARCH_FORWARD) ? "" : " backward",
                          pattern);

    // Regex error.
    if (find_err) {
      as = rblist_fmt("%r [%s]", as, find_err);
      find_err = NULL;
    }

    minibuf_write(as);

    int c = getkey();
    if (c == (KBD_CTRL | 'g')) {
      buf->pt = start;
      CMDCALL(edit_select_off);

      // Restore old mark position.
      assert(buf->mark);
      remove_marker(buf->mark);

      if (old_mark)
        buf->mark = marker_new(old_mark->pt);
      else
        buf->mark = old_mark;
      break;
    } else if (c == KBD_BS) {
      if (rblist_length(pattern) > 0) {
        pattern = rblist_sub(pattern, 0, rblist_length(pattern) - 1);
        buf->pt = start;
      } else
        ding();
    } else if (c & KBD_CTRL && ((c & 0xff) == 'r' || (c & 0xff) == 's')) {
      // Invert direction.
      if ((c & 0xff) == 'r' && dir == ISEARCH_FORWARD)
        dir = ISEARCH_BACKWARD;
      else if ((c & 0xff) == 's' && dir == ISEARCH_BACKWARD)
        dir = ISEARCH_FORWARD;
      if (rblist_length(pattern) > 0) {
        // Find next match.
        cur = buf->pt;

        // Save search string.
        last_search = pattern;
      } else if (last_search)
        pattern = last_search;
    } else if (c & KBD_META || c & KBD_CTRL || c > KBD_TAB) {
      if (rblist_length(pattern) > 0) {
        // Save search string.
        last_search = pattern;

        buf->mark->pt = start;
        minibuf_write(rblist_from_string("Mark saved when search started"));
      } else
        minibuf_clear();
      break;
    } else
      pattern = rblist_append(pattern, c);

    if (rblist_length(pattern) > 0) {
      if (dir == ISEARCH_FORWARD)
        last = search_forward(cur, pattern);
      else
        last = search_backward(cur, pattern);
    } else
      last = true;

    resync_display();
  }

  // done
  buf->flags &= ~BFLAG_ISEARCH;

  if (old_mark)
    remove_marker(old_mark);

  return true;
}

DEF(edit_find,
"\
Do incremental search forward for regular expression.\n\
As you type characters, they add to the search string and are found.\n\
Type return to exit, leaving point at location found.\n\
Type C-s to search again forward, C-r to search again backward.\n\
C-g when search is successful aborts and moves point to starting point.\
")
{
  ok = isearch(ISEARCH_FORWARD);
}
END_DEF

DEF(edit_find_backward,
"\
Do incremental search backward for regular expression.\n\
As you type characters, they add to the search string and are found.\n\
Type return to exit, leaving point at location found.\n\
Type C-r to search again backward, C-s to search again forward.\n\
C-g when search is successful aborts and moves point to starting point.\
")
{
  ok = isearch(ISEARCH_BACKWARD);
}
END_DEF

static bool no_upper(rblist as)
{
  RBLIST_FOR(c, as)
    if (isupper(c))
      return false;
  RBLIST_END

  return true;
}

// FIXME: Make edit_replace run on selection.
DEF_ARG(edit_replace,
"\
Replace the next occurrence of a regexp with other text.\n\
",
STR(find, "Replace string: ")
STR(repl, rblist_to_string(rblist_fmt("Replace `%r' with: ", find))))
{
  if (ok) {
    bool find_no_upper = no_upper(find) && get_variable_bool(rblist_from_string("case_replace"));
    if (search_forward(buf->pt, find)) {
      undo_save(UNDO_REPLACE_BLOCK,
                make_point(buf->pt.n, buf->pt.o - rblist_length(find)),
                rblist_length(find), rblist_length(repl));
      if (line_replace_text(buf->pt.n, buf->pt.o - rblist_length(find),
                            rblist_length(find), repl, find_no_upper))
        buf->flags |= BFLAG_MODIFIED;
    }
  }
}
END_DEF

DEF_ARG(edit_replace_all,
"\
Replace all occurrences of a regexp with other text from the cursor\n\
to end of the buffer.\n\
",
STR(find, "Replace string: ")
STR(repl, rblist_to_string(rblist_fmt("Replace `%r' with: ", find))))
{
  if (ok)
    while (F_edit_replace(list_append(list_append(list_new(), find), repl)))
      ;
}
END_DEF
