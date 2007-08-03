/* Minibuffer
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2004-2007 Reuben Thomas.
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

#include "main.h"
#include "term.h"
#include "extern.h"


static void minibuf_draw(rblist rbl)
{
  term_move(win.fheight - 1, 0);
  term_print(rblist_sub(rbl, 0, min(rblist_length(rbl), win.fwidth)));
  term_clrtoeol();
}

/*
 * Write the specified string in the minibuffer.
 */
void minibuf_write(rblist rbl)
{
  minibuf_draw(rbl);

  // Redisplay (and leave the cursor in the correct position).
  term_display();
  term_refresh();
}

/*
 * Write the specified error string in the minibuffer and beep.
 */
void minibuf_error(rblist rbl)
{
  minibuf_write(rbl);
  term_beep();

  if (thisflag & FLAG_DEFINING_MACRO)
    cancel_macro_definition();
}

/*
 * Read a string from the minibuffer.
 */
rblist minibuf_read(rblist rbl, rblist value)
{
  return minibuf_read_completion(rbl, value, NULL, NULL);
}

/*
 * Clear the minibuffer.
 */
void minibuf_clear(void)
{
  minibuf_draw(rblist_empty);
}


/*--------------------------------------------------------------------------
 * Minibuffer key action routines
 *--------------------------------------------------------------------------*/

/*
 * Draws the string "<prompt><value>" in the minibuffer and leaves the
 * cursor at offset "offset" within "<value>". If the string is too long
 * to fit on the terminal, various schemes are used to make it fit. First,
 * a scrolling window is used to show just part of the value. Second,
 * characters are chopped off the left of the prompt. If the terminal is
 * narrower than four charaters we give up and corrupt the display a bit.
 */
static void draw_minibuf_read(rblist prompt, rblist value, size_t offset)
{
  rblist rbl = prompt;          // Text to print.
  size_t visible_width = max(3, win.fwidth - rblist_length(prompt) - 2);
  visible_width--; /* Avoid the b.r. corner of the screen for broken
                      terminals and terminal emulators. */
  size_t scroll_pos =
    offset == 0 ? 0 : visible_width * ((offset - 1) / visible_width);
  if (scroll_pos > 0)
    rbl = rblist_concat(rbl, rblist_from_char('$'));

  // Cursor position within `rbl'.
  size_t cursor_pos = rblist_length(rbl) + (offset - scroll_pos);

  rbl = rblist_concat(rbl, rblist_sub(value, scroll_pos, min(rblist_length(value), scroll_pos + visible_width)));
  if (rblist_length(value) > scroll_pos + visible_width)
    rbl = rblist_concat(rbl, rblist_from_char('$'));

  // Handle terminals not wide enough to show "<prompt>$xxx$".
  if (rblist_length(rbl) > win.fwidth) {
    size_t to_lose = rblist_length(rbl) - win.fwidth;
    rbl = rblist_sub(rbl, to_lose, rblist_length(rbl));
    cursor_pos -= to_lose;
  }

  minibuf_draw(rbl);
  term_move(win.fheight - 1, cursor_pos);
  term_refresh();
}

static size_t mb_backward_char(size_t i)
{
  if (i > 0)
    --i;
  else
    term_beep();
  return i;
}

static size_t mb_forward_char(size_t i, rblist rbl)
{
  if ((size_t)i < rblist_length(rbl))
    ++i;
  else
    term_beep();
  return i;
}

static void mb_kill_line(size_t i, rblist *as)
{
  if ((size_t)i < rblist_length(*as))
    *as = rblist_sub(*as, 0, i);
  else
    term_beep();
}

static size_t mb_backward_delete_char(size_t i, rblist *as)
{
  if (i > 0) {
    i--;
    *as = rblist_concat(rblist_sub(*as, 0, i), rblist_sub(*as, i + 1, rblist_length(*as)));
  } else
    term_beep();
  return i;
}

static void mb_delete_char(size_t i, rblist *as)
{
  if (i < rblist_length(*as))
    *as = rblist_concat(rblist_sub(*as, 0, i), rblist_sub(*as, i + 1, rblist_length(*as)));
  else
    term_beep();
}

static void mb_prev_history(History *hp, rblist *as, size_t *_i, rblist *_saved)
{
  size_t i = *_i;
  rblist saved = *_saved;
  if (hp) {
    rblist elem = previous_history_element(hp);
    if (elem) {
      if (!saved)
        saved = *as;

      i = rblist_length(elem);
      *as = elem;
    }
  }
  *_i = i;
  *_saved = saved;
}

static void mb_next_history(History *hp, rblist *as, size_t *_i, rblist *_saved)
{
  size_t i = *_i;
  rblist saved = *_saved;
  if (hp) {
    rblist elem = next_history_element(hp);
    if (elem)
      *as = elem;
    else if (saved) {
      i = rblist_length(saved);
      *as = saved;
      saved = NULL;
    }
  }
  *_i = i;
  *_saved = saved;
}

/*
 * Read a string from the minibuffer using a completion.
 */
