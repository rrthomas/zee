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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"

/*
 * Write the specified string in the minibuffer.
 */
void minibuf_write(const char *s)
{
  term_minibuf_write(s);

  /* Redisplay (and leave the cursor in the correct position). */
  term_display();
  term_refresh();
}

/*
 * Write the specified error string in the minibuffer and beep.
 */
void minibuf_error(const char *s)
{
  minibuf_write(s);
  ding();
}

/*
 * Read a string from the minibuffer.
 */
astr minibuf_read(const char *s, const char *value)
{
  return term_minibuf_read(s, value ? value : "", NULL, NULL);
}

static int minibuf_read_forced(const char *s, const char *errmsg, Completion *cp)
{
  astr as;

  for (;;) {
    as = term_minibuf_read(s, "", cp, NULL);
    if (as == NULL)             /* Cancelled. */
      return -1;
    else {
      list s;
      int i;
      astr bs = astr_new();

      /* Complete partial words if possible. */
      astr_cpy(bs, as);
      if (completion_try(cp, bs, FALSE) == COMPLETION_MATCHED)
        astr_cpy_cstr(as, cp->match);

      for (s = list_first(cp->completions), i = 0; s != cp->completions;
           s = list_next(s), i++)
        if (!astr_cmp_cstr(as, s->item))
          return i;

      minibuf_error(errmsg);
      waitkey(WAITKEY_DEFAULT);
    }
  }
}

int minibuf_read_yesno(const char *s)
{
  Completion *cp;
  int retvalue;

  cp = completion_new();
  list_append(cp->completions, zstrdup("yes"));
  list_append(cp->completions, zstrdup("no"));

  retvalue = minibuf_read_forced(s, "Please answer yes or no.", cp);
  if (retvalue != -1) {
    /* The completions may be sorted by the minibuf completion
       routines. */
    if (!strcmp(list_at(cp->completions, (size_t)retvalue), "yes"))
      retvalue = TRUE;
    else
      retvalue = FALSE;
  }

  return retvalue;
}

int minibuf_read_boolean(const char *s)
{
  int retvalue;
  Completion *cp = completion_new();

  list_append(cp->completions, zstrdup("true"));
  list_append(cp->completions, zstrdup("false"));

  retvalue = minibuf_read_forced(s, "Please answer `true' or `false'.", cp);
  if (retvalue != -1) {
    /* The completions may be sorted by the minibuf completion
       routines. */
    if (!strcmp(list_at(cp->completions, (size_t)retvalue), "true"))
      retvalue = TRUE;
    else
      retvalue = FALSE;
  }

  return retvalue;
}

/*
 * Read a string from the minibuffer using a completion.
 */
astr minibuf_read_completion(const char *s, char *value, Completion *cp, History *hp)
{
  return term_minibuf_read(s, value, cp, hp);
}

/*
 * Clear the minibuffer.
 */
void minibuf_clear(void)
{
  term_minibuf_write("");
}
