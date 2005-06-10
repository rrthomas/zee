/* Self documentation facility functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2005 Reuben Thomas.
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
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"
#include "paths.h"

DEFUN_INT("help-about", help_about)
/*+
Show the version in the minibuffer.
+*/
{
  minibuf_write(PACKAGE_NAME " " VERSION " of " CONFIGURE_DATE " on " CONFIGURE_HOST);
}
END_DEFUN

/*
 * Fetch the documentation of a function or variable from the
 * AUTODOC automatically generated file.
 */
static astr get_funcvar_doc(const char *name, astr defval, int isfunc)
{
  FILE *f;
  astr buf, match, doc;
  int reading_doc = 0;

  if ((f = fopen(PATH_DATA "/AUTODOC", "r")) == NULL) {
    minibuf_error("Unable to read file `%s'",
                  PATH_DATA "/AUTODOC");
    return NULL;
  }

  match = astr_new();
  if (isfunc)
    astr_afmt(match, "\fF_%s", name);
  else
    astr_afmt(match, "\fV_%s", name);

  doc = astr_new();
  while ((buf = astr_fgets(f)) != NULL) {
    if (reading_doc) {
      if (*astr_char(buf, 0) == '\f') {
        astr_delete(buf);
        break;
      }
      if (isfunc || astr_len(defval) > 0) {
        astr_cat(doc, buf);
        astr_cat_cstr(doc, "\n");
      } else
        astr_cpy(defval, buf);
    } else if (!astr_cmp(buf, match))
      reading_doc = 1;
    astr_delete(buf);
  }

  fclose(f);
  astr_delete(match);

  if (!reading_doc) {
    minibuf_error("Cannot find documentation for `%s'", name);
    astr_delete(doc);
    return NULL;
  }

  return doc;
}

DEFUN_INT("help-command", help_command)
/*+
Display the full documentation of FUNCTION (a symbol).
+*/
{
  astr name, doc;

  if ((name = minibuf_read_function_name("Describe function: "))) {
    if ((doc = get_funcvar_doc(astr_cstr(name), NULL, TRUE))) {
      astr popup = astr_new();
      astr_afmt(popup, "Help for command `%s':\n\n%s", astr_cstr(name), astr_cstr(doc));
      popup_set(popup);
      astr_delete(doc);
    } else
      ok = FALSE;

    astr_delete(name);
  } else
    ok = FALSE;
}
END_DEFUN

DEFUN_INT("help-variable", help_variable)
/*+
Display the full documentation of VARIABLE (a symbol).
+*/
{
  astr name;

  if ((name = minibuf_read_variable_name("Describe variable: "))) {
    astr defval = astr_new(), doc, popup = astr_new();

    if ((doc = get_funcvar_doc(astr_cstr(name), defval, FALSE))) {
      astr_afmt(popup, "Help for variable `%s':\n\n"
                "Default value: %s\n"
                "Current value: %s\n\n"
                "Documentation:\n%s",
                astr_cstr(name), astr_cstr(defval),
                get_variable(astr_cstr(name)), astr_cstr(doc));
      popup_set(popup);
      astr_delete(doc);
    } else
      ok = FALSE;

    astr_delete(defval);
    astr_delete(name);
  } else
    ok = FALSE;
}
END_DEFUN

DEFUN_INT("help-key", help_key)
/*+
Display the command invoked by a key sequence.
+*/
{
  size_t key;
  const char *cmd;
  astr keyname;

  minibuf_write("Describe key:");
  key = getkey();
  keyname = chordtostr(key);

  if ((cmd = get_function_by_key(key)) == NULL) {
    minibuf_error("%s is unbound", astr_cstr(keyname));
    ok = FALSE;
  } else
    minibuf_write("%s runs the command `%s'", astr_cstr(keyname), cmd);

  astr_delete(keyname);
}
END_DEFUN
