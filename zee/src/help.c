/* Self documentation facility functions
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

#include <stdbool.h>

#include "config.h"

#include "main.h"
#include "extern.h"


DEF(help_about,
"\
Show the version in the minibuffer.\
")
{
  fprintf(stderr, "%d\n", CLUE_DO(L, "s = command_to_binding(\"file_quit\")"));
  const char *quitstr;
  CLUE_EXPORT(L, quitstr, s, string);
  if (quitstr == NULL)
    quitstr = "Alt-x, `file_quit', RETURN";

  minibuf_write(rblist_fmt(PACKAGE_STRING ". For a menu type Alt-x. To quit, type %s.", quitstr));
}
END_DEF

static const char *get_docstring(rblist name)
{
  (void)CLUE_DO(L, rblist_to_string(rblist_fmt("s = docstring[\"%r\"]", name)));
  const char *s;
  CLUE_EXPORT(L, s, s, string);
  return s;
}

DEF(help_thing,
"\
Display the help for the given thing.\
")
{
  rblist name;

  ok = false;

  if ((name = minibuf_read_name(rblist_from_string("Describe thing: ")))) {
    rblist where = rblist_empty;
    (void)CLUE_DO(L, rblist_to_string(rblist_fmt("s = command_to_binding(\"%r\")", name)));
    const char *bindings;
    CLUE_EXPORT(L, bindings, s, string);
    if (bindings)
      where = rblist_fmt("\n\nBound to: %s", bindings);
    const char *doc = get_docstring(name);
    if (doc == NULL) {
      doc = "No help available";
    }
    popup_set(rblist_fmt("Help for `%r':\n\n%s%r", name, doc, where));
    ok = true;
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
