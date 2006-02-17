/* Minibuffer
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2004-2005 Reuben Thomas.
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

#include "main.h"
#include "extern.h"

/* FIXME: \n's mess up the display */
static void term_minibuf_write(astr as)
{
  term_move(term_height() - 1, 0);
  term_print(astr_substr(as, 0, min(astr_len(as), term_width())));
  term_clrtoeol();
}

/*
 * Write the specified string in the minibuffer.
 */
void minibuf_write(astr as)
{
  term_minibuf_write(as);

  /* Redisplay (and leave the cursor in the correct position). */
  term_display();
  term_refresh();
}

/*
 * Write the specified error string in the minibuffer and beep.
 */
void minibuf_error(astr as)
{
  minibuf_write(as);
  ding();
}

/*
 * Read a string from the minibuffer.
 */
astr minibuf_read(astr as, astr value)
{
  return minibuf_read_completion(as, value, NULL, NULL);
}

static int minibuf_read_forced(astr prompt, astr errmsg, Completion *cp)
{
  astr as;

  for (;;) {
    as = minibuf_read_completion(prompt, astr_new(""), cp, NULL);
    if (as == NULL)             /* Cancelled. */
      return -1;
    else {
      list s;
      int i;
      astr bs = astr_dup(as);

      /* Complete partial words if possible. */
      if (completion_try(cp, bs, FALSE) == COMPLETION_MATCHED)
        as = astr_dup(cp->match);

      for (s = list_first(cp->completions), i = 0;
           s != cp->completions;
           s = list_next(s), i++)
        if (astr_cmp(as, s->item) == 0)
          return i;

      minibuf_error(errmsg);
      waitkey(WAITKEY_DEFAULT);
    }
  }
}

int minibuf_read_yesno(astr as)
{
  Completion *cp;
  int retvalue;

  cp = completion_new();
  list_append(cp->completions, astr_new("yes"));
  list_append(cp->completions, astr_new("no"));

  retvalue = minibuf_read_forced(as, astr_new("Please answer `yes' or `no'."), cp);
  if (retvalue != -1) {
    /* The completions may be sorted by the minibuf completion
       routines. */
    if (astr_cmp(list_at(cp->completions, (size_t)retvalue), astr_new("yes")))
      retvalue = TRUE;
    else
      retvalue = FALSE;
  }

  return retvalue;
}

int minibuf_read_boolean(astr as)
{
  int retvalue;
  Completion *cp = completion_new();

  list_append(cp->completions, astr_new("true"));
  list_append(cp->completions, astr_new("false"));

  retvalue = minibuf_read_forced(as, astr_new("Please answer `true' or `false'."), cp);
  if (retvalue != -1) {
    /* The completions may be sorted by the minibuf completion
       routines. */
    if (!strcmp(astr_cstr(list_at(cp->completions, (size_t)retvalue)), "true"))
      retvalue = TRUE;
    else
      retvalue = FALSE;
  }

  return retvalue;
}

/*
 * Clear the minibuffer.
 */
void minibuf_clear(void)
{
  term_minibuf_write(astr_new(""));
}


/*
 * Minibuffer key action routines
 */

static void draw_minibuf_read(astr prompt, astr value, astr match, size_t pointo)
{
  size_t margin = 1, n = 0;
  size_t width = term_width();

  term_minibuf_write(prompt);

  if (astr_len(prompt) + pointo + 1 >= width) {
    margin++;
    term_addch('$');
    n = pointo - pointo % (width - astr_len(prompt) - 2);
  }

  term_print(astr_substr(value, (ptrdiff_t)n,
                         min(width - astr_len(prompt) - margin, astr_len(value) - n)));
  term_print(match);

  if (astr_len(value + n) >= width - astr_len(prompt) - margin) {
    term_move(term_height() - 1, width - 1);
    term_addch('$');
  }

  term_move(term_height() - 1, astr_len(prompt) + margin - 1 +
            pointo % (width - astr_len(prompt) - margin));

  term_refresh();
}

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

static void mb_prev_history(History *hp, astr *as, ptrdiff_t *_i, astr *_saved)
{
  ptrdiff_t i = *_i;
  astr saved = *_saved;
  if (hp) {
    astr elem = previous_history_element(hp);
    if (elem) {
      if (!saved)
        saved = astr_dup(*as);

      i = astr_len(elem);
      *as = astr_dup(elem);
    }
  }
  *_i = i;
  *_saved = saved;
}

static void mb_next_history(History *hp, astr *as, ptrdiff_t *_i, astr *_saved)
{
  ptrdiff_t i = *_i;
  astr saved = *_saved;
  if (hp) {
    astr elem = next_history_element(hp);
    if (elem)
      *as = astr_dup(elem);
    else if (saved) {
      i = astr_len(saved);
      *as = astr_dup(saved);
      saved = NULL;
    }
  }
  *_i = i;
  *_saved = saved;
}

static void mb_complete(Completion *cp, int lasttab, astr *as, int *_thistab, ptrdiff_t *_i)
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
      thistab = completion_try(cp, *as, TRUE);
      assert(thistab != COMPLETION_NOTCOMPLETING);
      switch (thistab) {
      case COMPLETION_NONUNIQUE:
      case COMPLETION_MATCHED:
      case COMPLETION_MATCHEDNONUNIQUE:
        {
          astr bs = astr_new("");
          i = cp->matchsize;
          astr_ncat(bs, astr_cstr(cp->match), cp->matchsize);
          if (astr_cmp(as, bs) != 0)
            thistab = COMPLETION_NOTCOMPLETING;
          *as = bs;
          break;
        }
      case COMPLETION_NOTMATCHED:
        ding();
        break;
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
  else {
    char ch = (char)c;
    astr_nreplace(as, i++, 0, &ch, 1);
  }
  return i;
}

/*
 * Read a string from the minibuffer using a completion.
 */
astr minibuf_read_completion(astr prompt, astr value, Completion *cp, History *hp)
{
  int c, thistab, lasttab = COMPLETION_NOTCOMPLETING, ret = FALSE;
  ptrdiff_t i;
  char *s[] = {"", " [No match]", " [Sole completion]", " [Complete, but not unique]", ""};
  astr as = astr_dup(value), retval = NULL, saved;

  if (hp)
    prepare_history(hp);

  for (i = astr_len(as);;) {
    draw_minibuf_read(prompt, as, astr_new(s[lasttab]), (size_t)i);

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
      mb_prev_history(hp, &as, &i, &saved);
      break;
    case KBD_DOWN:
    case KBD_META | 'n':
      mb_next_history(hp, &as, &i, &saved);
      break;
    case KBD_TAB:
      mb_complete(cp, lasttab, &as, &thistab, &i);
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
