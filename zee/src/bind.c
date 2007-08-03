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
#include "rbacc.h"


static int bindings;            // Reference to bindings table

static const char *get_binding(size_t key)
{
  const char *s = NULL;

  lua_rawgeti(L, LUA_REGISTRYINDEX, bindings);
  lua_pushnumber(L, (lua_Number)key);
  lua_gettable(L, -2);
  s = lua_tostring(L, -1);
  lua_pop(L, 2); // Remove table and value
  
  return s;
}

static void bind_key(size_t key, rblist cmd)
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, bindings);
  lua_pushnumber(L, (lua_Number)key);
  if (cmd) {
    lua_pushstring(L, rblist_to_string(cmd));
  } else {
    lua_pushnil(L);
  }
  lua_settable(L, -3);
  lua_pop(L, 1); // Remove table
}

void init_bindings(void)
{
  lua_newtable(L);
  bindings = luaL_ref(L, LUA_REGISTRYINDEX);
  cmd_eval(file_read(rblist_from_string(PKGDATADIR "/cua_bindings")), NULL); // FIXME: Build this in using bin2c
}

void process_key(size_t key)
{
  const char *s = get_binding(key);
  bool ok; // FIXME: Use this value

  if (key == KBD_NOKEY) {
    return;
  }

  if (s) {
    lua_getglobal(L, s);
    ok = lua_pcall(L, 0, 1, 0) && lua_toboolean(L, -1);
    lua_pop(L, 1);
  } else {
    if (key == KBD_RET) {
      key = '\n';
    } else if (key == KBD_TAB) {
      key = '\t';
    }

    if (key <= 255) {
      CMDCALL_UINT(edit_insert_character, (int)key);
    } else {
      term_beep();
    }
  }

  /* Only add keystrokes if we're already in macro defining mode
     before the command call, to cope with macro_record */
  if (lastflag & FLAG_DEFINING_MACRO && thisflag & FLAG_DEFINING_MACRO) {
    add_cmd_to_macro();
  }
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

  if (lua_gettop(L) > 1) {
    key = strtochord(rblist_from_string(lua_tostring(L, -2)));
    name = rblist_from_string(lua_tostring(L, -1));
    lua_pop(L, 2);
  } else {
    minibuf_write(rblist_from_string("Bind key: "));
    key = getkey();
    name = minibuf_read_command_name(rblist_fmt("Bind key %r to command: ", chordtostr(key)));
  }

  if (name) {
    if (key != KBD_NOKEY) {
      bind_key(key, name);
    } else {
      minibuf_error(rblist_from_string("Invalid key"));
      ok = false;
    }
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

  if (lua_gettop(L) > 0) {
    key = strtochord(rblist_from_string(lua_tostring(L, -1)));
    lua_pop(L, 1);
  } else {
    minibuf_write(rblist_from_string("Unbind key: "));
    key = getkey();
  }

  if (key != KBD_NOKEY) {
    bind_key(key, NULL);
  } else {
    minibuf_error(rblist_from_string("Invalid key"));
    ok = false;
  }
}
END_DEF

rblist command_to_binding(rblist cmd)
{
  size_t n = 0;
  rbacc rba = rbacc_new();

  lua_rawgeti(L, LUA_REGISTRYINDEX, bindings);
  lua_pushnil(L); // first key
  while (lua_next(L, -2) != 0) {
    if (lua_isstring(L, -1) && (rblist_compare(rblist_from_string(lua_tostring(L, -1)), cmd) == 0)) {
      if (n++ != 0)
        rbacc_add_string(rba, ", ");
      rbacc_add_rblist(rba, chordtostr((size_t)lua_tonumber(L, -2)));
    }
    lua_pop(L, 1); // remove value; keep key for next iteration
  }
  lua_pop(L, 1); // pop table

  return rbacc_to_rblist(rba);
}

rblist binding_to_command(size_t key)
{
  const char *s;
  rblist ret = NULL;

  if ((s = get_binding(key)) == NULL) {
    if (key == KBD_RET || key == KBD_TAB || key <= 255) {
      return rblist_from_string("edit_insert_character");
    }
  } else {
    ret = rblist_from_string(s);
  }

  return ret;
}
