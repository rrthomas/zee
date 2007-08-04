/* Command parser and executor
   Copyright (c) 2005-2007 Reuben Thomas.
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

#include <stdbool.h>
#include <lua.h>

#include "config.h"
#include "main.h"
#include "extern.h"


/*
 * Register C functions in Lua.
 */
void init_commands(void)
{
  static luaL_Reg cmds[] = {
#define X(cmd_name, doc)                        \
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
