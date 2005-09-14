/* Lisp lists
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
#include <assert.h>

#include "main.h"
#include "extern.h"
#include "lists.h"
#include "eval.h"


le *leNew(const char *text)
{
  le *new = (le *)zmalloc(sizeof(le));

  new->data = text ? zstrdup(text) : NULL;
  new->quoted = 0;
  new->list_prev = NULL;
  new->list_next = NULL;

  return new;
}

void leWipe(le *list)
{
  if (list) {
    /* free descendants */
    leWipe(list->list_next);

    /* free ourself */
    free(list->data);
    free(list);
  }
}

le *leAddHead(le *list, le *element)
{
  if (!element)
    return list;

  element->list_next = list;
  if (list)
    list->list_prev = element;
  return element;
}

le *leAddTail(le *list, le *element)
{
  le *temp = list;

  /* if either element or list doesn't exist, return the 'new' list */
  if (!element)
    return list;
  if (!list)
    return element;

  /* find the end element of the list */
  while (temp->list_next)
    temp = temp->list_next;

  /* tack ourselves on */
  temp->list_next = element;
  element->list_prev = temp;

  /* return the list */
  return list;
}


le *leAddDataElement(le *list, const char *data, int quoted)
{
  le *newdata = leNew(data);
  assert(newdata);
  newdata->quoted = quoted;
  return leAddTail(list, newdata);
}

le *leDup(le *list)
{
  le *temp;
  if (!list)
    return NULL;

  temp = leNew(list->data);
  temp->list_next = leDup(list->list_next);

  if (temp->list_next)
    temp->list_next->list_prev = temp;

  return temp;
}

void leEval(le *list)
{
  while (list)
    evaluateNode(&list);
}
