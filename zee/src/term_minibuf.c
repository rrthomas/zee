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
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

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

  term_addnstr(value + n, min(width - prompt_len - margin, strlen(value) - n));
  term_addnstr(match, strlen(match));

  if (strlen(value + n) >= width - prompt_len - margin) {
    term_move(term_height() - 1, width - 1);
    term_addch('$');
  }

  term_move(term_height() - 1, prompt_len + margin - 1 +
            pointo % (width - prompt_len - margin));

  term_refresh();
}

static astr vminibuf_read(const char *prompt, const char *value,
                           Completion *cp, History *hp)
{
  int c, thistab, lasttab = -1;
  size_t prompt_len = strlen(prompt);
  ptrdiff_t i;
  char *s, *saved = NULL;
  astr as = astr_new();

  i = strlen(value);
  astr_cpy_cstr(as, value);

  for (;;) {
    switch (lasttab) {
    case COMPLETION_MATCHEDNONUNIQUE:
      s = " [Complete, but not unique]";
      break;
    case COMPLETION_NOTMATCHED:
      s = " [No match]";
      break;
    case COMPLETION_MATCHED:
      s = " [Sole completion]";
      break;
    default:
      s = "";
    }
    draw_minibuf_read(prompt, astr_cstr(as), prompt_len, s, (size_t)i);

    thistab = -1;

    switch (c = getkey()) {
    case KBD_NOKEY:
      break;
    case KBD_CTL | 'z':
      FUNCALL(suspend);
      break;
    case KBD_RET:
      term_move(term_height() - 1, 0);
      term_clrtoeol();
      if (saved)
        free(saved);
      return as;
    case KBD_CANCEL:
      term_move(term_height() - 1, 0);
      term_clrtoeol();
      if (saved)
        free(saved);
      return NULL;
    case KBD_CTL | 'a':
    case KBD_HOME:
      i = 0;
      break;
    case KBD_CTL | 'e':
    case KBD_END:
      i = astr_len(as);
      break;
    case KBD_CTL | 'b':
    case KBD_LEFT:
      if (i > 0)
        --i;
      else
        ding();
      break;
    case KBD_CTL | 'f':
    case KBD_RIGHT:
      if ((size_t)i < astr_len(as))
        ++i;
      else
        ding();
      break;
    case KBD_CTL | 'k':
      if ((size_t)i < astr_len(as))
        astr_truncate(as, i);
      else
        ding();
      break;
    case KBD_BS:
      if (i > 0)
        astr_remove(as, --i, 1);
      else
        ding();
      break;
    case KBD_DEL:
      if ((size_t)i < astr_len(as))
        astr_remove(as, i, 1);
      else
        ding();
      break;
    case KBD_META | 'v':
    case KBD_PGUP:
      if (cp == NULL) {
        ding();
        break;
      }

      if (cp->fl_poppedup) {
        completion_scroll_down();
        thistab = lasttab;
      }
      break;
    case KBD_CTL | 'v':
    case KBD_PGDN:
      if (cp == NULL) {
        ding();
        break;
      }

      if (cp->fl_poppedup) {
        completion_scroll_up();
        thistab = lasttab;
      }
      break;
    case KBD_UP:
    case KBD_META | 'p':
      if (hp) {
        const char *elem = previous_history_element(hp);
        if (elem) {
          if (!saved)
            saved = zstrdup(astr_cstr(as));

          i = strlen(elem);
          astr_cpy_cstr(as, elem);
        }
      }
      break;
    case KBD_DOWN:
    case KBD_META | 'n':
      if (hp) {
        const char *elem = next_history_element(hp);
        if (elem) {
          i = strlen(elem);
          astr_cpy_cstr(as, elem);
        }
        else if (saved) {
          i = strlen(saved);
          astr_cpy_cstr(as, saved);

          free(saved);
          saved = NULL;
        }
      }
      break;
    case KBD_TAB:
    got_tab:
      if (cp == NULL) {
        ding();
        break;
      }

      if (lasttab != -1 && lasttab != COMPLETION_NOTMATCHED
          && cp->fl_poppedup) {
        completion_scroll_up();
        thistab = lasttab;
      } else {
        astr bs = astr_new();
        astr_cpy(bs, as);
        thistab = completion_try(cp, bs, TRUE);
        astr_delete(bs);
        switch (thistab) {
        case COMPLETION_NONUNIQUE:
        case COMPLETION_MATCHED:
        case COMPLETION_MATCHEDNONUNIQUE:
          i = cp->matchsize;
          if (cp->fl_dir)
            i += astr_len(cp->path);
          bs = astr_new();
          if (cp->fl_dir)
            astr_cat(bs, cp->path);
          astr_ncat(bs, cp->match, cp->matchsize);
          if (astr_cmp(as, bs) != 0)
            thistab = -1;
          astr_cpy(as, bs);
          astr_delete(bs);
          break;
        case COMPLETION_NOTMATCHED:
          ding();
        }
      }
      break;
    case ' ':
      if (cp != NULL && !cp->fl_space)
        goto got_tab;
      /* FALLTHROUGH */
    default:
      if (c > 255 || !isprint(c)) {
        ding();
        break;
      }
      astr_insert_char(as, i++, c);
    }

    lasttab = thistab;
  }
}

astr term_minibuf_read(const char *prompt, const char *value,
                        Completion *cp, History *hp)
{
  Window *wp, *old_wp = cur_wp;
  astr as;

  if (hp)
    prepare_history(hp);

  as = vminibuf_read(prompt, value, cp, hp);

  if (cp != NULL && cp->fl_poppedup && (wp = find_window("*Completions*")) != NULL) {
    set_current_window(wp);
    if (cp->fl_close)
      FUNCALL(window_close);
    else if (cp->old_bp)
      switch_to_buffer(cp->old_bp);
    set_current_window(old_wp);
  }

  return as;
}
