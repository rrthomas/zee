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
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

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
static Function _last_command;

/*--------------------------------------------------------------------------
 * Key binding.
 *--------------------------------------------------------------------------*/

typedef struct binding *bindingp;

struct binding {
  size_t key;
  Function func;
};

/* Binding vector, number of items, max number of items. */
bindingp *binding = NULL;
static size_t nbindings, max_bindings = 0;

static bindingp get_binding(size_t key)
{
  size_t i;

  for (i = 0; i < nbindings; ++i)
    if (binding[i]->key == key)
      return binding[i];

  return NULL;
}

static void add_binding(bindingp p)
{
  size_t i;

  /* Reallocate vector if there is not enough space. */
  if (nbindings + 1 >= max_bindings) {
    max_bindings += 5;
    binding = zrealloc(binding, sizeof(p) * max_bindings);
  }

  /* Insert the binding at the sorted position. */
  for (i = 0; i < nbindings; i++)
    if (binding[i]->key > p->key) {
      memmove(&binding[i + 1], &binding[i], sizeof(p) * (nbindings - i));
      binding[i] = p;
      break;
    }
  if (i == nbindings)
    binding[nbindings] = p;
  ++nbindings;
}

static void bind_key(size_t key, Function func)
{
  bindingp p, s;

  if ((s = get_binding(key)) == NULL) {
    p = zmalloc(sizeof(*p));
    p->key = key;
    add_binding(p);
    p->func = func;
  } else
    s->func = func;
}

void bind_key_string(char *keystr, Function func)
{
  size_t key;

  if ((key = strtochord(keystr)) != KBD_NOKEY)
    bind_key(key, func);
}

size_t do_completion(astr as)
{
  size_t key;

  minibuf_write("%s", astr_cstr(as));
  key = getkey();
  minibuf_clear();

  return key;
}

void process_key(size_t key)
{
  int uni;
  bindingp p;

  if (key == KBD_NOKEY)
    return;

  if (key & KBD_META && isdigit(key & 255))
    /* Got an ESC x sequence where `x' is a digit. */
    universal_argument(KBD_META, (int)((key & 0xff) - '0'));
  else {
    if ((p = get_binding(key)) == NULL) {
      /* There are no bindings for the pressed key. */
      undo_save(UNDO_START_SEQUENCE, cur_bp->pt, 0, 0);
      for (uni = 0;
           uni < last_uniarg && self_insert_command(key);
           ++uni);
      undo_save(UNDO_END_SEQUENCE, cur_bp->pt, 0, 0);
    } else {
      p->func((lastflag & FLAG_SET_UNIARG) != 0, evalCastIntToLe(last_uniarg));
      _last_command = p->func;
    }
  }

  /* Only add keystrokes if we're already in macro defining mode
     before the function call, to cope with start-kbd-macro. */
  if (lastflag & FLAG_DEFINING_MACRO && thisflag & FLAG_DEFINING_MACRO)
    add_cmd_to_macro();
}

Function last_command(void)
{
  return _last_command;
}

/*--------------------------------------------------------------------------
 * Default functions binding.
 *--------------------------------------------------------------------------*/

struct fentry {
  /* The function name. */
  char *name;

  /* The function pointer. */
  Function func;

  /* The assigned keys. */
  char *key[3];
};

typedef struct fentry *fentryp;

