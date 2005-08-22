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


static list line_indent;     /* Current previous indentation levels */
static list toktype_buf;            /* Pushed back token types */
static list tok_buf;            /* Pushed back tokens */
static size_t indent, line;
static ptrdiff_t pos;
static int bol = TRUE;
static astr expr;

void lisp_parse_init(astr as)
{
  pos = 0;
  assert((expr = as));
  indent = 0;
  line = 0;
  bol = TRUE;
  line_indent = list_new();
  toktype_buf = list_new();
  tok_buf = list_new();
}

void lisp_parse_end(void)
{
  list_delete(line_indent);
  list_delete(toktype_buf);
  list_delete(tok_buf);
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
      if (bol)
        indent++;
      break;

    case '\t':
      if (bol)
        indent += 8 - indent % 8;
      break;

    case '#':
      /* Skip comment */
      do {
        c = getachar();
      } while (c != EOF && c != '\n');
      /* FALLTHROUGH */

    case '\n':
      indent = 0;
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
      if (bol) {
        bol = FALSE;
        if (indent <= (size_t)(list_head(line_indent))) {
          while (!list_empty(line_indent) && indent <= (size_t)(list_head(line_indent))) {
            (void)list_behead(line_indent);
            list_append(toktype_buf, (void *)T_CLOSE);
            list_append(tok_buf, astr_new());
          }
        }
        list_prepend(line_indent, (void *)indent);
        list_append(toktype_buf, (void *)T_OPEN);
        list_append(tok_buf, astr_new());
      }
      return c;
    }
  } while (TRUE);
}

static astr gettok(enum tokenname *toktype)
{
  int c;

  if (list_empty(toktype_buf)) {
    astr tok = astr_new();

    switch ((c = getchar_skipspace())) {
    case '\'':
      *toktype = T_QUOTE;
      break;

    case EOF:
      *toktype = T_CLOSE;
      break;

    case '\"':                    /* string */
      do {
        switch ((c = getachar())) {
        case '\n':
        case EOF:
          ungetachar();
          /* FALLTHROUGH */
        case '\"':
          *toktype = T_WORD;
          break;
        default:
          astr_cat_char(tok, (char)c);
        }
      } while (*toktype != T_WORD);
      break;

    default:                      /* word */
      do {
        astr_cat_char(tok, (char)c);

        if (c == '#' || c == ' ' || c == '\n' || c == EOF) {
          ungetachar();

          if (!astr_cmp_cstr(tok, "quote"))
            *toktype = T_QUOTE;
          else {
            astr_truncate(tok, -1);
            *toktype = T_WORD;
          }

          break;
        }

        c = getachar();
      } while (TRUE);
    }

    list_append(toktype_buf, (void *)*toktype);
    list_append(tok_buf, tok);
  }

  *toktype = (enum tokenname)list_behead(toktype_buf);
  return (astr)list_behead(tok_buf);
}

struct le *lisp_parse(struct le *list)
{
  int isquoted = FALSE;

  do {
    enum tokenname toktype = T_CLOSE;
    astr tok = gettok(&toktype);

    switch (toktype) {
    case T_CLOSE:
      astr_delete(tok);
      return list;

    case T_OPEN:
      list = leAddBranchElement(list, lisp_parse(NULL), isquoted);
      break;

    case T_QUOTE:
      isquoted = TRUE;
      break;

    case T_WORD:
      list = leAddDataElement(list, astr_cstr(tok), isquoted);
      break;
    }

    astr_delete(tok);
  } while (TRUE);
}
