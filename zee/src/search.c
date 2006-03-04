/* Search and replace functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2004 David A. Capello.
   Copyright (c) 2005-2006 Reuben Thomas.
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
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "main.h"
#include "extern.h"
#ifdef HAVE_REGEX_H
#include <regex.h>
#else
#include "regex.h"
#endif


static const char *find_err = NULL;

/* FIXME: Use PCRE instead */
static size_t find_substr(astr s1, astr as2, int bol, int eol, int backward)
{
  struct re_pattern_buffer pattern;
  struct re_registers search_regs;
  size_t ret = SIZE_MAX;
  int index;

  search_regs.num_regs = 1;
  search_regs.start = zmalloc(sizeof(regoff_t));
  search_regs.end = zmalloc(sizeof(regoff_t));

  pattern.translate = NULL;
  pattern.fastmap = NULL;
  pattern.buffer = NULL;
  pattern.allocated = 0;

  find_err = re_compile_pattern(astr_cstr(as2), (int)astr_len(as2), &pattern);
  if (!find_err) {
    pattern.not_bol = !bol;
    pattern.not_eol = !eol;

    if (!backward)
      index = re_search(&pattern, astr_cstr(s1), (int)astr_len(s1), 0, (int)astr_len(s1),
                        &search_regs);
    else
      index = re_search(&pattern, astr_cstr(s1), (int)astr_len(s1), (int)astr_len(s1), -(int)astr_len(s1),
                        &search_regs);

    if (index >= 0) {
      if (!backward)
        ret = search_regs.end[0];
      else
        ret = search_regs.start[0];
    }
  }

  regfree(&pattern);

  return ret;
}

static int search_forward(Line *startp, size_t starto, astr as)
{
  if (astr_len(as) > 0) {
    Line *lp;
    astr sp;

    for (lp = startp, sp = astr_sub(lp->item, (ptrdiff_t)starto, (ptrdiff_t)astr_len(lp->item));
         lp != list_last(buf->lines);
         lp = list_next(lp), sp = lp->item) {
      if (astr_len(sp) > 0) {
        size_t off = find_substr(sp, as, sp == lp->item, TRUE, FALSE);
        if (off != SIZE_MAX) {
          while (buf->pt.p != lp)
            CMDCALL(edit_navigate_next_line);
          buf->pt.o = off;
          return TRUE;
        }
      }
    }
  }

  return FALSE;
}

static int search_backward(Line *startp, size_t starto, astr as)
{
  if (astr_len(as) > 0) {
    Line *lp;
    size_t s1size;

    for (lp = startp, s1size = starto;
         lp != list_first(buf->lines);
         lp = list_prev(lp), s1size = astr_len(lp->item)) {
      astr sp = lp->item;
      if (s1size > 0) {
        size_t off = find_substr(sp, as, TRUE, s1size == astr_len(lp->item), TRUE);
        if (off != SIZE_MAX) {
          while (buf->pt.p != lp)
            CMDCALL(edit_navigate_previous_line);
          buf->pt.o = off;
          return TRUE;
        }
      }
    }
  }

  return FALSE;
}

static astr last_search = NULL;

#define ISEARCH_FORWARD		1
#define ISEARCH_BACKWARD	2

/*
 * Incremental search engine.
 */
