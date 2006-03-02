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
  term_print(astr_sub(as, 0, (ptrdiff_t)min(astr_len(as), term_width())));
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

/*
 * Repeatedly prompts until the user gives one of the answers in
 * cp->completions.
 * Returns NULL if cancelled, otherwise returns the index into cp->completions
 * of the chosen option. Note that cp->completions gets sorted so this index
 * may not be what you expect.
 * FIXME: This is crazy. Return the astr.
 */
static astr minibuf_read_forced(astr prompt, astr errmsg, Completion *cp)
{
  astr as;

  for (;;) {
    as = minibuf_read_completion(prompt, astr_new(""), cp, NULL);
    if (as == NULL)             /* Cancelled. */
      return NULL;

    /* Complete partial words if possible. */
    if (completion_try(cp, as) == COMPLETION_MATCHED) {
      as = astr_dup(cp->match);
      if (astr_len(as) == astr_len(list_first(cp->matches)->item))
        return as;
    }

    minibuf_error(errmsg);
    waitkey(WAITKEY_DEFAULT);
  }
}

/*
 * Forces the user to answer "yes" or "no".
 * Returns -1 for cancelled, otherwise TRUE for "yes" and FALSE for "no".
 * Suggestion: inline? Probably not.
 */
int minibuf_read_yesno(astr prompt)
{
  Completion *cp = completion_new();
  list_append(cp->completions, astr_new("yes"));
  list_append(cp->completions, astr_new("no"));
  astr reply = minibuf_read_forced(prompt, astr_new("Please answer `yes' or `no'."), cp);
  return reply==NULL ? -1 : !astr_cmp(astr_new("yes"), reply);
}

/*
 * Forces the user to answer "true" or "false".
 * Returns -1 for cancelled, otherwise TRUE for "true" and FALSE for "false".
 * Suggestion: inline? Probably not.
 */
int minibuf_read_boolean(astr prompt)
{
  Completion *cp = completion_new();
  list_append(cp->completions, astr_new("true"));
  list_append(cp->completions, astr_new("false"));
  astr reply = minibuf_read_forced(prompt, astr_new("Please answer `true' or `false'."), cp);
  return reply==NULL ? -1 : !astr_cmp(astr_new("true"), reply);
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

  term_print(astr_sub(value, (ptrdiff_t)n,
                         (ptrdiff_t)min(n + width - astr_len(prompt) - margin, astr_len(value))));
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
  CMDCALL(suspend);
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
  return 0;
}

static ptrdiff_t mb_eol(astr as)
{
  return astr_len(as);
}

static ptrdiff_t mb_backward_char(ptrdiff_t i)
{
  if (i > 0)
    --i;
  else
    ding();
  return i;
}

static ptrdiff_t mb_forward_char(ptrdiff_t i, astr as)
{
  if ((size_t)i < astr_len(as))
    ++i;
  else
    ding();
  return i;
}

static void mb_kill_line(ptrdiff_t i, astr *as)
{
  if ((size_t)i < astr_len(*as))
    *as = astr_sub(*as, 0, i);
  else
    ding();
}

static ptrdiff_t mb_backward_delete_char(ptrdiff_t i, astr *as)
{
  if (i > 0) {
    i--;
    *as = astr_cat(astr_sub(*as, 0, i), astr_sub(*as, i + 1, (ptrdiff_t)astr_len(*as)));
  } else
    ding();
  return i;
}

static void mb_delete_char(ptrdiff_t i, astr *as)
{
  if ((size_t)i < astr_len(as))
    *as = astr_cat(astr_sub(*as, 0, i), astr_sub(*as, i + 1, (ptrdiff_t)astr_len(*as)));
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
  return thistab;
}

static int mb_scroll_up(Completion *cp, int thistab, int lasttab)
{
  if (cp == NULL)
    ding();
  else if (cp->flags & COMPLETION_POPPEDUP) {
    popup_scroll_up();
    thistab = lasttab;
  }
  return thistab;
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

static ptrdiff_t mb_complete(Completion *cp, int tab, astr *as, ptrdiff_t *i)
{
  if (cp == NULL)
    ding();
  else {
    if (tab == COMPLETION_MATCHED && cp->flags & COMPLETION_POPPEDUP) {
      popup_scroll_up();
    } else {
      tab = completion_try(cp, *as);
      assert(tab != COMPLETION_NOTCOMPLETING);
      if (tab == COMPLETION_MATCHED) {
        if (astr_cmp(*as, cp->match) != 0)
          tab = COMPLETION_NOTCOMPLETING;
        *as = cp->match;
        *i = astr_len(*as);
      } else
        ding();
    }
  }

  return tab;
}

static ptrdiff_t mb_self_insert(int c, ptrdiff_t i, astr *as)
{
  if (c > 255 || !isprint(c))
    ding();
  else {
    astr bs = astr_new("");
    astr_cat_char(bs, c);
    *as = astr_cat(astr_cat(astr_sub(*as, 0, i), bs), astr_sub(*as, i, (ptrdiff_t)astr_len(*as)));
    i++;
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
  char *s[] = {"", " [No match]", ""};
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
      mb_kill_line(i, &as);
      break;
    case KBD_BS:
      i = mb_backward_delete_char(i, &as);
      break;
    case KBD_DEL:
      mb_delete_char(i, &as);
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
      thistab = mb_complete(cp, lasttab, &as, &i);
      break;
    default:
      i = mb_self_insert(c, i, &as);
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
