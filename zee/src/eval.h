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

#ifndef EVAL_H
#define EVAL_H

#include "lists.h"

typedef le *(*eval_cb)(le **list);
typedef struct evalLookupNode {
  char *word;
  eval_cb callback;
} evalLookupNode;

eval_cb lookupFunction(char *name);
le *evaluateNode(le **node);

int evalCastLeToInt(const le *levalue);
le *evalCastIntToLe(int intvalue);

#endif
