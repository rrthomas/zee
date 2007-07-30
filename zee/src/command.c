/* Command parser and executor
   Copyright (c) 2005-2006 Reuben Thomas.
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
#include <stdio.h>

#include "config.h"
#include "main.h"
#include "extern.h"
#include "rbacc.h"


static inline int getch(rblist line, size_t *pos)
{
  if (*pos < rblist_length(line))
    return rblist_get(line, (*pos)++);
  return EOF;
}

static rblist gettok(rblist line, size_t *pos)
{
  // Skip space and comments to next non-space character.
  int c;
  do {
    c = getch(line, pos);

    // Skip comments
    if (c == '#')
      do
        c = getch(line, pos);
      while (c != EOF && c != '\n');
  } while (c == ' ' || c == '\t');

  // Read token.
  rbacc tok = rbacc_new();
  while (c != '#' && c != ' ' && c != '\t' && c != '\n' && c != EOF) {
    tok = rbacc_add_char(tok, c);
    c = getch(line, pos);
  }

  if (c == EOF && rbacc_length(tok) == 0)
    return NULL;
  return rbacc_to_rblist(tok);
}

/*
 * Execute a string as commands
 */
bool cmd_eval(rblist s, rblist source)
{
  bool ok = true;
  size_t lineno = 1, pos = 0;
  int top = lua_gettop(L);

  assert(s);
  minibuf_error_set_source(source);

  for (rblist tok = gettok(s, &pos);
       tok != NULL;
       tok = gettok(s, &pos)) {
    minibuf_error_set_lineno(lineno);

    // Get tokens until we run out or reach a new line
    while (tok && tok != rblist_empty) {
      lua_pushstring(L, rblist_to_string(tok));
      tok = gettok(s, &pos);
    }

    // Execute the line
    rblist fname = NULL;
    lua_CFunction cmd = NULL;
    while (lua_gettop(L) > 0) {
      const char *fname_s = lua_tostring(L, top + 1);
      if (fname_s) {
        fname = rblist_from_string(fname_s);
        cmd = get_variable_cfunction(fname);
        if (cmd) {
          assert(cmd(L) == 1);
          ok = lua_toboolean(L, -1);
          lua_pop(L, 1);
        }          
      }
      lua_pop(L, 1);
      if (!fname || !cmd || !ok)
        break;
    }
    if (fname && !cmd) {
      minibuf_error(rblist_fmt("No such command `%r'", fname));
      ok = false;
    }
    lineno++;
  }

  minibuf_error_set_lineno(0);
  minibuf_error_set_source(NULL);
  return ok;
}

/*
 * Set up command name to C function mapping
 */
void init_commands(void)
{
#define X(cmd_name, doc)                                                \
  lua_register(L, # cmd_name, F_ ## cmd_name);
#include "tbl_funcs.h"
#undef X
#define X(cmd_name, doc)                            \
  lua_pushcfunction(L, F_ ## cmd_name);             \
  lua_pushstring(L, # cmd_name);                    \
  lua_settable(L, LUA_GLOBALSINDEX);
#include "tbl_funcs.h"
#undef X
}

/*
 * Read a command name from the minibuffer.
 */
rblist minibuf_read_command_name(rblist prompt)
{
  static History commands_history;
  rblist ms;
  Completion *cp = completion_new();
  cp->completions = LUA_GLOBALSINDEX; // FIXME: Need to filter out commands

  for (;;) {
    ms = minibuf_read_completion(prompt, rblist_empty, cp, &commands_history);

    if (ms == NULL)
      return NULL;

    if (rblist_length(ms) == 0) {
      minibuf_error(rblist_from_string("No command name given"));
      return NULL;
    }

    // Complete partial words if possible
    if (completion_try(cp, ms))
      ms = cp->match;

    if (get_variable_cfunction(ms) != NULL) {
      add_history_element(&commands_history, ms);
      minibuf_clear();        // Remove any error message
      break;
    } else {
      minibuf_error(rblist_fmt("Undefined command `%r'", ms));
      waitkey(WAITKEY_DEFAULT);
    }
  }

  return ms;
}
