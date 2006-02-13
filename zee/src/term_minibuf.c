/* Minibuffer handling
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2005 Reuben Thomas.
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

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"

void term_minibuf_write(const char *s)
{
  size_t i;

  term_move(term_height() - 1, 0);
  for (i = 0; *s != '\0' && i < term_width(); s++, i++)
    term_addch(*s);
  term_clrtoeol();
}

static void draw_minibuf_read(const char *prompt, const char *value,
                              size_t prompt_len, char *match, size_t pointo)
{
  size_t margin = 1, n = 0;
  size_t width = term_width();

  term_minibuf_write(prompt);

  if (prompt_len + pointo + 1 >= width) {
    margin++;
    term_addch('$');
    n = pointo - pointo % (width - prompt_len - 2);
  }

  term_nprint(min(width - prompt_len - margin, strlen(value) - n), value + n);
  term_print(match);

  if (strlen(value + n) >= width - prompt_len - margin) {
    term_move(term_height() - 1, width - 1);
    term_addch('$');
  }

  term_move(term_height() - 1, prompt_len + margin - 1 +
            pointo % (width - prompt_len - margin));

  term_refresh();
}


/*
 * Minibuffer key action routines
 */

static void mb_suspend(void)
{
  FUNCALL(suspend);
}

static void mb_return(void)
{
  term_move(term_height() - 1, 0);
  term_clrtoeol();
}

static void mb_cancel(void)
{
  term_move(term_height() - 1, 0);
  term_clrtoeol();
}

static ptrdiff_t mb_bol(void)
{
  ptrdiff_t i;
  i = 0;
  return(i);
}

static ptrdiff_t mb_eol(astr as)
{
  ptrdiff_t i;
  i = astr_len(as);
  return(i);
}

static ptrdiff_t mb_backward_char(ptrdiff_t i)
{
  if (i > 0)
    --i;
  else
    ding();
  return(i);
}

static ptrdiff_t mb_forward_char(ptrdiff_t i, astr as)
{
  if ((size_t)i < astr_len(as))
    ++i;
  else
    ding();
  return(i);
}

static void mb_kill_line(ptrdiff_t i, astr as)
{
  if ((size_t)i < astr_len(as))
    astr_truncate(as, i);
  else
    ding();
}

static ptrdiff_t mb_backward_delete_char(ptrdiff_t i, astr as)
{
  if (i > 0)
    astr_remove(as, --i, 1);
  else
    ding();
  return(i);
}

static void mb_delete_char(ptrdiff_t i, astr as)
{
  if ((size_t)i < astr_len(as))
    astr_remove(as, i, 1);
  else
    ding();
}

static int mb_scroll_down(Completion *cp, int thistab, int lasttab)
{
  if (cp == NULL)
    ding();
  else if (cp->flags & COMPLETION_POPPEDUP) {
    popup_scroll_down();
    thistab = lasttab;
  }
  return(thistab);
}

static int mb_scroll_up(Completion *cp, int thistab, int lasttab)
{
  if (cp == NULL)
    ding();
  else if (cp->flags & COMPLETION_POPPEDUP) {
    popup_scroll_up();
    thistab = lasttab;
  }
  return(thistab);
}

static void mb_prev_history(History *hp, astr as, ptrdiff_t *_i, char **_saved)
{
  ptrdiff_t i = *_i;
  char *saved = *_saved;
  if (hp) {
    const char *elem = previous_history_element(hp);
    if (elem) {
      if (!saved)
        saved = zstrdup(astr_cstr(as));

      i = strlen(elem);
      astr_cpy_cstr(as, elem);
    }
  }
  *_i = i;
  *_saved = saved;
}

static void mb_next_history(History *hp, astr as, ptrdiff_t *_i, char **_saved)
{
  ptrdiff_t i = *_i;
  char *saved = *_saved;
  if (hp) {
    const char *elem = next_history_element(hp);
    if (elem) {
      i = strlen(elem);
      astr_cpy_cstr(as, elem);
    }
    else if (saved) {
      i = strlen(saved);
      astr_cpy_cstr(as, saved);
      saved = NULL;
    }
  }
  *_i = i;
  *_saved = saved;
}

