/* Lisp eval
   Copyright (c) 2001 Scott "Jerry" Lawrence.
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"
#include "eval.h"


static le *eval_cb_command_helper(Function f, le **list)
{
  f(list);
  return *list;
}

#define X(cmd_name, c_name) \
  static le *eval_cb_ ## c_name(le **list) \
  { \
    return eval_cb_command_helper(F_ ## c_name, list); \
  }
#include "tbl_funcs.h"
#undef X


static le *eval_cb_setq(le **list)
{
  char *newvalue = NULL;

  if (*list != NULL) {
    char *name = (*list)->data;
    *list = (*list)->list_next;
    if (*list != NULL) {
      newvalue = (*list)->data;
      *list = (*list)->list_next;
    }

    set_variable(name, newvalue);
  }

  return NULL;
}


static evalLookupNode evalTable[] = {
  { "setq"	, eval_cb_setq		},

#define X(cmd_name, c_name) \
	{ cmd_name, eval_cb_ ## c_name },
#include "tbl_funcs.h"
#undef X

  { NULL	, NULL			}
};


eval_cb lookupFunction(char *name)
{
  int i;
  for (i = 0; evalTable[i].word; i++)
    if (!strcmp(evalTable[i].word, name))
      return evalTable[i].callback;

  return NULL;
}


le *evaluateNode(le **node)
{
  eval_cb prim;

  if (*node == NULL)
    return NULL;

  assert((*node)->data);
  prim = lookupFunction((*node)->data);

  *node = (*node)->list_next;

  if (prim)
    return prim(node);
  else
    return *node;
}


int evalCastLeToInt(const le *levalue)
{
  if (levalue == NULL || levalue->data == NULL)
    return 0;

  return atoi(levalue->data);
}

le *evalCastIntToLe(int intvalue)
{
  char *buf;
  le *list;

  zasprintf(&buf, "%d", intvalue);
  list = leNew(buf);
  free(buf);

  return list;
}


static int execute_function(const char *name, int uniarg)
{
  Function func;
  Macro *mp;

  if ((func = get_function(name))) {
    le *arg = evalCastIntToLe(uniarg);
    return func(uniarg ? &arg : NULL);
  } else if ((mp = get_macro(name)))
    return call_macro(mp);
  else
    return FALSE;
}

DEFUN_INT("execute-extended-command", execute_extended_command)
/*+
Read function name, then read its arguments and call it.
+*/
{
  astr name, msg = astr_new();

  if (uniused && uniarg != 0)
    astr_afmt(msg, "%d M-x ", uniarg);
  else
    astr_cat_cstr(msg, "M-x ");

  name = minibuf_read_function_name(astr_cstr(msg));
  astr_delete(msg);
  if (name == NULL)
    return FALSE;

  ok = execute_function(astr_cstr(name), uniarg);
  astr_delete(name);
}
END_DEFUN
