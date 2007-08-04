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

// FIXME: Use Lua booleans
bool get_variable_bool(rblist var)
{
  rblist rbl = get_variable_string(var);

  if (rbl)
    return !rblist_compare(rbl, rblist_from_string("true"));

  return false;
}

DEF(preferences_set_variable,
"\
Set a variable to the specified value.\
")
{
  rblist var, val = NULL;

  if (lua_gettop(L) > 1) {
    var = rblist_from_string(lua_tostring(L, -2));
    val = rblist_from_string(lua_tostring(L, -1));
    lua_pop(L, 2);
  } else if ((var = minibuf_read_name(rblist_from_string("Set variable: "))))
    val = minibuf_read(rblist_fmt("Set %r to value: ", var), rblist_empty);

  if (var && val) {
    lua_pushstring(L, rblist_to_string(val));
    lua_setglobal(L, rblist_to_string(var));
  } else {
    ok = false;
  }
}
END_DEF
