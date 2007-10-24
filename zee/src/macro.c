/* Macro facility functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2005, 2007 Reuben Thomas.
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

#include "config.h"
#include "main.h"
#include "extern.h"


static bool cmd_started = false, macro_defined = false;

#define MACRO_NAME "_macro"
#define CMD_NAME "_cmd"

static void macro_new(const char *name)
{
  lua_newtable(L);
  lua_setglobal(L, name);
}

static void add_macro_key(const char *name, size_t key)
{
  lua_getglobal(L, name);
  if (lua_istable(L, -1)) {
    lua_pushnumber(L, (lua_Number)key);
    lua_rawseti(L, -2, (int)lua_objlen(L, -2) + 1);
  }
  lua_pop(L, 1);
  macro_defined = true;
}

/* FIXME: macros should be executed immediately and abort on error;
   they should be stored as a macro list, not a series of
   keystrokes. Macros should return success/failure. */
void add_cmd_to_macro(void)
{
  assert(cmd_started);
  lua_getglobal(L, CMD_NAME);
  lua_pushnil(L); // first key
  while (lua_next(L, -2) != 0) {
    if (lua_isnumber(L, -1))
      add_macro_key(MACRO_NAME, (size_t)lua_tonumber(L, -1));
    lua_pop(L, 1); // remove value; keep key for next iteration
  }
  lua_pop(L, 1); // pop table
  cmd_started = false;
}

void add_key_to_cmd(size_t key)
{
  if (!cmd_started) {
    macro_new(CMD_NAME);
    cmd_started = true;
  }

  add_macro_key(CMD_NAME, key);
}

void cancel_macro_definition(void)
{
  cmd_started = macro_defined = false;
  thisflag &= ~FLAG_DEFINING_MACRO;
}

DEF(macro_record,
"\
Record subsequent keyboard input, defining a macro.\n\
The commands are recorded even as they are executed.\n\
Use macro_stop to finish recording and make the macro available.\n\
Use macro_name to give it a permanent name.\
")
{
  if (thisflag & FLAG_DEFINING_MACRO) {
    minibuf_error(rblist_from_string("Already defining a macro"));
    ok = false;
  } else {
    if (macro_defined)
      cancel_macro_definition();

    minibuf_write(rblist_from_string("Defining macro..."));
    thisflag |= FLAG_DEFINING_MACRO;
    macro_new(MACRO_NAME);
  }
}
END_DEF

DEF(macro_stop,
"\
Finish defining a macro.\
")
{
  if (!(thisflag & FLAG_DEFINING_MACRO)) {
    minibuf_error(rblist_from_string("Not defining a macro"));
    ok = false;
  } else
    thisflag &= ~FLAG_DEFINING_MACRO;
}
END_DEF

DEF(macro_name,
"\
Assign a name to the last macro defined.\n\
Argument SYMBOL is the name to define.\n\
The symbol's command definition becomes the macro string.\n\
")
{
  rblist ms;

  if ((ms = minibuf_read(rblist_from_string("Name for last macro: "), rblist_empty)) == NULL) {
    minibuf_error(rblist_from_string("No command name given"));
    ok = false;
  } else if (!macro_defined) {
    minibuf_error(rblist_from_string("No macro defined"));
    ok = false;
  } else {
    lua_getglobal(L, MACRO_NAME);
    lua_setglobal(L, rblist_to_string(ms));
  }
}
END_DEF

// FIXME: Allow named commands to be run.
void call_macro(const char *name)
{
  lua_getglobal(L, name);
  lua_pushnil(L); // first key
  while (lua_next(L, -2) != 0) {
    if (lua_isnumber(L, -1))
      ungetkey((size_t)lua_tonumber(L, -1));
    lua_pop(L, 1); // remove value; keep key for next iteration
  }
  lua_pop(L, 1); // pop table
}

DEF(macro_play,
"\
Play back the last macro that you defined.\n\
To name a macro so you can call it after defining others, use\n\
macro_name.\
")
{
  if (!macro_defined) {
    minibuf_error(rblist_from_string("No macro has been defined"));
    ok = false;
  } else
    call_macro(MACRO_NAME);
}
END_DEF
