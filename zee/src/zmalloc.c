/* Memory allocation functions
   Copyright (c) 1997-2004 Sandro Sigala.
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

#include "config.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <gc/gc.h>

#include "main.h"
#include "zmalloc.h"

/*
 * Return a zeroed allocated memory area.
 */
void *zmalloc(size_t size)
{
  void *ptr;

  assert(size > 0);

  if ((ptr = GC_MALLOC(size)) == NULL) {
    fprintf(stderr, PACKAGE_NAME ": cannot allocate memory\n");
    die(1);
  }

  return ptr;
}

/*
 * Resize an allocated memory area.
 */
void *zrealloc(void *ptr, size_t oldsize, size_t newsize)
{
  void *newptr;

  assert(newsize > 0);

  if ((newptr = GC_REALLOC(ptr, newsize)) == NULL) {
    fprintf(stderr, PACKAGE_NAME ": cannot reallocate memory\n");
    die(1);
  }

  if (newsize > oldsize)
    memset((char *)newptr + oldsize, 0, newsize - oldsize);

  return newptr;
}

/*
 * Duplicate a string.
 */
char *zstrdup(const char *s)
{
  return strcpy(zmalloc(strlen(s) + 1), s);
}