static struct fentry fentry_table[] = {
#define X(cmd_name, c_name) \
	{cmd_name, F_ ## c_name, {NULL, NULL, NULL}},
#include "tbl_funcs.h"
#undef X
};

#define fentry_table_size (sizeof(fentry_table) / sizeof(fentry_table[0]))

static int bind_compar(const void *p1, const void *p2)
{
  return strcmp(((fentryp)p1)->name, ((fentryp)p2)->name);
}

static void recursive_free_bindings(void)
{
  size_t i;
  for (i = 0; i < nbindings; ++i)
    free(binding[i]);
  free(binding);
}

void free_bindings(void)
{
  recursive_free_bindings();
  free_history_elements(&functions_history);
}

static char *bsearch_function(char *name)
{
  struct fentry key, *entryp;
  key.name = name;
  entryp = bsearch(&key, fentry_table, fentry_table_size, sizeof(fentry_table[0]), bind_compar);
  return entryp ? entryp->name : NULL;
}

static fentryp get_fentry(char *name)
{
  size_t i;
  assert(name);
  for (i = 0; i < fentry_table_size; ++i)
    if (!strcmp(name, fentry_table[i].name))
      return &fentry_table[i];
  return NULL;
}

Function get_function(char *name)
{
  fentryp f = get_fentry(name);
  return f ? f->func : NULL;
}

char *get_function_name(Function p)
{
  size_t i;
  for (i = 0; i < fentry_table_size; ++i)
    if (fentry_table[i].func == p)
      return fentry_table[i].name;
  return NULL;
}

static astr bindings_string(fentryp f)
{
  size_t i;
  astr as = astr_new();

  for (i = 0; i < 3; ++i) {
    astr key = simplify_key(f->key[i]);
    if (astr_len(key) > 0) {
        astr_cat_cstr(as, (i == 0) ? "" : ", ");
        astr_cat(as, key);
    }
    astr_delete(key);
  }

  return as;
}

/*
 * Read a function name from the minibuffer.
 * The returned buffer must be freed by the caller.
 */
char *minibuf_read_function_name(const char *fmt, ...)
{
  va_list ap;
  size_t i;
  char *buf, *ms;
  list p;
  Completion *cp;

  va_start(ap, fmt);
  buf = minibuf_format(fmt, ap);
  va_end(ap);

  cp = completion_new(FALSE);
  for (i = 0; i < fentry_table_size; ++i)
    list_append(cp->completions, zstrdup(fentry_table[i].name));

  for (;;) {
    ms = minibuf_read_completion(buf, "", cp, &functions_history);

    if (ms == NULL) {
      free_completion(cp);
      cancel();
      return NULL;
    }

    if (ms[0] == '\0') {
      free_completion(cp);
      minibuf_error("No function name given");
      return NULL;
    } else {
      astr as = astr_new();
      astr_cpy_cstr(as, ms);
      /* Complete partial words if possible. */
      if (completion_try(cp, as, FALSE) == COMPLETION_MATCHED)
        ms = cp->match;
      astr_delete(as);
      for (p = list_first(cp->completions); p != cp->completions;
           p = list_next(p))
        if (!strcmp(ms, p->item)) {
          ms = p->item;
          break;
        }
      if (bsearch_function(ms) || get_macro(ms)) {
        add_history_element(&functions_history, ms);
        minibuf_clear();        /* Remove any error message. */
        ms = zstrdup(ms);       /* Might be about to be freed. */
        break;
      } else {
        minibuf_error("Undefined function name `%s'", ms);
        waitkey(WAITKEY_DEFAULT);
      }
    }
  }

  free_completion(cp);

  return ms;
}

static int execute_function(char *name, int uniarg)
{
  Function func;
  Macro *mp;

  if ((func = get_function(name)))
    return func(uniarg ? 1 : 0, evalCastIntToLe(uniarg));
  else if ((mp = get_macro(name)))
    return call_macro(mp);
  else
    return FALSE;
}

DEFUN_INT("execute-extended-command", execute_extended_command)
/*+
Read function name, then read its arguments and call it.
+*/
{
  char *name;
  astr msg = astr_new();

  if (uniused && uniarg != 0)
    astr_afmt(msg, "%d M-x ", uniarg);
  else
    astr_cat_cstr(msg, "M-x ");

  name = minibuf_read_function_name(astr_cstr(msg));
  astr_delete(msg);
  if (name == NULL)
    return FALSE;

  ok = execute_function(name, uniarg);
  free(name);
}
END_DEFUN

DEFUN("global-set-key", global_set_key)
/*+
Bind a command to a key sequence.
Read key sequence and function name, and bind the function to the key
sequence.
+*/
{
  size_t key = KBD_NOKEY;
  char *name = NULL, *keystr = NULL;

  ok = FALSE;

  if (uniused) {
    if (argc == 3) {
      keystr = evaluateNode(branch->list_next)->data;
      name = evaluateNode(branch->list_next->list_next)->data;
    }
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

    if ((func = get_function(name))) {
      ok = TRUE;
      if (uniused)
        bind_key_string(keystr, func);
      else
        bind_key(key, func);
    } else
      minibuf_error("No such function `%d'", name);

    free(name);
  }
}
END_DEFUN

DEFUN_INT("where-is", where_is)
/*+
Print message listing key sequences that invoke the command DEFINITION.
Argument is a command definition, usually a symbol with a function definition.
If INSERT (the prefix arg) is non-nil, insert the message in the buffer.
+*/
{
  char *name;
  fentryp f;

  name = minibuf_read_function_name("Where is command: ");

  if (name == NULL || (f = get_fentry(name)) == NULL)
    ok = FALSE;
  else {
    if (f->key[0]) {
      astr bindings = bindings_string(f);
      astr as = astr_new();

      astr_afmt(as, "%s is on %s", name, astr_cstr(bindings));
      if (uniused)
        bprintf("%s", astr_cstr(as));
      else
        minibuf_write("%s", astr_cstr(as));

      astr_delete(as);
      astr_delete(bindings);
    } else
      minibuf_write("%s is not on any key", name);
  }
}
END_DEFUN

char *get_function_by_key_sequence(void)
{
  bindingp p;
  size_t key = getkey();

  if (key & KBD_META && isdigit(key & 255))
    return "universal-argument";

  p = get_binding(key);
  if (p == NULL) {
    if (key == KBD_RET || key == KBD_TAB || key <= 255)
      return "self-insert-command";
    else
      return NULL;
  } else
    return get_function_name(p->func);
}
