/* Lua to Zee bindings
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

#include <stdbool.h>
#include <lua.h>

#include "main.h"
#include "extern.h"


lua_obj get_variable(const char *key)
{
  lua_obj ret;
  ret.type = LUA_TNIL;

  if (key) {
    lua_getglobal(L, key);
    switch ((ret.type = lua_type(L, -1))) {
    case LUA_TSTRING:
      ret.v.string = lua_tostring(L, -1);
      break;
    case LUA_TNUMBER:
      ret.v.number = lua_tonumber(L, -1);
      break;
    case LUA_TBOOLEAN:
      ret.v.boolean = lua_toboolean(L, -1);
      break;
    }
    lua_pop(L, 1);
  }

  return ret;
}

const char *get_variable_string(const char *key)
{
  lua_obj o = get_variable(key);
  return o.type == LUA_TSTRING ? o.v.string : NULL;
}

int get_variable_number(const char *var)
{
  lua_obj o = get_variable(var);
  return o.type == LUA_TNUMBER ? (int)o.v.number : 0;
}

bool get_variable_bool(const char *var)
{
  lua_obj o = get_variable(var);
  return o.type == LUA_TBOOLEAN ? o.v.boolean : false;
}

DEF(preferences_set_variable,
"\
Set a variable to the specified value.\
")
{
  const char *var, *val = NULL;

  if (lua_gettop(L) > 1) {
    var = lua_tostring(L, -2);
    val = lua_tostring(L, -1);
    lua_pop(L, 2);
  } else if ((var = rblist_to_string(minibuf_read_name(rblist_from_string("Set variable: ")))))
    val = rblist_to_string(minibuf_read(rblist_fmt("Set %r to value: ", var), rblist_empty));

  if (var && val) {
    (void)CLUE_DO(L, rblist_to_string(rblist_fmt("%s = %s", var, val)));
  } else {
    ok = false;
  }
}
END_DEF

/*
 * Register C functions in Lua.
 */
void init_commands(void)
{
  static luaL_Reg cmds[] = {
#define X(cmd_name) \
    {# cmd_name, F_ ## cmd_name},
#include "tbl_funcs.h"
#undef X
    {NULL, NULL}
  };

  lua_pushvalue(L, LUA_GLOBALSINDEX);
  luaL_register(L, NULL, cmds);
}

/*
 * Execute a string as commands
 */
// FIXME: Make a better error message.
bool cmd_eval(rblist s, rblist source)
{
  (void)source;
  return luaL_dostring(L, rblist_to_string(s)) == 0;
}

/*
 * Load a file of Lua, aborting if not found.
 */
void require(char *s)
{
  if (luaL_dofile(L, s)) {
    fprintf(stderr, PACKAGE_NAME " is not properly installed: could not load file `%s'\n%s\n", s, lua_tostring(L, -1));
    die(1);
  }
}
