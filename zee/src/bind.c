/* Key bindings
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2007 Reuben Thomas.
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
#include <stdbool.h>

#include "main.h"
#include "extern.h"
#include "rbacc.h"


rblist binding_to_command(size_t key)
{
  (void)CLUE_DO(L, rblist_to_string(rblist_fmt("s = _bindings[%d]", key)));
  const char *s;
  CLUE_GET(L, s, string, s);
  return s ? rblist_from_string(s) : NULL;
}

static void bind_key(size_t key, rblist cmd)
{
  CLUE_SET(L, key, number, (lua_Number)key);
  CLUE_SET(L, cmd, string, cmd ? rblist_to_string(cmd) : "nil");
  (void)CLUE_DO(L, "_bindings[key] = cmd");
}

void init_bindings(void)
{
  require(PKGDATADIR "/cua_bindings");
}

void process_key(size_t key)
{
  if (key == KBD_NOKEY) {
    return;
  }

  bool ok = true;
  rblist rbl = binding_to_command(key);
  if (rbl) {
    (void)CLUE_DO(L, rblist_to_string(rblist_fmt("_ok = %r()", rbl)));
    CLUE_GET(L, _ok, boolean, ok);
  } else {
    if (key == KBD_RET) {
      key = '\n';
    } else if (key == KBD_TAB) {
      key = '\t';
    }

    /* FIXME: Make edit_insert_character bindable like
       self_insert_command in Zile, and bind it; printable characters
       should be bound to it by default. */
    if (key <= 0xff) {
      CMDCALL_UINT(edit_insert_character, (int)key);
    } else {
      term_beep();
    }
  }

  /* Only add keystrokes if we're already in macro defining mode
     before the command call, to cope with macro_record */
  if (ok && lastflag & FLAG_DEFINING_MACRO && thisflag & FLAG_DEFINING_MACRO) {
    add_cmd_to_macro();
  }
}

DEF(key_bind,
"\
Bind a command to a key chord.\n\
Read key chord and command name, and bind the command to the key\n\
chord.\
")
{
  size_t key = KBD_NOKEY;
  rblist name = NULL;

  if (lua_gettop(L) > 1) {
    (void)CLUE_DO(L, rblist_to_string(rblist_fmt("_key = strtochord(\"%s\")", lua_tostring(L, -2))));
    CLUE_GET(L, _key, number, key);
    name = rblist_from_string(lua_tostring(L, -1));
    lua_pop(L, 2);
  } else {
    minibuf_write(rblist_from_string("Bind key: "));
    key = getkey();
    (void)CLUE_DO(L, rblist_to_string(rblist_fmt("_key = chordtostr(%d)", key)));
    const char *key_name;
    CLUE_GET(L, _key, string, key_name);
    name = minibuf_read_name(rblist_fmt("Bind key %s to command: ", key_name));
  }

  if (name) {
    if (key != KBD_NOKEY) {
      bind_key(key, name);
    } else {
      minibuf_error(rblist_from_string("Invalid key"));
      ok = false;
    }
  }
}
END_DEF

DEF(key_unbind,
"\
Unbind a key.\n\
Read key chord, and unbind it.\
")
{
  size_t key = KBD_NOKEY;

  if (lua_gettop(L) > 0) {
    (void)CLUE_DO(L, rblist_to_string(rblist_fmt("_key = strtochord(\"%s\")", lua_tostring(L, -1))));
    CLUE_GET(L, _key, number, key);
    lua_pop(L, 1);
  } else {
    minibuf_write(rblist_from_string("Unbind key: "));
    key = getkey();
  }

  if (key != KBD_NOKEY) {
    bind_key(key, NULL);
  } else {
    minibuf_error(rblist_from_string("Invalid key"));
    ok = false;
  }
}
END_DEF
