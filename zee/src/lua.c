/* Lua utility functions
   Copyright (c) 2007 Reuben Thomas.  All rights reserved.

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

#include <lauxlib.h>

#include "main.h"
#include "extern.h"


int lualist_new(void)
{
  lua_newtable(L);
  return luaL_ref(L, LUA_REGISTRYINDEX);
}

void lualist_free(int l)
{
  luaL_unref(L, LUA_REGISTRYINDEX, l);
}

size_t lualist_length(int l)
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, l);
  size_t ret = lua_objlen(L, -1);
  lua_pop(L, 1);
  return ret;
}

const char *lualist_get_string(int l, size_t n)
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, l);
  lua_rawgeti(L, -1, (int)n);
  const char *ret = lua_tostring(L, -1);
  lua_pop(L, 2);
  return ret;
}

void lualist_set_string(int l, size_t n, const char *s)
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, l);
  lua_pushstring(L, s);
  lua_rawseti(L, -2, (int)n);
  lua_pop(L, 1);
}
