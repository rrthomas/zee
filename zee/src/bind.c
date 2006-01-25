/* Key bindings and extended commands
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2005 Reuben Thomas.
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
#include <stdarg.h>
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

void init_bindings(void)
{
  bindings = vec_new(sizeof(Binding));
}

void free_bindings(void)
{
  vec_delete(bindings);
  free_history_elements(&functions_history);
}

/* FIXME: Handling of uniarg should be done exclusively by
   universal-argument. */
void process_key(size_t key)
{
  int uni;
  Binding *p = get_binding(key);

  if (key == KBD_NOKEY)
    return;

  if (p == NULL)
    undo_save(UNDO_START_SEQUENCE, buf.pt, 0, 0, FALSE);
  for (uni = 0;
       uni < last_uniarg &&
         (p ? p->func(0, 0, NULL) : self_insert_command(key));
       uni++);
  if (p == NULL)
    undo_save(UNDO_END_SEQUENCE, buf.pt, 0, 0, FALSE);

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
#define X(cmd_name, c_name) \
	{cmd_name, F_ ## c_name},
#include "tbl_funcs.h"
#undef X
};

#define fentries (sizeof(ftable) / sizeof(ftable[0]))

static int fentry_compar(const void *p1, const void *p2)
{
  return strcmp(((FEntry *)p1)->name, ((FEntry *)p2)->name);
}

static const char *bsearch_function(const char *name)
{
  FEntry key, *entryp;
  key.name = name;
  entryp = bsearch(&key, ftable, fentries, sizeof(ftable[0]), fentry_compar);
  return entryp ? entryp->name : NULL;
}

Function get_function(const char *name)
{
  size_t i;
  if (name)
    for (i = 0; i < fentries; i++)
      if (strcmp(name, ftable[i].name) == 0)
        return ftable[i].func;
  return NULL;
}

const char *get_function_name(Function f)
{
  size_t i;
  for (i = 0; i < fentries; i++)
    if (ftable[i].func == f)
      return ftable[i].name;
  return NULL;
}

/*
 * Read a function name from the minibuffer.
 * The returned buffer must be freed by the caller.
 */
astr minibuf_read_function_name(const char *fmt, ...)
{
  va_list ap;
  size_t i;
  char *buf;
  astr ms;
  list p;
  Completion *cp;

  va_start(ap, fmt);
  buf = minibuf_format(fmt, ap);
  va_end(ap);

  cp = completion_new(FALSE);
  for (i = 0; i < fentries; ++i)
    list_append(cp->completions, zstrdup(ftable[i].name));

  for (;;) {
    ms = minibuf_read_completion(buf, "", cp, &functions_history);

    if (ms == NULL) {
      cancel();
      break;
    } else if (astr_len(ms) == 0) {
      minibuf_error("No function name given");
      astr_delete(ms);
      ms = NULL;
      break;
    } else {
      astr as = astr_new();
      astr_cpy(as, ms);
      /* Complete partial words if possible */
      if (completion_try(cp, as, FALSE) == COMPLETION_MATCHED)
        astr_cpy_cstr(ms, cp->match);
      astr_delete(as);
      for (p = list_first(cp->completions); p != cp->completions;
           p = list_next(p))
        if (!astr_cmp_cstr(ms, p->item)) {
          astr_cpy_cstr(ms, p->item);
          break;
        }
      if (bsearch_function(astr_cstr(ms)) || get_macro(astr_cstr(ms))) {
        add_history_element(&functions_history, astr_cstr(ms));
        minibuf_clear();        /* Remove any error message */
        break;
      } else {
        minibuf_error("Undefined function name `%s'", astr_cstr(ms));
        waitkey(WAITKEY_DEFAULT);
      }
    }
  }

  free(buf);
  free_completion(cp);

  return ms;
}

DEFUN("global-set-key", global_set_key)
/*+
Bind a command to a key sequence.
Read key sequence and function name, and bind the function to the key
sequence.
+*/
{
  size_t key = KBD_NOKEY;
  astr keystr = NULL, name = NULL;

  ok = FALSE;

  if (argc > 0) {
    keystr = astr_cpy_cstr(astr_new(), (char *)((*lp)->item));
    key = strtochord(astr_cstr(keystr));
    astr_delete(keystr);
    *lp = list_next(*lp);
    name = astr_cpy_cstr(astr_new(), (char *)((*lp)->item));
    *lp = list_next(*lp);
  } else {
    astr as;

    minibuf_write("Set key globally: ");
    key = getkey();

    as = chordtostr(key);
    name = minibuf_read_function_name("Set key %s to command: ", astr_cstr(as));
    astr_delete(as);
  }

  if (name) {
    Function func;

    if ((func = get_function(astr_cstr(name)))) {
      ok = TRUE;
      if (key != KBD_NOKEY)
        bind_key(key, func);
    } else
      minibuf_error("No such function `%s'", astr_cstr(name));

    astr_delete(name);
  }
}
END_DEFUN

static astr function_to_binding(Function f)
{
  size_t i, n = 0;
  astr as = astr_new();

  for (i = 0; i < vec_items(bindings); i++)
    if (vec_item(bindings, i, Binding).func == f) {
      size_t key = vec_item(bindings, i, Binding).key;
      astr binding = chordtostr(key);
      astr_cat_cstr(as, (n++ == 0) ? "" : ", ");
      astr_cat_delete(as, binding);
    }

  return as;
}

DEFUN_INT("where-is", where_is)
/*+
Print message listing key sequences that invoke the command DEFINITION.
Argument is a command definition, usually a symbol with a function definition.
+*/
{
  astr name;
  Function f;

  name = minibuf_read_function_name("Where is command: ");

  if ((f = get_function(astr_cstr(name))) == NULL)
    ok = FALSE;
  else {
    astr bindings = function_to_binding(f);

    if (astr_len(bindings) > 0)
      minibuf_write("%s is on %s", astr_cstr(name), astr_cstr(bindings));
    else
      minibuf_write("%s is not on any key", astr_cstr(name));

    astr_delete(bindings);
  }
}
END_DEFUN

const char *binding_to_function(size_t key)
{
  Binding *p;

  if (key & KBD_META && isdigit(key & 255))
    return "universal-argument";

  if ((p = get_binding(key)) == NULL) {
    if (key == KBD_RET || key == KBD_TAB || key <= 255)
      return "self-insert-command";
    else
      return NULL;
  } else
    return get_function_name(p->func);
}
