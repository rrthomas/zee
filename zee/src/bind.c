/* Key bindings
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2007 Reuben Thomas.
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


typedef struct {
  size_t key;
  Command cmd;
} Binding;

static int bindings;            // Reference to bindings table

static Binding *get_binding(size_t key)
{
  Binding *p = NULL;

  lua_rawgeti(L, LUA_REGISTRYINDEX, bindings);
  lua_pushnumber(L, (lua_Number)key);
  lua_gettable(L, -2);

  if (lua_isuserdata(L, -1)) {
    p = zmalloc(sizeof(Binding));
    p->key = key;
    p->cmd = (Command)lua_touserdata(L, -1);
  }

  lua_pop(L, 2);                // Remove table and value
  
  return p;
}

static void bind_key(size_t key, Command cmd)
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, bindings);
  lua_pushnumber(L, (lua_Number)key);

  if (cmd == NULL)
    lua_pushnil(L);
  else
    lua_pushlightuserdata(L, cmd);

  lua_settable(L, -3);
  lua_pop(L, 1);                // Remove table
}

void init_bindings(void)
{
  lua_newtable(L);
  bindings = luaL_ref(L, LUA_REGISTRYINDEX);
#include "default_bindings.c"
}

void process_key(size_t key)
{
  Binding *p = get_binding(key);

  if (key == KBD_NOKEY)
    return;

  if (p)
    p->cmd(list_new());
  else {
    if (key == KBD_RET)
      key = '\n';
    else if (key == KBD_TAB)
      key = '\t';

    if (key <= 255)
      CMDCALL_UINT(edit_insert_character, (int)key);
    else
      term_beep();
  }

  /* Only add keystrokes if we're already in macro defining mode
     before the command call, to cope with macro_record */
  if (lastflag & FLAG_DEFINING_MACRO && thisflag & FLAG_DEFINING_MACRO)
    add_cmd_to_macro();
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
    minibuf_write(rblist_from_string("Bind key: "));
    key = getkey();
    name = minibuf_read_command_name(rblist_fmt("Bind key %r to command: ", chordtostr(key)));
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
      minibuf_error(rblist_fmt("No such command `%r'", name));
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

  bind_key(key, NULL);
}
END_DEF

rblist command_to_binding(Command cmd)
{
  size_t n = 0;
  rblist rbl = rblist_empty;    // FIXME: Use rbacc

  lua_rawgeti(L, LUA_REGISTRYINDEX, bindings);
  lua_pushnil(L);               // first key
  while (lua_next(L, -2) != 0) {
    if ((Command)lua_touserdata(L, -1) == cmd) {
      size_t key = (size_t)lua_tonumber(L, -2);
      rblist binding = chordtostr(key);
      if (n++ != 0)
        rbl = rblist_concat(rbl, rblist_from_string(", "));
      rbl = rblist_concat(rbl, binding);
    }
    lua_pop(L, 1);        // remove value; keep key for next iteration
  }
  lua_pop(L, 2);                // pop last key and table

  return rbl;
}

rblist binding_to_command(size_t key)
{
  Binding *p;

  if ((p = get_binding(key)) == NULL) {
    if (key == KBD_RET || key == KBD_TAB || key <= 255)
      return rblist_from_string("edit_insert_character");
    else
      return NULL;
  } else
    return get_command_name(p->cmd);
}
