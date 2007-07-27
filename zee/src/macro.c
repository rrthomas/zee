/* Macro facility functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2005 Reuben Thomas.
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


static vector *cur_mp = NULL, *cmd_mp = NULL;

static vector *macro_new(void)
{
  return vec_new(sizeof(size_t));
}

static void add_macro_key(vector *mp, size_t key)
{
  vec_item(mp, vec_items(mp), size_t) = key;
}

/* FIXME: macros should be executed immediately and abort on error;
   they should be stored as a macro list, not a series of
   keystrokes. vectors should return success/failure. */
void add_cmd_to_macro(void)
{
  size_t i;
  assert(cmd_mp);
  for (i = 0; i < vec_items(cmd_mp); i++)
    add_macro_key(cur_mp, vec_item(cmd_mp, i, size_t));
  cmd_mp = NULL;
}

void add_key_to_cmd(size_t key)
{
  if (cmd_mp == NULL)
    cmd_mp = macro_new();

  add_macro_key(cmd_mp, key);
}

void cancel_macro_definition(void)
{
  cmd_mp = cur_mp = NULL;
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
    if (cur_mp)
      cancel_macro_definition();

    minibuf_write(rblist_from_string("Defining macro..."));

    thisflag |= FLAG_DEFINING_MACRO;
    cur_mp = macro_new();
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
FIXME: Such a command cannot be called at the moment!\
")
{
  rblist ms;

  if ((ms = minibuf_read(rblist_from_string("Name for last macro: "), rblist_empty)) == NULL) {
    minibuf_error(rblist_from_string("No command name given"));
    ok = false;
  } else if (cur_mp == NULL) {
    minibuf_error(rblist_from_string("No macro defined"));
    ok = false;
  } else
    set_variable_blob(ms, vec_copy(cur_mp));
}
END_DEF

void call_macro(vector *mp)
{
  /* The loop termination condition is really i >= 0, but unsigned
     types are always >= 0. */
  for (size_t i = vec_items(mp) - 1; i < vec_items(mp); i--)
    ungetkey(vec_item(mp, i, size_t));
}

DEF(macro_play,
"\
Play back the last macro that you defined.\n\
To name a macro so you can call it after defining others, use\n\
macro_name.\
")
{
  if (cur_mp == NULL) {
    minibuf_error(rblist_from_string("No macro has been defined"));
    ok = false;
  } else
    call_macro(cur_mp);
}
END_DEF
