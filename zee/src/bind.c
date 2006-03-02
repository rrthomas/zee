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

#include <ctype.h>

#include "main.h"
#include "extern.h"


static History commands_history;

/*--------------------------------------------------------------------------
 * Key binding
 *--------------------------------------------------------------------------*/

vector *bindings;               /* Vector of Binding *s */

static Binding *get_binding(size_t key)
{
  size_t i;

  for (i = 0; i < vec_items(bindings); i++)
    if (vec_item(bindings, i, Binding).key == key)
      return (Binding *)vec_index(bindings, i);

  return NULL;
}

static void add_binding(size_t key, Command cmd)
{
  Binding b;

  b.key = key;
  b.cmd = cmd;

  vec_item(bindings, vec_items(bindings), Binding) = b;
}

void bind_key(size_t key, Command cmd)
{
  Binding *p;

  assert(key != KBD_NOKEY);

  if ((p = get_binding(key)) == NULL)
    add_binding(key, cmd);
  else
    p->cmd = cmd;
}

static void unbind_key(size_t key)
{
  Binding *p;

  if ((p = get_binding(key)))
    vec_shrink(bindings, vec_offset(bindings, p), 1);
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
    undo_save(UNDO_START_SEQUENCE, buf->pt, 0, 0);
  for (uni = 0;
       uni < uniarg &&
         (p ? p->cmd(list_new()) : CMDCALL_INT(self_insert_command, (int)key));
       uni++);
  if (p == NULL)
    undo_save(UNDO_END_SEQUENCE, buf->pt, 0, 0);

  /* Only add keystrokes if we're already in macro defining mode
     before the command call, to cope with start-kbd-macro */
  if (lastflag & FLAG_DEFINING_MACRO && thisflag & FLAG_DEFINING_MACRO)
    add_cmd_to_macro();
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

static astr get_command_name(Command f)
{
  size_t i;
  for (i = 0; i < fentries; i++)
    if (ftable[i].cmd == f)
      return astr_new(ftable[i].name);
  return NULL;
}

/*
 * Read a command name from the minibuffer.
 */
astr minibuf_read_command_name(astr as)
{
  astr ms;
  Completion *cp = completion_new();

  for (size_t i = 0; i < fentries; ++i)
    list_append(cp->completions, astr_new(ftable[i].name));

  for (;;) {
    if ((ms = minibuf_read_completion(as, astr_new(""), cp, &commands_history)) == NULL) {
      CMDCALL(cancel);
      break;
    } else if (astr_len(ms) == 0) {
      minibuf_error(astr_new("No command name given"));
      ms = NULL;
      break;
    } else {
      astr as = astr_dup(ms);
      /* Complete partial words if possible */
      if (completion_try(cp, as) == COMPLETION_MATCHED)
        ms = astr_dup(cp->match);
      for (list p = list_first(cp->completions); p != cp->completions;
           p = list_next(p))
        if (!astr_cmp(ms, p->item)) {
          ms = astr_dup(p->item);
          break;
        }
      if (get_command(ms) || get_macro(ms)) {
        add_history_element(&commands_history, ms);
        minibuf_clear();        /* Remove any error message */
        break;
      } else {
        minibuf_error(astr_afmt("Undefined command `%s'", astr_cstr(ms)));
        waitkey(WAITKEY_DEFAULT);
      }
    }
  }

  return ms;
}

DEF(unbind_key,
"\
Unbind a key.\n\
Read key chord, and unbind it.\
")
{
  size_t key = KBD_NOKEY;

  if (list_length(l) > 0)
    key = strtochord(list_behead(l));
  else {
    minibuf_write(astr_new("Unbind key: "));
    key = getkey();
  }

  unbind_key(key);
}
END_DEF

DEF(bind_key,
"\
Bind a command to a key chord.\n\
Read key chord and command name, and bind the command to the key\n\
chord.\
")
{
  size_t key = KBD_NOKEY;
  astr name = NULL;

  ok = FALSE;

  if (list_length(l) > 1) {
    key = strtochord(list_behead(l));
    name = list_behead(l);
  } else {
    astr as;

    minibuf_write(astr_new("Bind key: "));
    key = getkey();

    as = chordtostr(key);
    name = minibuf_read_command_name(astr_afmt("Bind key %s to command: ", astr_cstr(as)));
  }

  if (name) {
    Command cmd;

    if ((cmd = get_command(name))) {
      if (key != KBD_NOKEY) {
        bind_key(key, cmd);
        ok = TRUE;
      } else
        minibuf_error(astr_new("Invalid key"));
    } else
      minibuf_error(astr_afmt("No such command `%s'", astr_cstr(name)));
  }
}
END_DEF

astr command_to_binding(Command f)
{
  size_t i, n = 0;
  astr as = astr_new("");

  for (i = 0; i < vec_items(bindings); i++)
    if (vec_item(bindings, i, Binding).cmd == f) {
      size_t key = vec_item(bindings, i, Binding).key;
      astr binding = chordtostr(key);
      if (n++ != 0)
        astr_cat(as, astr_new(", "));
      astr_cat(as, binding);
    }

  return as;
}

DEF(where_is,
"\
Show key sequences that invoke the command COMMAND.\n\
FIXME: Make it work non-interactively.\
")
{
  astr name;
  Command f;

  name = minibuf_read_command_name(astr_new("Where is command: "));

  if ((f = get_command(name)) == NULL)
    ok = FALSE;
  else {
    astr bindings = command_to_binding(f);

    if (astr_len(bindings) > 0)
      minibuf_write(astr_afmt("%s is on %s", astr_cstr(name), astr_cstr(bindings)));
    else
      minibuf_write(astr_afmt("%s is not on any key", astr_cstr(name)));
  }
}
END_DEF

astr binding_to_command(size_t key)
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
    return get_command_name(p->cmd);
}
