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

#include "config.h"

#include "main.h"
#include "extern.h"

DEFUN(help_about,
"\
Show the version in the minibuffer.\
")
{
  minibuf_write(astr_new(PACKAGE_NAME " " VERSION " of " CONFIGURE_DATE " on " CONFIGURE_HOST));
}
END_DEFUN

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

DEFUN(help_command,
"\
Display the full documentation of FUNCTION (a symbol).\
")
{
  astr name;

  ok = FALSE;

  if ((name = minibuf_read_function_name(astr_new("Describe function: ")))) {
    size_t i;
    for (i = 0; i < fentries; i++)
      if (!astr_cmp(astr_new(ftable[i].name), name)) {
        popup_set(astr_afmt("Help for command `%s':\n\n%s", ftable[i].name, ftable[i].doc));
        ok = TRUE;
        break;
      }
  }
}
END_DEFUN

static struct {
  const char *name;
  const char *fmt;
  const char *defval;
  const char *doc;
} vtable[] = {
#define X(name, fmt, defvalue, doc) \
	{name, fmt, defvalue, doc},
#include "tbl_vars.h"
#undef X
};
#define ventries (sizeof(vtable) / sizeof(vtable[0]))

DEFUN(help_variable,
"\
Display the full documentation of VARIABLE (a symbol).\
")
{
  astr name;

  ok = FALSE;

  if ((name = minibuf_read_variable_name(astr_new("Describe variable: ")))) {
    size_t i;
    for (i = 0; i < ventries; i++)
      if (!astr_cmp(astr_new(vtable[i].name), name)) {
        popup_set(astr_afmt("Help for variable `%s':\n\n"
                            "Default value: %s\n"
                            "Current value: %s\n\n"
                            "Documentation:\n%s",
                            vtable[i].name, vtable[i].defval,
                            astr_cstr(get_variable(name)), vtable[i].doc));
        ok = TRUE;
      }
  }
}
END_DEFUN

DEFUN(help_key,
"\
Display the command invoked by a key sequence.\
")
{
  size_t key;
  astr keyname, cmd;

  minibuf_write(astr_new("Describe key:"));
  key = getkey();
  keyname = chordtostr(key);

  if ((cmd = binding_to_function(key)) == NULL) {
    minibuf_error(astr_afmt("%s is unbound", astr_cstr(keyname)));
    ok = FALSE;
  } else
    minibuf_write(astr_afmt("%s runs the command `%s'", astr_cstr(keyname), astr_cstr(cmd)));
}
END_DEFUN
