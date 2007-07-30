/* C API for Lua lists
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

#include <stdlib.h>
#include <lauxlib.h>

#include "list.h"
#include "main.h"
#include "extern.h"


// FIXME: Check we have room to push stuff on the Lua stack
// EVERYWHERE.

// Create an empty list, returning a reference to the list
list list_new(void)
{
  lua_newtable(L);
  return luaL_ref(L, LUA_REGISTRYINDEX);
}

// Destroy a list reference
void list_free(list l)
{
  luaL_unref(L, LUA_REGISTRYINDEX, l);
}

// Return the length of a list
size_t list_length(list l)
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, l);
  size_t ret = lua_objlen(L, -1);
  lua_pop(L, 1);
  return ret;
}

// Add an item to the tail of a list
list list_append(list l, void *i)
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, l);
  lua_pushlightuserdata(L, i);
  lua_rawseti(L, -2, (int)list_length(l) + 1);
  lua_pop(L, 1);
  return l;
}

// Add an item to the tail of a list
list list_append_string(list l, const char *s)
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, l);
  lua_pushstring(L, s);
  lua_rawseti(L, -2, (int)list_length(l) + 1);
  lua_pop(L, 1);
  return l;
}

// Return an arbitrary list item that is a string, or NULL if it is
// not a string
const char *list_get_string(int l, size_t n)
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, l);
  lua_rawgeti(L, -1, (int)n);
  const char *ret = lua_tostring(L, -1);
  lua_pop(L, 2);
  return ret;
}

// Set an arbitrary list item to a string
list list_set_string(list l, size_t n, const char *s)
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, l);
  lua_pushstring(L, s);
  lua_rawseti(L, -2, (int)n);
  lua_pop(L, 1);
  return l;
}

/*
 * Remove the first item of a list, returning the item, or NULL if the
 * list is empty
 */
const void *list_behead(list l)
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, l);
  lua_rawgeti(L, -1, (int)1);
  const char *ret = lua_topointer(L, -1);
  lua_pop(L, 1);

  // Move other list items down
  for (size_t n = 1; n < list_length(l); n++) {
    lua_rawgeti(L, -1, (int)n + 1);
    lua_rawseti(L, -2, (int)n);
  }

  lua_pop(L, 1);
  return ret;
}

// Remove the first item of a list, returning the string, or NULL if the
// list is empty or the item is not a string
const char *list_behead_string(list l)
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, l);
  lua_rawgeti(L, -1, (int)1);
  const char *ret = lua_tostring(L, -1);
  lua_pop(L, 1);

  // Move other list items down
  for (size_t n = 1; n <= list_length(l); n++) {
    lua_rawgeti(L, -1, (int)n + 1);
    lua_rawseti(L, -2, (int)n);
  }

  lua_pop(L, 1);
  return ret;
}

// Remove the first item of a list, returning the item, or NULL if the
// list is empty
const void *list_betail(list l)
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, l);
  lua_rawgeti(L, -1, (int)lua_objlen(L, -1));
  const char *ret = lua_topointer(L, -1);
  lua_pop(L, 2);
  return ret;
}
