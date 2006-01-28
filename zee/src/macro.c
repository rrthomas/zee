/* Macro facility functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2005 Reuben Thomas.
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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"


static Macro *cur_mp = NULL, *cmd_mp = NULL, *head_mp = NULL;

static Macro *macro_new(void)
{
  Macro *mp = zmalloc(sizeof(Macro));
  mp->keys = vec_new(sizeof(size_t));
  return mp;
}

static void macro_delete(Macro *mp)
{
  if (mp) {
    free(mp->keys);
    free(mp);
  }
}

static void add_macro_key(Macro *mp, size_t key)
{
  vec_item(mp->keys, vec_items(mp->keys), size_t) = key;
}

void add_cmd_to_macro(void)
{
  size_t i;
  assert(cmd_mp);
  for (i = 0; i < vec_items(cmd_mp->keys); i++)
    add_macro_key(cur_mp, vec_item(cmd_mp->keys, i, size_t));
  macro_delete(cmd_mp);
  cmd_mp = NULL;
}

void add_key_to_cmd(size_t key)
{
  if (cmd_mp == NULL)
    cmd_mp = macro_new();

  add_macro_key(cmd_mp, key);
}

void cancel_kbd_macro(void)
{
  macro_delete(cmd_mp);
  macro_delete(cur_mp);
  cmd_mp = cur_mp = NULL;
  thisflag &= ~FLAG_DEFINING_MACRO;
}

DEFUN("start-kbd-macro", start_kbd_macro)
/*+
Record subsequent keyboard input, defining a keyboard macro.
The commands are recorded even as they are executed.
Use C-x ) to finish recording and make the macro available.
Use M-x name-last-kbd-macro to give it a permanent name.
+*/
{
  if (thisflag & FLAG_DEFINING_MACRO) {
    minibuf_error("Already defining a keyboard macro");
    ok = FALSE;
  } else {
    if (cur_mp)
      cancel_kbd_macro();

    minibuf_write("Defining keyboard macro...");

    thisflag |= FLAG_DEFINING_MACRO;
    cur_mp = macro_new();
  }
}
END_DEFUN

DEFUN("end-kbd-macro", end_kbd_macro)
/*+
Finish defining a keyboard macro.
The definition was started by C-x (.
The macro is now available for use via C-x e.
+*/
{
  if (!(thisflag & FLAG_DEFINING_MACRO)) {
    minibuf_error("Not defining a keyboard macro");
    ok = FALSE;
  } else
    thisflag &= ~FLAG_DEFINING_MACRO;
}
END_DEFUN

DEFUN("name-last-kbd-macro", name_last_kbd_macro)
/*+
Assign a name to the last keyboard macro defined.
Argument SYMBOL is the name to define.
The symbol's function definition becomes the keyboard macro string.
Such a "function" cannot be called from Lisp, but it is a valid editor command.
+*/
{
  astr ms;
  Macro *mp;

  if ((ms = minibuf_read("Name for last kbd macro: ", "")) == NULL) {
    minibuf_error("No command name given");
    ok = FALSE;
  } else if (cur_mp == NULL) {
    minibuf_error("No keyboard macro defined");
    ok = FALSE;
  } else {
    if ((mp = get_macro(astr_cstr(ms)))) {
      /* If a macro with this name already exists, update its key list */
      free(mp->keys);
    } else {
      /* Add a new macro to the list */
      mp = macro_new();
      mp->next = head_mp;
      mp->name = zstrdup(astr_cstr(ms));
      head_mp = mp;
    }

    /* Copy the keystrokes from cur_mp. */
    mp->keys = vec_copy(cur_mp->keys);
  }
}
END_DEFUN

/* FIXME: macros should be executed immediately and abort on error;
   they should be stored as a macro list, not a series of
   keystrokes. */
void call_macro(Macro *mp)
{
  size_t i;

  /* The loop termination condition is really i >= 0, but unsigned
     types are always >= 0, and we can't easily get SIZE_T_MAX. */
  for (i = vec_items(mp->keys) - 1; i < vec_items(mp->keys); i--)
    ungetkey(vec_item(mp->keys, i, size_t));
}

DEFUN("call-last-kbd-macro", call_last_kbd_macro)
/*+
Call the last keyboard macro that you defined with C-x (.

To make a macro permanent so you can call it even after
defining others, use M-x name-last-kbd-macro.
+*/
{
  if (cur_mp == NULL) {
    minibuf_error("No kbd macro has been defined");
    ok = FALSE;
  } else
    call_macro(cur_mp);
}
END_DEFUN

/*
 * Free all the macros (used at exit).
 */
void free_macros(void)
{
  Macro *mp, *next;

  macro_delete(cur_mp);

  for (mp = head_mp; mp; mp = next) {
    next = mp->next;
    macro_delete(mp);
  }
}

/*
 * Find a macro given its name.
 */
Macro *get_macro(const char *name)
{
  Macro *mp;
  assert(name);
  for (mp = head_mp; mp; mp = mp->next)
    if (!strcmp(mp->name, name))
      return mp;
  return NULL;
}
