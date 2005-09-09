/* Variables handling functions
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
#include "eval.h"


le *mainVarList = NULL;
le *defunList = NULL;


static le *variableFind(le *varlist, const char *key)
{
  if (key != NULL)
    for (; varlist; varlist = varlist->list_next)
      if (!strcmp(key, varlist->data))
        return varlist;

  return NULL;
}

void variableSet(le **varlist, const char *key, le *value)
{
  if (key && value) {
    le *temp = variableFind(*varlist, key);

    if (temp)
      leWipe(temp->branch);
    else {
      temp = leNew(key);
      *varlist = leAddHead(*varlist, temp);
    }

    temp->branch = leDup(value);
  }
}

void variableSetString(le **varlist, const char *key, const char *value)
{
  if (key && value) {
    le *temp = leNew(value);
    variableSet(varlist, key, temp);
    leWipe(temp);
  }
}

le *variableGet(le *varlist, char *key)
{
  le *temp = variableFind(varlist, key);
  return temp ? temp->branch : NULL;
}

char *variableGetString(le *varlist, const char *key)
{
  le *temp = variableFind(varlist, key);
  if (temp && temp->branch && temp->branch->data
      && countNodes(temp->branch) == 1)
    return temp->branch->data;
  return NULL;
}


/*
 * Default variables values table.
 */
static struct var_entry {
  char *var;                    /* Variable name. */
  char *fmt;                    /* Variable format (boolean, etc.). */
  char *val;                    /* Default value. */
} def_vars[] = {
#define X(var, fmt, val, doc) { var, fmt, val },
#include "tbl_vars.h"
#undef X
};

void set_variable(const char *var, const char *val)
{
  /* Variables automatically become buffer-local when set. */
  assert(cur_bp);
  variableSetString(&cur_bp->vars, var, val);
}

static struct var_entry *get_variable_default(const char *var)
{
  struct var_entry *p;
  for (p = &def_vars[0]; p < &def_vars[sizeof(def_vars) / sizeof(def_vars[0])]; p++)
    if (!strcmp(p->var, var))
      return p;

  return NULL;
}

char *get_variable(const char *var)
{
  char *s = NULL;

  /* Have to be able to run this before the first buffer is created. */
  if (cur_bp)
    s = variableGetString(cur_bp->vars, var);

  if (s == NULL) {
    struct var_entry *p = get_variable_default(var);
    if (p)
      s = p->val;
  }

  return s;
}

int get_variable_number(char *var)
{
  char *s;

  if ((s = get_variable(var)))
    return atoi(s);

  return 0;
}

int get_variable_bool(char *var)
{
  char *s;

  if ((s = get_variable(var)) != NULL)
    return !strcmp(s, "true");

  return FALSE;
}

astr minibuf_read_variable_name(char *msg)
{
  astr ms;
  Completion *cp = completion_new(FALSE);
  le *lp;

  for (lp = mainVarList; lp != NULL; lp = lp->list_next)
    list_append(cp->completions, zstrdup(lp->data));

  for (;;) {
    ms = minibuf_read_completion(msg, "", cp, NULL);

    if (ms == NULL) {
      free_completion(cp);
      cancel();
      return NULL;
    }

    if (astr_len(ms) == 0) {
      free_completion(cp);
      minibuf_error("No variable name given");
      return NULL;
    } else if (get_variable(astr_cstr(ms)) == NULL) {
      minibuf_error("Undefined variable name `%s'", astr_cstr(ms));
      waitkey(WAITKEY_DEFAULT);
    } else {
      minibuf_clear();
      break;
    }
  }

  free_completion(cp);

  return ms;
}

DEFUN_INT("set-variable", set_variable)
/*+
Set a variable value to the user-specified value.
+*/
{
  char *fmt;
  astr var, val = NULL;

  assert(cur_bp);

  if ((var = minibuf_read_variable_name("Set variable: ")) == NULL)
    ok = FALSE;
  else {
    struct var_entry *p = get_variable_default(astr_cstr(var));
    fmt = p ? p->fmt : "";
    if (!strcmp(fmt, "b")) {
      int i;
      if ((i = minibuf_read_boolean("Set %s to value: ", astr_cstr(var))) == -1)
        ok = cancel();
      else
        astr_cpy_cstr(val, (i == TRUE) ? "true" : "false");
    } else /* Non-boolean variable. */
      if ((val = minibuf_read("Set %s to value: ", "", astr_cstr(var))) == NULL)
        ok = cancel();

    if (ok)
      set_variable(astr_cstr(var), astr_cstr(val));
  }
}
END_DEFUN
