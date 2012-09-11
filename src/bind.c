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


void init_bindings(void)
{
  /* Bind all printing keys to self_insert_command */
  for (size_t i = 0; i <= 0xff; i++) {
    if (isprint(i)) {
      (void)CLUE_DO(L, rblist_to_string(rblist_fmt("bind_key(%d, \"edit_insert_character\")", i)));
    }
  }

  require(PKGDATADIR "/cua_bindings");
}

void process_key(size_t key)
{
  if (key == KBD_NOKEY) {
    return;
  }

  bool ok = true;
  (void)CLUE_DO(L, rblist_to_string(rblist_fmt("s = binding_to_command(%d)", key)));
  const char *s;
  CLUE_GET(L, s, string, s);
  if (s) {
    (void)CLUE_DO(L, rblist_to_string(rblist_fmt("_ok = %s()", s)));
    CLUE_GET(L, _ok, boolean, ok);
  } else {
    term_beep();
  }

  /* Only add keystrokes if we're already in macro defining mode
     before the command call, to cope with macro_record */
  if (ok && lastflag & FLAG_DEFINING_MACRO && thisflag & FLAG_DEFINING_MACRO) {
    add_cmd_to_macro();
  }
}