static void mb_complete(Completion *cp, int lasttab, astr as, int *_thistab, ptrdiff_t *_i)
{
  int thistab = *_thistab;
  ptrdiff_t i = *_i;
  if (cp == NULL)
    ding();
  else {
    if (lasttab != COMPLETION_NOTCOMPLETING &&
        lasttab != COMPLETION_NOTMATCHED &&
        cp->flags & COMPLETION_POPPEDUP) {
      popup_scroll_up();
      thistab = lasttab;
    } else {
      astr bs = astr_new();
      astr_cpy(bs, as);
      thistab = completion_try(cp, bs, TRUE);
      assert(thistab != COMPLETION_NOTCOMPLETING);
      switch (thistab) {
      case COMPLETION_NONUNIQUE:
      case COMPLETION_MATCHED:
      case COMPLETION_MATCHEDNONUNIQUE:
        i = cp->matchsize;
        bs = astr_new();
        astr_ncat(bs, cp->match, cp->matchsize);
        if (astr_cmp(as, bs) != 0)
          thistab = COMPLETION_NOTCOMPLETING;
        astr_cpy(as, bs);
        break;
      case COMPLETION_NOTMATCHED:
        ding();
      }
    }
  }
  *_thistab = thistab;
  *_i = i;
}

static ptrdiff_t mb_self_insert(int c, ptrdiff_t i, astr as)
{
  if (c > 255 || !isprint(c))
    ding();
  else
    astr_insert_char(as, i++, c);
  return i;
}

/*
 * Read a string from the minibuffer.
 */
astr term_minibuf_read(const char *prompt, const char *value, Completion *cp, History *hp)
{
  int c, thistab, lasttab = COMPLETION_NOTCOMPLETING, ret = FALSE;
  ptrdiff_t i;
  char *s[] = {"", " [No match]", " [Sole completion]", " [Complete, but not unique]", ""};
  char *saved = NULL;
  astr as = astr_new(), retval = NULL;

  if (hp)
    prepare_history(hp);

  i = strlen(value);
  astr_cpy_cstr(as, value);

  for (;;) {
    draw_minibuf_read(prompt, astr_cstr(as), strlen(prompt), s[lasttab], (size_t)i);

    thistab = COMPLETION_NOTCOMPLETING;

    switch (c = getkey()) {
    case KBD_NOKEY:
      break;
    case KBD_CTRL | 'z':
      mb_suspend();
      break;
    case KBD_RET:
      mb_return();
      retval = as;
      ret = TRUE;
      break;
    case KBD_CANCEL:
      mb_cancel();
      ret = TRUE;
      break;
    case KBD_CTRL | 'a':
    case KBD_HOME:
      i = mb_bol();
      break;
    case KBD_CTRL | 'e':
    case KBD_END:
      i = mb_eol(as);
      break;
    case KBD_CTRL | 'b':
    case KBD_LEFT:
      i = mb_backward_char(i);
      break;
    case KBD_CTRL | 'f':
    case KBD_RIGHT:
      i = mb_forward_char(i, as);
      break;
    case KBD_CTRL | 'k':
      mb_kill_line(i, as);
      break;
    case KBD_BS:
      i = mb_backward_delete_char(i, as);
      break;
    case KBD_DEL:
      mb_delete_char(i, as);
      break;
    case KBD_META | 'v':
    case KBD_PGUP:
      thistab = mb_scroll_down(cp, thistab, lasttab);
      break;
    case KBD_CTRL | 'v':
    case KBD_PGDN:
      thistab = mb_scroll_up(cp, thistab, lasttab);
      break;
    case KBD_UP:
    case KBD_META | 'p':
      mb_prev_history(hp, as, &i, &saved);
      break;
    case KBD_DOWN:
    case KBD_META | 'n':
      mb_next_history(hp, as, &i, &saved);
      break;
    case KBD_TAB:
      mb_complete(cp, lasttab, as, &thistab, &i);
      break;
    default:
      i = mb_self_insert(c, i, as);
    }

    lasttab = thistab;
    if (ret)
      break;
  }

  if (cp && cp->flags & COMPLETION_POPPEDUP) {
    popup_clear();
    term_refresh();
  }

  return retval;
}
