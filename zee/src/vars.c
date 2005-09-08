/* Lisp variables
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

#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"
#include "vars.h"
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

void variableSetNumber(le **varlist, char *key, int value)
{
  char *buf;

  zasprintf(&buf, "%d", value);
  variableSetString(varlist, key, buf);
  free(buf);
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
