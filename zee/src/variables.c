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

#include "main.h"
#include "extern.h"


static list root_varlist;

typedef struct {
  astr key;
  astr value;
} vpair;

void init_variables(void)
{
  root_varlist = list_new();
}

static list variable_find(list varlist, astr key)
{
  list p;

  if (key)
    for (p = list_first(varlist); p != varlist; p = list_next(p))
      if (!astr_cmp(key, ((vpair *)(p->item))->key))
        return p;

  return NULL;
}

static void variable_update(list varlist, astr key, astr value)
{
  if (key && value) {
    list temp = variable_find(varlist, key);

    if (temp == NULL || temp->item == NULL) {
      vpair *vp = zmalloc(sizeof(vpair));
      vp->key = key;
      list_prepend(varlist, vp);
      temp = list_first(varlist);
    }

    ((vpair *)(temp->item))->value = value;
  }
}


/*
 * Default variables values table.
 */
typedef struct {
  const char *var;              /* Variable name. */
  const char *fmt;              /* Variable format (boolean, etc.). */
  const char *val;              /* Default value. */
} var_entry;

static var_entry def_vars[] = {
#define X(var, fmt, val, doc) \
    {var, fmt, val},
#include "tbl_vars.h"
#undef X
};

static var_entry *get_variable_default(astr var)
{
  var_entry *p;
  for (p = &def_vars[0]; p < &def_vars[sizeof(def_vars) / sizeof(def_vars[0])]; p++)
    if (!astr_cmp(astr_new(p->var), var))
      return p;

  return NULL;
}

void set_variable(astr var, astr val)
{
  list varlist = root_varlist;

  /* Variables automatically become buffer-local when set if there is
     a buffer. */
  if (buf.vars)
    varlist = buf.vars;

  variable_update(varlist, var, val);
}

astr get_variable(astr var)
{
  var_entry *p;

  /* Have to be able to run this before the first buffer is created. */
  if (buf.vars) {
    list temp = variable_find(buf.vars, var);
    if (temp && temp->item)
      return ((vpair *)(temp->item))->value;
  }

  if ((p = get_variable_default(var)))
    return astr_new(p->val);

  return NULL;
}

int get_variable_number(astr var)
{
  astr as;

  if ((as = get_variable(var)))
    return atoi(astr_cstr(as));

  return 0;
}

int get_variable_bool(astr var)
{
  astr as;

  if ((as = get_variable(var)))
    return !astr_cmp(as, astr_new("true"));

  return FALSE;
}

astr minibuf_read_variable_name(astr msg)
{
  astr ms;
  Completion *cp = completion_new();
  var_entry *p;

  for (p = &def_vars[0]; p < &def_vars[sizeof(def_vars) / sizeof(def_vars[0])]; p++)
    list_append(cp->completions, astr_new(p->var));

  for (;;) {
    ms = minibuf_read_completion(msg, astr_new(""), cp, NULL);

    if (ms == NULL) {
      CMDCALL(cancel);
      return NULL;
    }

    if (astr_len(ms) == 0) {
      minibuf_error(astr_new("No variable name given"));
      return NULL;
    } else if (get_variable(ms) == NULL) {
      minibuf_error(astr_afmt("There is no variable called `%s'", astr_cstr(ms)));
      waitkey(WAITKEY_DEFAULT);
    } else {
      minibuf_clear();
      break;
    }
  }

  return ms;
}

/* FIXME: Doesn't work in .zee */
DEF(set_variable,
"\
Set a variable to the specified value.\
")
{
  astr var, val;

  ok = FALSE;

  if (argc > 1) {
    var = list_behead(l);
    val = list_behead(l);
  } else {
    if ((var = minibuf_read_variable_name(astr_new("Set variable: ")))) {
      var_entry *p = get_variable_default(var);
      if (!astr_cmp(astr_new(p ? p->fmt : ""), astr_new("b"))) {
        int i;
        if ((i = minibuf_read_boolean(astr_afmt("Set %s to value: ", astr_cstr(var)))) == -1)
          CMDCALL(cancel);
        else {
          val = astr_new((i == TRUE) ? "true" : "false");
          ok = TRUE;
        }
      } else                    /* Non-boolean variable. */
        if ((val = minibuf_read(astr_afmt("Set %s to value: ", astr_cstr(var)), astr_new(""))) == NULL)
          CMDCALL(cancel);
        else
          ok = TRUE;
    }
  }

  if (ok)
    set_variable(var, val);
}
END_DEF
