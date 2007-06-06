/* Memory allocation functions
   Copyright (c) 2005-2006 Reuben Thomas.
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

#ifndef DEBUG
#include <gc/gc.h>
#else
#include <stdlib.h>
#endif

#include <stdio.h>
#include <string.h>

#include "zmalloc.h"


/*
 * Resize an allocated memory area. Bytes that are no longer needed are zeroed
 * before being freed. Bytes that are newly needed are zeroed after being
 * allocated.
 */
void *zrealloc(void *ptr, size_t oldsize, size_t newsize)
{
  if (newsize < oldsize)
    memset((char *)ptr + newsize, 0, oldsize - newsize);

  // Behaviour in this case is unspecified for malloc and GC_REALLOC
  if (newsize == 0)
    return NULL;

#ifndef DEBUG
  ptr = GC_REALLOC(ptr, newsize);
#else
  ptr = realloc(ptr, newsize);
#endif

  if (ptr == NULL) {
    fprintf(stderr, PACKAGE_NAME ": out of memory\n");
    die(2);
  }

#ifdef DEBUG // GC_REALLOC does this anyway
  if (newsize > oldsize)
    memset((char *)ptr + oldsize, 0, newsize - oldsize);
#endif

  return ptr;
}
