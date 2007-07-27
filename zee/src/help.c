/* Self documentation facility functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2006 Reuben Thomas.
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


DEF(help_about,
"\
Show the version in the minibuffer.\
")
{
  rblist quitstr = command_to_binding(F_file_quit);

  if (!rblist_compare(quitstr, rblist_empty))
    quitstr = rblist_from_string("Alt-x, `file_quit', RETURN");

  minibuf_write(rblist_fmt(PACKAGE_STRING ". For a menu type Alt-x. To quit, type %r.", quitstr));
}
END_DEF

struct {
  const char *name;
  const char *doc;
} ftable [] = {
#define X(name, doc) \
	{# name, doc},
#include "tbl_funcs.h"
#undef X
};
#define fentries (sizeof(ftable) / sizeof(ftable[0]))

DEF(help_command,
"\
Display the help for the given command.\
")
{
  rblist name;

  ok = false;

  if ((name = minibuf_read_command_name(rblist_from_string("Describe command: ")))) {
    size_t i;
    for (i = 0; i < fentries; i++)
      if (!rblist_compare(rblist_from_string(ftable[i].name), name)) {
        rblist bindings = command_to_binding(get_command(name)), where = rblist_empty;
        if (rblist_length(bindings) > 0)
          where = rblist_fmt("\n\nBound to: %r", bindings);
        popup_set(rblist_fmt("Help for command `%s':\n\n%s%r", ftable[i].name, ftable[i].doc, where));
        ok = true;
        break;
      }
  }
}
END_DEF

static struct {
  const char *name;
  const char *defval;
  const char *doc;
} vtable[] = {
#define X(name, defvalue, doc) \
	{name, defvalue, doc},
#include "tbl_vars.h"
#undef X
};
#define ventries (sizeof(vtable) / sizeof(vtable[0]))

DEF(help_variable,
"\
Display the full documentation of VARIABLE (a symbol).\
")
{
  rblist name;

  ok = false;

  if ((name = minibuf_read_variable_name(rblist_from_string("Describe variable: ")))) {
    size_t i;
    for (i = 0; i < ventries; i++)
      if (!rblist_compare(rblist_from_string(vtable[i].name), name)) {
        popup_set(rblist_fmt("Help for variable `%s':\n\n"
                              "Default value: %s\n"
                              "Current value: %r\n\n"
                              "Documentation:\n%s",
                              vtable[i].name, vtable[i].defval,
                              get_variable(name), vtable[i].doc));
        ok = true;
      }
  }
}
END_DEF

DEF(help_key,
"\
Display the command invoked by a key sequence.\
")
{
  size_t key;
  rblist keyname, cmd;

  minibuf_write(rblist_from_string("Describe key:"));
  key = getkey();
  keyname = chordtostr(key);

  if ((cmd = binding_to_command(key)) == NULL) {
    minibuf_error(rblist_fmt("%r is unbound", keyname));
    ok = false;
  } else
    minibuf_write(rblist_fmt("%r runs the command `%r'", keyname, cmd));
}
END_DEF
