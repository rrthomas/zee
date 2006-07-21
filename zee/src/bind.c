/* Key bindings and extended commands
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2006 Reuben Thomas.
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
#include "extern.h"
#include "vector.h"


static History commands_history;

/*--------------------------------------------------------------------------
 * Key binding
 *--------------------------------------------------------------------------*/

typedef struct {
  size_t key;
  Command cmd;
} Binding;

vector *bindings;               /* Vector of Binding *s */

static Binding *get_binding(size_t key)
{
  size_t i;

  for (i = 0; i < vec_items(bindings); i++)
    if (vec_item(bindings, i, Binding).key == key)
      return (Binding *)vec_index(bindings, i);

  return NULL;
}

void bind_key(size_t key, Command cmd)
{
  Binding *p;

  assert(key != KBD_NOKEY);

  if ((p = get_binding(key)) == NULL) {
    Binding b;

    b.key = key;
    b.cmd = cmd;

    vec_item(bindings, vec_items(bindings), Binding) = b;
  } else
    p->cmd = cmd;
}

void unbind_key(size_t key)
{
  Binding *p;

  if ((p = get_binding(key)))
    vec_shrink(bindings, vec_offset(bindings, p), 1);
}

void init_bindings(void)
{
  bindings = vec_new(sizeof(Binding));
}

void process_key(size_t key)
{
  Binding *p = get_binding(key);

  if (key == KBD_NOKEY)
    return;

  if (p)
    p->cmd(list_new());
  else
    CMDCALL_UINT(edit_insert_character, (int)key);

  /* Only add keystrokes if we're already in macro defining mode
     before the command call, to cope with start-kbd-macro */
  if (lastflag & FLAG_DEFINING_MACRO && thisflag & FLAG_DEFINING_MACRO)
    add_cmd_to_macro();
}

/*
 * Read a command name from the minibuffer.
 */
rblist minibuf_read_command_name(rblist prompt)
{
  rblist ms;
  Completion *cp = completion_new();
  cp->completions = command_list();

  for (;;) {
    ms = minibuf_read_completion(prompt, rblist_empty, cp, &commands_history);

    if (ms == NULL) {
      CMDCALL(edit_select_off);
      return NULL;
    }

    if (rblist_length(ms) == 0) {
      minibuf_error(rblist_from_string("No command name given"));
      return NULL;
    }

    /* Complete partial words if possible */
    if (completion_try(cp, ms))
      ms = cp->match;

    if (get_command(ms) || get_macro(ms)) {
      add_history_element(&commands_history, ms);
      minibuf_clear();        /* Remove any error message */
      break;
    } else {
      minibuf_error(astr_afmt("Undefined command `%r'", ms));
      waitkey(WAITKEY_DEFAULT);
    }
  }

  return ms;
}

DEF(key_bind,
"\
Bind a command to a key chord.\n\
Read key chord and command name, and bind the command to the key\n\
chord.\
")
{
  size_t key = KBD_NOKEY;
  rblist name = NULL;

  ok = false;

  if (list_length(l) > 1) {
    key = strtochord(list_behead(l));
    name = list_behead(l);
  } else {
    rblist as;

    minibuf_write(rblist_from_string("Bind key: "));
    key = getkey();

    as = chordtostr(key);
    name = minibuf_read_command_name(astr_afmt("Bind key %r to command: ", as));
  }

  if (name) {
    Command cmd;

    if ((cmd = get_command(name))) {
      if (key != KBD_NOKEY) {
        bind_key(key, cmd);
        ok = true;
      } else
        minibuf_error(rblist_from_string("Invalid key"));
    } else
      minibuf_error(astr_afmt("No such command `%r'", name));
  }
}
END_DEF

DEF(key_unbind,
"\
Unbind a key.\n\
Read key chord, and unbind it.\
")
{
  size_t key = KBD_NOKEY;

  if (list_length(l) > 0)
    key = strtochord(list_behead(l));
  else {
    minibuf_write(rblist_from_string("Unbind key: "));
    key = getkey();
  }

  unbind_key(key);
}
END_DEF

rblist command_to_binding(Command cmd)
{
  size_t i, n = 0;
  rblist as = rblist_empty;

  for (i = 0; i < vec_items(bindings); i++)
    if (vec_item(bindings, i, Binding).cmd == cmd) {
      size_t key = vec_item(bindings, i, Binding).key;
      rblist binding = chordtostr(key);
      if (n++ != 0)
        as = rblist_concat(as, rblist_from_string(", "));
      as = rblist_concat(as, binding);
    }

  return as;
}

rblist binding_to_command(size_t key)
{
  Binding *p;

  if ((p = get_binding(key)) == NULL) {
    if (key == KBD_RET || key == KBD_TAB || key <= 255)
      return rblist_from_string("self_insert_command");
    else
      return NULL;
  } else
    return get_command_name(p->cmd);
}
