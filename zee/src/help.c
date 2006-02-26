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
#include "paths.h"

DEFUN(help_about)
/*+
Show the version in the minibuffer.
+*/
{
  minibuf_write(astr_new(PACKAGE_NAME " " VERSION " of " CONFIGURE_DATE " on " CONFIGURE_HOST));
}
END_DEFUN

/*
 * Fetch the documentation of a function or variable from the AUTODOC
 * automatically generated file.
 * FIXME: load it all at startup.
 *
 * name - name of function or variable to find, without its F_ or V_ prefix.
 * defval - unused if isfunc, otherwise return value: default value of variable.
 * isfunc - true for function, false for variable.
 */
static astr get_funcvar_doc(astr name, astr *defval, int isfunc)
{
  FILE *f;
  astr buf, match, doc;
  int reading_doc = 0;

  if ((f = fopen(PATH_DATA "/AUTODOC", "r")) == NULL) {
    minibuf_error(astr_new("Unable to read file `" PATH_DATA "/AUTODOC'"));
    return NULL;
  }

  if (isfunc)
    match = astr_afmt("\fF_%s", astr_cstr(name));
  else
    match = astr_afmt("\fV_%s", astr_cstr(name));

  /* FIXME: following loop is completely mad! -- should be two loops. */

  doc = astr_new("");
  if (!isfunc)
    *defval = astr_new("");
  
  while ((buf = astr_fgets(f))) {
    if (reading_doc) {
      if (astr_len(buf) > 0 && *astr_char(buf, 0) == '\f')
        break;
      if (isfunc || astr_len(*defval) > 0) {
        astr_cat(doc, buf);
        astr_cat(doc, astr_new("\n"));
      } else
        *defval = astr_dup(buf);
    } else if (!astr_cmp(buf, match))
      reading_doc = 1;
  }

  fclose(f);

  if (!reading_doc) {
    minibuf_error(astr_afmt("Cannot find documentation for `%s'", astr_cstr(name)));
    return NULL;
  }

  return doc;
}

DEFUN(help_command)
/*+
Display the full documentation of FUNCTION (a symbol).
+*/
{
  astr name, doc;

  if ((name = minibuf_read_function_name(astr_new("Describe function: ")))) {
    if ((doc = get_funcvar_doc(name, NULL, TRUE)))
      popup_set(astr_afmt("Help for command `%s':\n\n%s", astr_cstr(name), astr_cstr(doc)));
    else
      ok = FALSE;
  } else
    ok = FALSE;
}
END_DEFUN

DEFUN(help_variable)
/*+
Display the full documentation of VARIABLE (a symbol).
+*/
{
  astr name;

  ok = FALSE;

  if ((name = minibuf_read_variable_name(astr_new("Describe variable: ")))) {
    astr defval, doc;
    if ((doc = get_funcvar_doc(name, &defval, FALSE))) {
      popup_set(astr_afmt("Help for variable `%s':\n\n"
                          "Default value: %s\n"
                          "Current value: %s\n\n"
                          "Documentation:\n%s",
                          astr_cstr(name), astr_cstr(defval),
                          astr_cstr(get_variable(name)), astr_cstr(doc)));
      ok = TRUE;
    }
  }
}
END_DEFUN

DEFUN(help_key)
/*+
Display the command invoked by a key sequence.
+*/
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
