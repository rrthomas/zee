/* Lisp main routine
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

#include <stdio.h>
#include <assert.h>
#include "main.h"
#include "extern.h"
#include "eval.h"


le *lisp_read(astr as)
{
  struct le *list;

  lisp_parse_init(as);
  list = lisp_parse(NULL);
  lisp_parse_end();

  return list;
}


le *lisp_read_file(const char *file)
{
  le *list;
  FILE *fp = fopen(file, "r");
  astr as;

  if (fp == NULL)
    return NULL;

  as = astr_fread(fp);
  fclose(fp);
  list = lisp_read(as);
  astr_delete(as);

  return list;
}
