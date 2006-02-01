/* Minibuffer facility functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2004-2005 Reuben Thomas.
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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"

static History files_history;

/*--------------------------------------------------------------------------
 * Minibuffer wrapper functions.
 *--------------------------------------------------------------------------*/

char *minibuf_format(const char *fmt, va_list ap)
{
  char *buf;
  zvasprintf(&buf, fmt, ap);
  return buf;
}

void free_minibuf(void)
{
  free_history_elements(&files_history);
}

static void minibuf_vwrite(const char *fmt, va_list ap)
{
  char *buf;

  buf = minibuf_format(fmt, ap);

  term_minibuf_write(buf);
  free(buf);

  /* Redisplay (and leave the cursor in the correct position). */
  term_display();
  term_refresh();
}

/*
 * Write the specified string in the minibuffer.
 */
void minibuf_write(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  minibuf_vwrite(fmt, ap);
  va_end(ap);
}

/*
 * Write the specified error string in the minibuffer and beep.
 */
void minibuf_error(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  minibuf_vwrite(fmt, ap);
  va_end(ap);

  ding();
}

/*
 * Read a string from the minibuffer.
 */
astr minibuf_read(const char *fmt, const char *value, ...)
{
  va_list ap;
  char *buf;
  astr as;

  va_start(ap, value);
  buf = minibuf_format(fmt, ap);
  va_end(ap);

  as = term_minibuf_read(buf, value ? value : "", NULL, NULL);
  free(buf);

  return as;
}

static int minibuf_read_forced(const char *fmt, const char *errmsg,
                               Completion *cp, ...)
{
  va_list ap;
  char *buf;
  astr as;

  va_start(ap, cp);
  buf = minibuf_format(fmt, ap);
  va_end(ap);

  for (;;) {
    as = term_minibuf_read(buf, "", cp, NULL);
    if (as == NULL) { /* Cancelled. */
      free(buf);
      return -1;
    } else {
      list s;
      int i;
      astr bs = astr_new();

      /* Complete partial words if possible. */
      astr_cpy(bs, as);
      if (completion_try(cp, bs, FALSE) == COMPLETION_MATCHED)
        astr_cpy_cstr(as, cp->match);
      astr_delete(bs);

      for (s = list_first(cp->completions), i = 0; s != cp->completions;
           s = list_next(s), i++)
        if (!astr_cmp_cstr(as, s->item)) {
          astr_delete(as);
          free(buf);
          return i;
        }

      minibuf_error(errmsg);
      waitkey(WAITKEY_DEFAULT);
    }
  }
}

int minibuf_read_yesno(const char *fmt, ...)
{
  va_list ap;
  char *buf;
  Completion *cp;
  int retvalue;

  va_start(ap, fmt);
  buf = minibuf_format(fmt, ap);
  va_end(ap);

  cp = completion_new();
  list_append(cp->completions, zstrdup("yes"));
  list_append(cp->completions, zstrdup("no"));

  retvalue = minibuf_read_forced(buf, "Please answer yes or no.", cp);
  if (retvalue != -1) {
    /* The completions may be sorted by the minibuf completion
       routines. */
    if (!strcmp(list_at(cp->completions, (size_t)retvalue), "yes"))
      retvalue = TRUE;
    else
      retvalue = FALSE;
  }
  free_completion(cp);
  free(buf);

  return retvalue;
}

int minibuf_read_boolean(const char *fmt, ...)
{
  va_list ap;
  char *buf;
  Completion *cp;
  int retvalue;

  va_start(ap, fmt);
  buf = minibuf_format(fmt, ap);
  va_end(ap);

  cp = completion_new();
  list_append(cp->completions, zstrdup("true"));
  list_append(cp->completions, zstrdup("false"));

  retvalue = minibuf_read_forced(buf, "Please answer true or false.", cp);
  if (retvalue != -1) {
    /* The completions may be sorted by the minibuf completion
       routines. */
    if (!strcmp(list_at(cp->completions, (size_t)retvalue), "true"))
      retvalue = TRUE;
    else
      retvalue = FALSE;
  }
  free_completion(cp);
  free(buf);

  return retvalue;
}

/*
 * Read a string from the minibuffer using a completion.
 */
astr minibuf_read_completion(const char *fmt, char *value, Completion *cp, History *hp, ...)
{
  va_list ap;
  char *buf;
  astr as;

  va_start(ap, hp);
  buf = minibuf_format(fmt, ap);
  va_end(ap);

  as = term_minibuf_read(buf, value, cp, hp);
  free(buf);

  return as;
}

/*
 * Clear the minibuffer.
 */
void minibuf_clear(void)
{
  term_minibuf_write("");
}
