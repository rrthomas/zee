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

  assert(s);
  minibuf_error_set_source(source);

  for (rblist tok = gettok(s, &pos);
       tok != NULL;
       tok = gettok(s, &pos)) {
    minibuf_error_set_lineno(lineno);

      // Get tokens until we run out or reach a new line
    list l = list_new();
    while (tok && tok != rblist_empty) {
      list_append(l, tok);
      tok = gettok(s, &pos);
    }

    // Execute the line
    rblist fname;
    Command cmd;
    while ((fname = list_behead(l)) &&
           (cmd = get_command(fname)) &&
           (ok &= cmd(l)))
      ;
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

/*--------------------------------------------------------------------------
 * Command name to C function mapping
 *--------------------------------------------------------------------------*/

static struct {
  const char *name;             // The command name
  Command cmd;                  // The function pointer
} ftable[] = {
#define X(cmd_name, doc) \
	{# cmd_name, F_ ## cmd_name},
#include "tbl_funcs.h"
#undef X
};
#define fentries (sizeof(ftable) / sizeof(ftable[0]))

Command get_command(rblist name)
{
  size_t i;
  if (name)
    for (i = 0; i < fentries; i++)
      if (!rblist_compare(name, rblist_from_string(ftable[i].name)))
        return ftable[i].cmd;
  return NULL;
}

rblist get_command_name(Command cmd)
{
  size_t i;
  for (i = 0; i < fentries; i++)
    if (ftable[i].cmd == cmd)
      return rblist_from_string(ftable[i].name);
  return NULL;
}

list command_list(void)
{
  list l = list_new();
  for (size_t i = 0; i < fentries; ++i)
    list_append(l, rblist_from_string(ftable[i].name));
  return l;
}

/*
 * Read a command name from the minibuffer.
 */
rblist minibuf_read_command_name(rblist prompt)
{
  static History commands_history;
  rblist ms;
  Completion *cp = completion_new();
  cp->completions = command_list();

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

    if (get_command(ms) || get_variable_blob(ms)) {
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
