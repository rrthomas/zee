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


rblist get_variable(rblist key)
{
  rblist ret = NULL;

  if (key) {
    lua_getfield(L, LUA_GLOBALSINDEX, rblist_to_string(key));
    if (lua_isstring(L, -1))
      ret = rblist_from_string(lua_tostring(L, -1));
    lua_pop(L, 1);
  }

  return ret;
}

void set_variable(rblist key, rblist val)
{
  if (key && val) {
    lua_pushstring(L, rblist_to_string(val));
    lua_setfield(L, LUA_GLOBALSINDEX, rblist_to_string(key));
  }
}

int get_variable_number(rblist var)
{
  rblist rbl = get_variable(var);

  if (rbl)
    return atoi(rblist_to_string(rbl));

  return 0;
}

bool get_variable_bool(rblist var)
{
  rblist rbl = get_variable(var);

  if (rbl)
    return !rblist_compare(rbl, rblist_from_string("true"));

  return false;
}

rblist minibuf_read_variable_name(rblist msg)
{
  rblist ms;
  Completion *cp = completion_new();

  lua_pushnil(L);  /* first key */
  while (lua_next(L, LUA_GLOBALSINDEX) != 0) {
    lua_pop(L, 1);        // remove value; keep key for next iteration
    list_append(cp->completions, rblist_from_string(lua_tostring(L, -1)));
  }
  lua_pop(L, 1);                // pop last key

  for (;;) {
    ms = minibuf_read_completion(msg, rblist_empty, cp, NULL);

    if (ms == NULL)
      return NULL;

    if (rblist_length(ms) == 0) {
      minibuf_error(rblist_from_string("No variable name given"));
      return NULL;
    } else if (get_variable(ms) == NULL) {
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
    var = list_behead(l);
    val = list_behead(l);
  } else if ((var = minibuf_read_variable_name(rblist_from_string("Set variable: "))))
    val = minibuf_read(rblist_fmt("Set %r to value: ", var), rblist_empty);

  if (val)
    set_variable(var, val);
  else
    ok = false;
}
END_DEF
