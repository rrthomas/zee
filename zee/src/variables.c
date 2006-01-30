/* Variables handling functions
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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"


typedef struct {
  const char *key;
  const char *value;
} vpair;

static list variable_find(list varlist, const char *key)
{
  list p;

  if (key != NULL)
    for (p = list_first(varlist); p != varlist; p = list_next(p))
      if (!strcmp(key, ((vpair *)(p->item))->key))
        return p;

  return NULL;
}

static void variable_update(list varlist, const char *key, const char *value)
{
  if (key && value) {
    list temp = variable_find(varlist, key);

    if (temp && temp->item)
      free((void *)(((vpair *)(temp->item))->value));
    else {
      vpair *vp = zmalloc(sizeof(vpair));
      vp->key = zstrdup(key);
      list_prepend(varlist, vp);
      temp = list_first(varlist);
    }

    ((vpair *)(temp->item))->value = zstrdup(value);
  }
}


/*
 * Default variables values table.
 */
static struct var_entry {
  char *var;                    /* Variable name. */
  char *fmt;                    /* Variable format (boolean, etc.). */
  const char *val;              /* Default value. */
} def_vars[] = {
#define X(var, fmt, val, doc) { var, fmt, val },
#include "tbl_vars.h"
#undef X
};

static struct var_entry *get_variable_default(const char *var)
{
  struct var_entry *p;
  for (p = &def_vars[0]; p < &def_vars[sizeof(def_vars) / sizeof(def_vars[0])]; p++)
    if (!strcmp(p->var, var))
      return p;

  return NULL;
}

void set_variable(const char *var, const char *val)
{
  /* Variables automatically become buffer-local when set if there is
     a buffer. */
  variable_update(buf.vars, var, val);
}

const char *get_variable(const char *var)
{
  const char *s = NULL;

  /* Have to be able to run this before the first buffer is created. */
  if (buf.vars) {
    list temp = variable_find(buf.vars, var);
    if (temp && temp->item)
      s = (char *)(temp->item);
  }

  if (s == NULL) {
    struct var_entry *p = get_variable_default(var);
    if (p)
      s = p->val;
  }

  return s;
}

int get_variable_number(char *var)
{
  const char *s;

  if ((s = get_variable(var)))
    return atoi(s);

  return 0;
}

int get_variable_bool(char *var)
{
  const char *s;

  if ((s = get_variable(var)) != NULL)
    return !strcmp(s, "true");

  return FALSE;
}

astr minibuf_read_variable_name(char *msg)
{
  astr ms;
  Completion *cp = completion_new(FALSE);
  struct var_entry *p;

  for (p = &def_vars[0]; p < &def_vars[sizeof(def_vars) / sizeof(def_vars[0])]; p++)
    list_append(cp->completions, zstrdup(p->var));

  for (;;) {
    ms = minibuf_read_completion(msg, "", cp, NULL);

    if (ms == NULL) {
      free_completion(cp);
      FUNCALL(cancel);
      return NULL;
    }

    if (astr_len(ms) == 0) {
      free_completion(cp);
      minibuf_error("No variable name given");
      return NULL;
    } else if (get_variable(astr_cstr(ms)) == NULL) {
      minibuf_error("There is no variable called `%s'", astr_cstr(ms));
      waitkey(WAITKEY_DEFAULT);
    } else {
      minibuf_clear();
      break;
    }
  }

  free_completion(cp);

  return ms;
}

DEFUN("set-variable", set_variable)
/*+
Set a variable to the specified value.
+*/
{
  char *fmt;
  astr var, val = NULL;

  if (argc > 0) {
    const char *newvalue = NULL;

    if (*lp != NULL) {
      const char *name = (char *)((*lp)->item);
      *lp = list_next(*lp);
      if (*lp != NULL) {
        newvalue = (char *)((*lp)->item);
        *lp = list_next(*lp);
      }

      set_variable(name, newvalue);
    }
  } else {
    if ((var = minibuf_read_variable_name("Set variable: ")) == NULL)
      ok = FALSE;
    else {
      struct var_entry *p = get_variable_default(astr_cstr(var));
      fmt = p ? p->fmt : "";
      if (!strcmp(fmt, "b")) {
        int i;
        if ((i = minibuf_read_boolean("Set %s to value: ", astr_cstr(var))) == -1)
          ok = FUNCALL(cancel);
        else
          astr_cpy_cstr(val, (i == TRUE) ? "true" : "false");
      } else                      /* Non-boolean variable. */
        if ((val = minibuf_read("Set %s to value: ", "", astr_cstr(var))) == NULL)
          ok = FUNCALL(cancel);

      if (ok)
        set_variable(astr_cstr(var), astr_cstr(val));
    }
  }
}
END_DEFUN
