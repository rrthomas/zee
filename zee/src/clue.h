/* Clue: minimal C-Lua integration
   Copyright (c) 2007 Reuben Thomas.
   All rights reserved.

   Clue is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Clue is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Clue; see the file COPYING.  If not, write to the Free
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

#include <assert.h>
#include <lua.h>
#include <lauxlib.h>


/* Before using Clue:

      * #include "clue.h"
      * In a suitably global scope: CLUE_DECLS;
*/
#define CLUE_DECLS(L)                           \
  lua_State *L

/* Call this macro before using the others */
#define CLUE_INIT(L)                            \
  do {                                          \
    L = luaL_newstate();                        \
    assert(L);                                  \
    luaL_openlibs(L);                           \
  } while (0)

/* Call this macro after last use of Clue */
#define CLUE_CLOSE(L)                           \
  lua_close(L)

/* Import a C value `cexp' into a Lua global `lvar', as Lua type
   `lty'. */
#define CLUE_IMPORT(L, cexp, lvar, lty)         \
  do {                                          \
    lua_push ## lty(L, cexp);                   \
    lua_setglobal(L, #lvar);                    \
  } while (0)

/* Export a Lua variable `lvar' of Lua type `lty' to a C variable
   `cvar'. */
#define CLUE_EXPORT(L, cvar, lvar, lty)         \
  do {                                          \
    lua_getglobal(L, #lvar);                    \
    cvar = lua_to ## lty(L, -1);                \
    lua_pop(L, 1);                              \
  } while (0)

/* Run some Lua code `code'. */
#define CLUE_DO(L, code)                        \
  luaL_dostring(L, code)
