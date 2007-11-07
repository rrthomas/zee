/* Clue: minimal C-Lua integration

   Copyright (c) 2007 Reuben Thomas.

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE. */


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
