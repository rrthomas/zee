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
        tok = astr_sub(tok, 0, (ptrdiff_t)astr_len(tok) - 1);
        break;
      }
      c = getch();
    } while (TRUE);
  }

  return tok;
}

/*
 * Execute a string as commands
 */
void cmd_eval(astr as)
{
  pos = 0;
  assert((expr = as));
  line = 0;
  bol = TRUE;

  astr tok = gettok();
  while (tok) {
    Command cmd;
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
           (cmd = get_command(fname)) &&
           cmd(l))
      ;

    /* The first token for the next line, if any, was read above */
  }
  expr = NULL;
}

/*--------------------------------------------------------------------------
 * Command name to C function mapping
 *--------------------------------------------------------------------------*/

static struct {
  const char *name;             /* The command name */
  Command cmd;                  /* The function pointer */
} ftable[] = {
#define X(cmd_name, doc) \
	{# cmd_name, F_ ## cmd_name},
#include "tbl_funcs.h"
#undef X
};
#define fentries (sizeof(ftable) / sizeof(ftable[0]))

Command get_command(astr name)
{
  size_t i;
  if (name)
    for (i = 0; i < fentries; i++)
      if (!astr_cmp(name, astr_new(ftable[i].name)))
        return ftable[i].cmd;
  return NULL;
}

astr get_command_name(Command cmd)
{
  size_t i;
  for (i = 0; i < fentries; i++)
    if (ftable[i].cmd == cmd)
      return astr_new(ftable[i].name);
  return NULL;
}

list command_list(void)
{
  list l = list_new();
  for (size_t i = 0; i < fentries; ++i)
    list_append(l, astr_new(ftable[i].name));
  return l;
}
