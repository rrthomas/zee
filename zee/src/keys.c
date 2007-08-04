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


// Key code and name arrays

static size_t keycode[] = {
#define X(key_sym, key_name, key_code) \
	key_sym,
#include "tbl_keys.h"
#undef X
};

static const char *keyname[] = {
#define X(key_sym, key_name, key_code) \
	key_name,
#include "tbl_keys.h"
#undef X
};

/*
 * Convert a key chord into its ASCII representation
 */
rblist chordtostr(size_t key)
{
  size_t i;
  rbacc rba = rbacc_new();

  if (key & KBD_CTRL)
    rbacc_add_string(rba, "C-");
  if (key & KBD_META)
    rbacc_add_string(rba, "M-");
  key &= ~(KBD_CTRL | KBD_META);

  bool found = false;
  for (i = 0; i < sizeof(keycode) / sizeof(keycode[0]); i++)
    if (keycode[i] == key) {
      rbacc_add_string(rba, keyname[i]);
      found = true;
      break;
    }

  if (found == false) {
    if (isgraph(key))
      rbacc_add_char(rba, (int)(key & 0xff));
    else
      rbacc_add_rblist(rba, rblist_fmt("<%x>", key));
  }

  return rbacc_to_rblist(rba);
}

/*
 * Find a key name as a prefix of `rbl', returning its keycode, and
 * the number of characters consumed in `*len'.
 */
static size_t strtokey(rblist rbl, size_t *len)
{
  size_t i;

  for (i = 0; i < sizeof(keyname) / sizeof(keyname[0]); i++) {
    size_t keylen = strlen(keyname[i]);
    if (rblist_length(rbl) >= keylen &&
        strncmp(rblist_to_string(rbl), keyname[i], keylen) == 0) {
      *len = keylen;
      return keycode[i];
    }
  }

  *len = 1;
  return (size_t)rblist_get(rbl, 0);
}

/*
 * Convert a key chord string to its key code
 */
size_t strtochord(rblist chord)
{
  size_t key = 0, len = 0, k;

  do {
    size_t l;
    k = strtokey(rblist_sub(chord, len, rblist_length(chord)), &l);
    key |= k;
    len += l;
  } while (k == KBD_CTRL || k == KBD_META);

  if (len != rblist_length(chord))
    key = KBD_NOKEY;

  return key;
}
