/* Command parser
   Copyright (c) 2001 Scott "Jerry" Lawrence.
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
#include "main.h"
#include "extern.h"


static size_t line;
static ptrdiff_t pos;
static int bol;
static astr expr;

static int getch(void)
{
  if ((size_t)pos < astr_len(expr))
    return *astr_char(expr, pos++);
  return EOF;
}

static void ungetch(void)
{
  if (pos > 0 && (size_t)pos < astr_len(expr))
    pos--;
}

/*
 * Skip space to next non-space character
 */
static int getch_skipspace(void) {
  do {
    int c = getch();

    switch (c) {
    case ' ':
    case '\t':
      break;

    case '#':
      /* Skip comment */
      do {
        c = getch();
      } while (c != EOF && c != '\n');
      /* FALLTHROUGH */

    case '\n':
      bol = TRUE;
      line++;
      break;

    case '\\':
      if ((c = getch()) == '\n')
        line++;                 /* For source position, count \n */
      else
        ungetch();
      break;

    default:
      return c;
    }
  } while (TRUE);
}

static astr gettok(void)
{
  int c;
  astr tok = astr_new("");

  switch ((c = getch_skipspace())) {
  case EOF:
    return NULL;

  case '\"':                    /* string */
    {
      int eos = FALSE;

      do {
        switch ((c = getch())) {
        case '\n':
        case EOF:
          ungetch();
          /* FALLTHROUGH */
        case '\"':
          eos = TRUE;
          break;
        default:
          astr_cat_char(tok, c);
        }
      } while (!eos);
    }
    break;

  default:                      /* word */
    do {
      astr_cat_char(tok, c);
      if (c == '#' || c == ' ' || c == '\n' || c == EOF) {
        ungetch();
        astr_truncate(tok, -1);
        break;
      }
      c = getch();
    } while (TRUE);
  }

  return tok;
}

void cmd_parse_init(astr as)
{
  pos = 0;
  assert((expr = as));
  line = 0;
  bol = TRUE;
}

void cmd_parse_end(void)
{
  expr = NULL;
}

void cmd_eval(void)
{
  Function func;
  astr tok = gettok();

  while (tok) {
    list l = list_new();
    astr fname;

    /* Get tokens until we run out or reach a new line */
    while (tok && !bol) {
      list_append(l, tok);
      tok = gettok();
    }
    bol = FALSE;

    /* Execute the line */
    while ((fname = list_behead(l)) &&
           (func = get_function(fname)) &&
           func(list_length(l), 0, l))
      ;

    /* The first token for the next line, if any, was read above */
  }
}

void cmd_eval_file(astr file)
{
  astr as;

  file_read(&as, file);
  cmd_parse_init(as);
  cmd_eval();
  cmd_parse_end();
}
