/* Lisp parser
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
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"


static size_t line;
static ptrdiff_t pos;
/* FIXME: either use \n tokens to limit each exp to one line, or
   remove bol */
static int bol = TRUE;
static astr expr;

void cmd_parse_init(astr as)
{
  pos = 0;
  assert((expr = as));
  line = 0;
  bol = TRUE;
}

static int getachar(void)
{
  if ((size_t)pos < astr_len(expr))
    return *astr_char(expr, pos++);
  return EOF;
}

static void ungetachar(void)
{
  if (pos > 0 && (size_t)pos < astr_len(expr))
    pos--;
}

/*
 * Skip space to next non-space character
 */
static int getchar_skipspace(void) {
  do {
    int c = getachar();

    switch (c) {
    case ' ':
    case '\t':
      break;

    case '#':
      /* Skip comment */
      do {
        c = getachar();
      } while (c != EOF && c != '\n');
      /* FALLTHROUGH */

    case '\n':
      bol = TRUE;
      line++;
      break;

    case '\\':
      if ((c = getachar()) == '\n')
        line++;                 /* For source position, count \n */
      else
        ungetachar();
      break;

    default:
      return c;
    }
  } while (TRUE);
}

static astr gettok(int *eof)
{
  int c;

  astr tok = astr_new();

  switch ((c = getchar_skipspace())) {
  case EOF:
    *eof = TRUE;
    break;

  case '\"':                    /* string */
    {
      int eow = FALSE;

      do {
        switch ((c = getachar())) {
        case '\n':
        case EOF:
          ungetachar();
          /* FALLTHROUGH */
        case '\"':
          eow = TRUE;
          break;
        default:
          astr_cat_char(tok, (char)c);
        }
      } while (!eow);
    }
    break;

  default:                      /* word */
    do {
      astr_cat_char(tok, (char)c);

      if (c == '#' || c == ' ' || c == '\n' || c == EOF) {
        ungetachar();

        astr_truncate(tok, -1);
        break;
      }

      c = getachar();
    } while (TRUE);
  }

  return tok;
}

void cmd_parse(list lp)
{
  int eof = FALSE;
  astr tok;

  do {
    tok = gettok(&eof);
    if (!eof)
      list_append(lp, (void *)astr_cstr(tok));
  } while (!eof);

  astr_delete(tok);
}

list cmd_read(astr as)
{
  list lp = list_new();

  cmd_parse_init(as);
  cmd_parse(lp);

  return lp;
}

list cmd_read_file(const char *file)
{
  list lp;
  FILE *fp = fopen(file, "r");
  astr as;

  if (fp == NULL)
    return NULL;

  as = astr_fread(fp);
  fclose(fp);
  lp = cmd_read(as);
  astr_delete(as);

  return lp;
}
