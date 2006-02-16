/* Key bindings and extended commands
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2006 Reuben Thomas.
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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"

static History functions_history;

/*--------------------------------------------------------------------------
 * Key binding
 *--------------------------------------------------------------------------*/

/* Bindings vector */
vector *bindings;

static Binding *get_binding(size_t key)
{
  size_t i;

  for (i = 0; i < vec_items(bindings); i++)
    if (vec_item(bindings, i, Binding).key == key)
      return (Binding *)vec_index(bindings, i);

  return NULL;
}

static void add_binding(size_t key, Function func)
{
  Binding b;

  b.key = key;
  b.func = func;

  vec_item(bindings, vec_items(bindings), Binding) = b;
}

void bind_key(size_t key, Function func)
{
  Binding *p;

  assert(key != KBD_NOKEY);

  if ((p = get_binding(key)) == NULL)
    add_binding(key, func);
  else
    p->func = func;
}

static void unbind_key(size_t key)
{
  Binding *p;

  if ((p = get_binding(key)) != NULL)
    vec_shrink(bindings, vec_offset(bindings, p, Binding), 1);
}

void init_bindings(void)
{
  bindings = vec_new(sizeof(Binding));
}

/* FIXME: Handling of universal arg should be done exclusively by
   universal-argument. */
void process_key(size_t key)
{
  int uni;
  Binding *p = get_binding(key);

  if (key == KBD_NOKEY)
    return;

  if (p == NULL)
    undo_save(UNDO_START_SEQUENCE, buf.pt, 0, 0);
  for (uni = 0;
       uni < uniarg &&
         (p ? p->func(0, 0, NULL) : FUNCALL_INT(self_insert_command, (int)key));
       uni++);
  if (p == NULL)
    undo_save(UNDO_END_SEQUENCE, buf.pt, 0, 0);

  /* Only add keystrokes if we're already in macro defining mode
     before the function call, to cope with start-kbd-macro */
  if (lastflag & FLAG_DEFINING_MACRO && thisflag & FLAG_DEFINING_MACRO)
    add_cmd_to_macro();
}

/*--------------------------------------------------------------------------
 * Function name to C function mapping
 *--------------------------------------------------------------------------*/

typedef struct {
  const char *name;             /* The function name */
  Function func;                /* The function pointer */
} FEntry;

static FEntry ftable[] = {
#define X(cmd_name) \
	{# cmd_name, F_ ## cmd_name},
#include "tbl_funcs.h"
#undef X
};

#define fentries (sizeof(ftable) / sizeof(ftable[0]))

Function get_function(astr name)
{
  size_t i;
  if (name)
    for (i = 0; i < fentries; i++)
      if (strcmp(astr_cstr(name), ftable[i].name) == 0)
        return ftable[i].func;
  return NULL;
}

static astr get_function_name(Function f)
{
  size_t i;
  for (i = 0; i < fentries; i++)
    if (ftable[i].func == f)
      return astr_new(ftable[i].name);
  return NULL;
}

/*
 * Read a function name from the minibuffer.
 * The returned buffer must be freed by the caller.
 */
astr minibuf_read_function_name(astr as)
{
  size_t i;
  astr ms;
  list p;
  Completion *cp;

  cp = completion_new();
  for (i = 0; i < fentries; ++i)
    list_append(cp->completions, astr_new(ftable[i].name));

  for (;;) {
    ms = minibuf_read_completion(as, astr_new(""), cp, &functions_history);

    if (ms == NULL) {
      FUNCALL(cancel);
      break;
    } else if (astr_len(ms) == 0) {
      minibuf_error(astr_new("No function name given"));
      ms = NULL;
      break;
    } else {
      astr as = astr_dup(ms);
      /* Complete partial words if possible */
      if (completion_try(cp, as, FALSE) == COMPLETION_MATCHED)
        ms = astr_dup(cp->match);
      for (p = list_first(cp->completions); p != cp->completions;
           p = list_next(p))
        if (!astr_cmp(ms, p->item)) {
          ms = astr_dup(p->item);
          break;
        }
      if (get_function(ms) || get_macro(ms)) {
        add_history_element(&functions_history, ms);
        minibuf_clear();        /* Remove any error message */
        break;
      } else {
        minibuf_error(astr_afmt("Undefined function name `%s'", astr_cstr(ms)));
        waitkey(WAITKEY_DEFAULT);
      }
    }
  }

  return ms;
}

DEFUN(unbind_key)
{
/*+
Unbind a key.
Read key chord, and unbind it.
+*/
  size_t key = KBD_NOKEY;

  if (argc > 0) {
    astr keystr = astr_new((char *)((*lp)->item));
    key = strtochord(keystr);
    *lp = list_next(*lp);
  } else {
    minibuf_write(astr_new("Unbind key: "));
    key = getkey();
  }

  unbind_key(key);
}
END_DEFUN

DEFUN(bind_key)
/*+
Bind a command to a key chord.
Read key chord and function name, and bind the function to the key
chord.
+*/
{
  size_t key = KBD_NOKEY;
  astr name = NULL;

  ok = FALSE;

  if (argc > 0) {
    key = strtochord(astr_new((char *)((*lp)->item)));
    *lp = list_next(*lp);
    name = astr_new((char *)((*lp)->item));
    *lp = list_next(*lp);
  } else {
    astr as;

    minibuf_write(astr_new("Bind key: "));
    key = getkey();

    as = chordtostr(key);
    name = minibuf_read_function_name(astr_afmt("Bind key %s to command: ", astr_cstr(as)));
  }

  if (name) {
    Function func;

    if ((func = get_function(name))) {
      ok = TRUE;
      if (key != KBD_NOKEY)
        bind_key(key, func);
      else
        minibuf_error(astr_new("Invalid key"));
    } else
      minibuf_error(astr_afmt("No such function `%s'", astr_cstr(name)));
  }
}
END_DEFUN

static astr function_to_binding(Function f)
{
  size_t i, n = 0;
  astr as = astr_new("");

  for (i = 0; i < vec_items(bindings); i++)
    if (vec_item(bindings, i, Binding).func == f) {
      size_t key = vec_item(bindings, i, Binding).key;
      astr binding = chordtostr(key);
      astr_cat_cstr(as, (n++ == 0) ? "" : ", ");
      astr_cat(as, binding);
    }

  return as;
}

DEFUN(where_is)
/*+
Print message listing key sequences that invoke the command DEFINITION.
Argument is a command definition, usually a symbol with a function definition.
+*/
{
  astr name;
  Function f;

  name = minibuf_read_function_name(astr_new("Where is command: "));

  if ((f = get_function(name)) == NULL)
    ok = FALSE;
  else {
    astr bindings = function_to_binding(f);

    if (astr_len(bindings) > 0)
      minibuf_write(astr_afmt("%s is on %s", astr_cstr(name), astr_cstr(bindings)));
    else
      minibuf_write(astr_afmt("%s is not on any key", astr_cstr(name)));
  }
}
END_DEFUN

astr binding_to_function(size_t key)
{
  Binding *p;

  if (key & KBD_META && isdigit(key & 255))
    return astr_new("universal_argument");

  if ((p = get_binding(key)) == NULL) {
    if (key == KBD_RET || key == KBD_TAB || key <= 255)
      return astr_new("self_insert_command");
    else
      return NULL;
  } else
    return get_function_name(p->func);
}
