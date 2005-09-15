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


int evaluateNode(list *node)
{
  Function prim;

  if (*node == NULL)
    return FALSE;

  assert((*node)->item);
  prim = get_function((char *)((*node)->item));

  *node = list_next(*node);

  if (prim)
    return prim(node);

  return FALSE;
}

void evalList(list lp)
{
  for (lp = list_next(lp); lp->item; evaluateNode(&lp))
    ;
}

int evalCastLeToInt(list levalue)
{
  if (levalue->item == NULL)
    return 0;

  return atoi((char *)(levalue->item));
}

list evalCastIntToLe(int intvalue)
{
  char *buf;
  list lp = list_new();

  zasprintf(&buf, "%d", intvalue);
  lp->item = buf;

  return lp;
}

void leWipe(list lp)
{
  if (lp) {
    list l;

    for (l = list_first(lp); l != lp; l = list_next(l))
      free(l->item);

    list_delete(lp);
  }
}

static int execute_function(const char *name, int uniarg)
{
  Function func;
  Macro *mp;

  if ((func = get_function(name))) {
    list arg = evalCastIntToLe(uniarg);
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