static int isearch(int dir)
{
  int c;
  int last = TRUE;
  astr as;
  astr pattern = astr_new("");
  Point start, cur;
  Marker *old_mark;

  assert(buf->mark);
  old_mark = marker_new(buf->mark->pt);

  start = buf->pt;
  cur = buf->pt;

  /* I-search mode. */
  buf->flags |= BFLAG_ISEARCH;

  for (;;) {
    /* Make the minibuf message. */
    as = astr_afmt("%sI-search%s: %s",
              (last ? " " : "Failing "),
              (dir == ISEARCH_FORWARD) ? "" : " backward",
              astr_cstr(pattern));

    /* Regex error. */
    if (find_err) {
      if ((strncmp(find_err, "Premature ", 10) == 0) ||
          (strncmp(find_err, "Unmatched ", 10) == 0) ||
          (strncmp(find_err, "Invalid ", 8) == 0)) {
        find_err = "incomplete input";
      }
      astr_cat(as, astr_afmt(" [%s]", find_err));
      find_err = NULL;
    }

    minibuf_write(as);

    c = getkey();

    if (c == KBD_CANCEL) {
      buf->pt = start;
      thisflag |= FLAG_NEED_RESYNC;
      CMDCALL(edit_select_off);

      /* Restore old mark position. */
      assert(buf->mark);
      remove_marker(buf->mark);

      if (old_mark)
        buf->mark = marker_new(old_mark->pt);
      else
        buf->mark = old_mark;
      break;
    } else if (c == KBD_BS) {
      if (astr_len(pattern) > 0) {
        pattern = astr_sub(pattern, 0, (ptrdiff_t)astr_len(pattern) - 1);
        buf->pt = start;
        thisflag |= FLAG_NEED_RESYNC;
      } else
        ding();
    } else if (c & KBD_CTRL && ((c & 0xff) == 'r' || (c & 0xff) == 's')) {
      /* Invert direction. */
      if ((c & 0xff) == 'r' && dir == ISEARCH_FORWARD)
        dir = ISEARCH_BACKWARD;
      else if ((c & 0xff) == 's' && dir == ISEARCH_BACKWARD)
        dir = ISEARCH_FORWARD;
      if (astr_len(pattern) > 0) {
        /* Find next match. */
        cur = buf->pt;

        /* Save search string. */
        last_search = astr_dup(pattern);
      }
      else if (last_search)
        pattern = astr_dup(last_search);
    } else if (c & KBD_META || c & KBD_CTRL || c > KBD_TAB) {
      if (astr_len(pattern) > 0) {
        /* Save mark. */
        set_mark_to_point();
        buf->mark->pt = start;

        /* Save search string. */
        last_search = astr_dup(pattern);

        minibuf_write(astr_new("Mark saved when search started"));
      } else
        minibuf_clear();
      break;
    } else
      astr_cat_char(pattern, c);

    if (astr_len(pattern) > 0) {
      if (dir == ISEARCH_FORWARD)
        last = search_forward(cur.p, cur.o, pattern);
      else
        last = search_backward(cur.p, cur.o, pattern);
    } else
      last = TRUE;

    if (thisflag & FLAG_NEED_RESYNC)
      resync_display();
  }

  /* done */
  buf->flags &= ~BFLAG_ISEARCH;

  if (old_mark)
    remove_marker(old_mark);

  return TRUE;
}

/* FIXME: If there is a selection, use it to initialise the search pattern. */
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

static int no_upper(astr as)
{
  size_t i;

  for (i = 0; i < astr_len(as); i++)
    if (isupper(*astr_char(as, (ptrdiff_t)i)))
      return FALSE;

  return TRUE;
}

DEF(edit_find_and_replace,
"\
Replace occurrences of a regexp with other text.\n\
As each match is found, the user must type a character saying\n\
what to do with it.\
")
{
  int count = 0, noask = FALSE, exitloop = FALSE, find_no_upper;
  astr find, repl;

  if ((find = minibuf_read(astr_new("Query replace string: "), astr_new(""))) == NULL)
    ok = CMDCALL(edit_select_off);
  else if (astr_len(find) == 0)
    ok = FALSE;
  else {
    find_no_upper = no_upper(find);

    if ((repl = minibuf_read(astr_afmt("Query replace `%s' with: ", astr_cstr(find)), astr_new(""))) == NULL)
      ok = CMDCALL(edit_select_off);
    if (ok) {
      /* Spaghetti code follows... :-( */
      /* FIXME: Can we use minibuf_read_forced? */
      while (search_forward(buf->pt.p, buf->pt.o, find)) {
        if (!noask) {
          int c;
          if (thisflag & FLAG_NEED_RESYNC)
            resync_display();
          for (;;) {
            minibuf_write(astr_afmt("Query replacing `%s' with `%s' (y, n, !, ., q)? ", astr_cstr(find), astr_cstr(repl)));
            c = getkey();
            if (c == KBD_CANCEL || c == KBD_RET || c == ' ' || c == 'y' || c == 'n' ||
                c == 'q' || c == '.' || c == '!')
              goto exitloop;
            minibuf_error(astr_new("Please answer y, n, !, . or q."));
            waitkey(WAITKEY_DEFAULT);
          }
        exitloop:
          minibuf_clear();

          switch (c) {
          case KBD_CANCEL: /* C-g */
            ok = CMDCALL(edit_select_off);
            /* Fall through. */
          case 'q': /* Quit immediately. */
            goto endoffunc;
          case '.': /* Replace and quit. */
            exitloop = TRUE;
            goto replblock;
          case '!': /* Replace all without asking. */
            noask = TRUE;
            goto replblock;
          case ' ': /* Replace. */
          case 'y':
            goto replblock;
            break;
          case 'n': /* Do not replace. */
          case KBD_RET:
          case KBD_DEL:
            goto nextmatch;
          }
        }

      replblock:
        ++count;
        undo_save(UNDO_REPLACE_BLOCK,
                  make_point(buf->pt.n, buf->pt.o - astr_len(find)),
                  astr_len(find), astr_len(repl));
        if (line_replace_text(&buf->pt.p, buf->pt.o - astr_len(find),
                              astr_len(find), repl, find_no_upper))
          buf->flags |= BFLAG_MODIFIED;
        /* FALLTHROUGH */
      nextmatch:
        if (exitloop)
          break;
      }

    endoffunc:
      if (thisflag & FLAG_NEED_RESYNC)
        resync_display();
      term_display();

      minibuf_write(astr_afmt("Replaced %d occurrences", count));
    }
  }
}
END_DEF
