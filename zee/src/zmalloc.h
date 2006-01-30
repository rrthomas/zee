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

#ifndef ZMALLOC_H

#include <stddef.h>
#include <stdarg.h>

/* Prototype here to break dependency cycle with extern.h */
void die(int exitcode);

void *zmalloc(size_t size);
void *zrealloc(void *ptr, size_t oldsize, size_t newsize);
char *zstrdup(const char *s);
int zvasprintf(char **ptr, const char *fmt, va_list vargs);
int zasprintf(char **ptr, const char *fmt, ...);

#endif