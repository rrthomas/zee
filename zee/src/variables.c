/* Variables functions
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
#include <stdlib.h>
#include <lua.h>

#include "main.h"
#include "extern.h"


void set_variable(rblist key, rblist val)
{
  if (key && val) {
    lua_pushstring(L, rblist_to_string(val));
    lua_setglobal(L, rblist_to_string(key));
  }
}

void set_variable_blob(rblist key, void *val)
{
  if (key && val) {
    lua_pushlightuserdata(L, val);
    lua_setglobal(L, rblist_to_string(key));
  }
}

rblist get_blob_variable_string(void *key)
{
  rblist ret = NULL;

  if (key) {
    lua_pushlightuserdata(L, key);
    lua_gettable(L, LUA_GLOBALSINDEX);
    if (lua_isstring(L, -1))
      ret = rblist_from_string(lua_tostring(L, -1));
    lua_pop(L, 1);
  }

  return ret;
}

void *get_variable_blob(rblist key)
{
  void *ret = NULL;

  if (key) {
    lua_getglobal(L, rblist_to_string(key));
    if (lua_isuserdata(L, -1))
      ret = lua_touserdata(L, -1);
    lua_pop(L, 1);
  }

  return ret;
}

rblist get_variable_string(rblist key)
{
  rblist ret = NULL;

  if (key) {
    lua_getglobal(L, rblist_to_string(key));
    if (lua_isstring(L, -1))
      ret = rblist_from_string(lua_tostring(L, -1));
    lua_pop(L, 1);
  }

  return ret;
}

int get_variable_number(rblist var)
{
  rblist rbl = get_variable_string(var);

  if (rbl)
    return atoi(rblist_to_string(rbl));

  return 0;
}

bool get_variable_bool(rblist var)
{
  rblist rbl = get_variable_string(var);

  if (rbl)
    return !rblist_compare(rbl, rblist_from_string("true"));

  return false;
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

DEF(preferences_set_variable,
"\
Set a variable to the specified value.\
")
{
  rblist var, val = NULL;

  if (list_length(l) > 1) {
    var = rblist_from_string(list_behead_string(l));
    val = rblist_from_string(list_behead_string(l));
  } else if ((var = minibuf_read_variable_name(rblist_from_string("Set variable: "))))
    val = minibuf_read(rblist_fmt("Set %r to value: ", var), rblist_empty);

  if (val)
    set_variable(var, val);
  else
    ok = false;
}
END_DEF
