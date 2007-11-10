/* Keyboard functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2004-2007 Reuben Thomas.
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

#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "term.h"
#include "extern.h"
#include "rbacc.h"


// Get keystrokes

#define MAX_KEY_BUF	16

static int key_buf[MAX_KEY_BUF];
static int *keyp = key_buf;

/*
 * Get a keystroke, waiting for up to timeout 10ths of a second if
 * mode contains GETKEY_DELAYED, and translating it into a
 * keycode unless mode contains GETKEY_UNFILTERED.
 */
size_t xgetkey(int mode, size_t timeout)
{
  size_t key;

  if (keyp > key_buf)
    return *--keyp;

  key = term_xgetkey(mode, timeout);
  if (thisflag & FLAG_DEFINING_MACRO)
    add_key_to_cmd(key);
  return key;
}

/*
 * Wait for a keystroke indefinitely, and return the
 * corresponding keycode.
 */
size_t getkey(void)
{
  return xgetkey(0, 0);
}

/*
 * Wait for timeout 10ths if a second or until a key is pressed.
 * The key is then available with [x]getkey().
 */
void waitkey(size_t timeout)
{
  ungetkey(xgetkey(GETKEY_DELAYED, timeout));
}

void ungetkey(size_t key)
{
  if (keyp < key_buf + MAX_KEY_BUF && key != KBD_NOKEY)
    *keyp++ = key;
}