rblist minibuf_read_completion(rblist prompt, rblist value, Completion *cp, History *hp)
{
  int c;
  bool ret = false, ok = true;
  size_t i;
  rblist rbl = value, retval = NULL, saved = NULL;

  if (hp)
    prepare_history(hp);

  rblist old_as = NULL;

  for (i = rblist_length(rbl);;) {
    if (cp != NULL && (!old_as || rblist_compare(old_as, rbl))) {
      // Using completions and `rbl' has changed, so display new completions.
      completion_try(cp, rbl);
      completion_remove_suffix(cp);
      completion_remove_prefix(cp, rbl);
      completion_popup(cp);
      old_as = rbl;
    }
    draw_minibuf_read(prompt, rbl, (size_t)i);

    switch (c = getkey()) {
    case KBD_NOKEY:
      break;
    case KBD_CTRL | 'z':
      {
        CMDCALL(file_suspend);
        break;
      }
    case KBD_RET:
      minibuf_clear();
      retval = rbl;
      ret = true;
      break;
    case KBD_CTRL | 'a':
    case KBD_HOME:
      i = 0;
      break;
    case KBD_CTRL | 'e':
    case KBD_END:
      i = rblist_length(rbl);
      break;
    case KBD_CTRL | 'b':
    case KBD_LEFT:
      i = mb_backward_char(i);
      break;
    case KBD_CTRL | 'f':
    case KBD_RIGHT:
      i = mb_forward_char(i, rbl);
      break;
    case KBD_CTRL | 'g':
      minibuf_clear();
      ret = true;
      break;
    case KBD_CTRL | 'k':
      mb_kill_line(i, &rbl);
      break;
    case KBD_BS:
      i = mb_backward_delete_char(i, &rbl);
      break;
    case KBD_DEL:
      mb_delete_char(i, &rbl);
      break;
    case KBD_META | 'v':
    case KBD_PGUP:
      if (cp == NULL)
        term_beep();
      else
        popup_scroll_up();
      break;
    case KBD_CTRL | 'v':
    case KBD_PGDN:
      if (cp == NULL)
        term_beep();
      else
        popup_scroll_down();
      break;
    case KBD_UP:
    case KBD_META | 'p':
      mb_prev_history(hp, &rbl, &i, &saved);
      break;
    case KBD_DOWN:
    case KBD_META | 'n':
      mb_next_history(hp, &rbl, &i, &saved);
      break;
    case KBD_TAB:
      if (cp == NULL || list_length(cp->matches) == 0)
        term_beep();
      else {
        if (rblist_compare(rbl, cp->match) != 0) {
          rbl = cp->match;
          i = rblist_length(rbl);
        } else
          popup_scroll_down_and_loop();
      }
      break;
    default:
      if (c > 255 || !isprint(c))
        term_beep();
      else {
        rbl = rblist_concat(rblist_concat(rblist_sub(rbl, 0, i), rblist_from_char(c)),
                      rblist_sub(rbl, i, rblist_length(rbl)));
        i++;
      }
    }

    if (ret)
      break;
  }

  popup_clear();
  term_display();
  term_refresh();

  return retval;
}

/*
 * Read a command name from the minibuffer.
 */
rblist minibuf_read_command_name(rblist prompt)
{
  static History commands_history;
  rblist ms;
  Completion *cp = completion_new();
  bool ok = false;
  cp->completions = LUA_GLOBALSINDEX; // FIXME: Need to filter out commands

  for (;;) {
    ms = minibuf_read_completion(prompt, rblist_empty, cp, &commands_history);

    if (ms == NULL)
      return NULL;

    if (rblist_length(ms) == 0) {
      minibuf_error(rblist_from_string("No command name given"));
      return NULL;
    }

    // Complete partial words if possible
    if (completion_try(cp, ms))
      ms = cp->match;

    lua_getglobal(L, rblist_to_string(ms));
    if (lua_iscfunction(L, -1)) {
      add_history_element(&commands_history, ms);
      minibuf_clear(); // Remove any error message
      ok = true;
    } else {
      minibuf_error(rblist_fmt("Undefined command `%r'", ms));
      waitkey(WAITKEY_DEFAULT);
    }

    lua_pop(L, -1);
  }

  return ms;
}

rblist minibuf_read_variable_name(rblist msg)
{
  rblist ms;
  Completion *cp = completion_new();

  cp->completions = LUA_GLOBALSINDEX; // FIXME: Need to filter out commands

  for (;;) {
    ms = minibuf_read_completion(msg, rblist_empty, cp, NULL);

    if (ms == NULL)
      return NULL;

    if (rblist_length(ms) == 0) {
      minibuf_error(rblist_from_string("No variable name given"));
      return NULL;
    } else if (get_variable_string(ms) == NULL) {
      minibuf_error(rblist_fmt("There is no variable called `%r'", ms));
      waitkey(WAITKEY_DEFAULT);
    } else {
      minibuf_clear();
      break;
    }
  }

  return ms;
}
