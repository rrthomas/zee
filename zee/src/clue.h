/* C-Lua integration
   Copyright (c) 2007 Reuben Thomas.
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

#include <assert.h>
#include <lua.h>
#include <lauxlib.h>

#define CLUE_DECLS                              \
  lua_State *L

#define CLUE_INIT                               \
  assert(L = luaL_newstate());                  \
  luaL_openlibs(L)
  
#define CLUE_IMPORT(cexp, lvar, ty)             \
  do {                                          \
    lua_push ## ty(L, cexp);                    \
    lua_setglobal(L, #lvar);                    \
  } while (0)

#define CLUE_IMPORT_REF(cexp, lvar)             \
  do {                                          \
    lua_rawgeti(L, LUA_REGISTRYINDEX, cexp);    \
    lua_setglobal(L, #lvar);                    \
  } while (0)

#define CLUE_EXPORT(cvar, lvar, ty)             \
  do {                                          \
    lua_getglobal(L, #lvar);                    \
    cvar = lua_to ## ty(L, -1);                 \
    lua_pop(L, 1);                              \
  } while (0)

#define CLUE_DO(code)                           \
  luaL_dostring(L, code)
