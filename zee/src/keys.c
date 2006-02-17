/* Keyboard functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2004-2006 Reuben Thomas.
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

#include <string.h>
#include <ctype.h>

#include "main.h"
#include "extern.h"


/* Get keystrokes */

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


/* Key code and name arrays */

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
astr chordtostr(size_t key)
{
  int found;
  size_t i;
  astr as = astr_new("");

  if (key & KBD_CTRL)
    astr_cat_cstr(as, "C-");
  if (key & KBD_META)
    astr_cat_cstr(as, "M-");
  key &= ~(KBD_CTRL | KBD_META);

  for (found = FALSE, i = 0; i < sizeof(keycode) / sizeof(keycode[0]); i++)
    if (keycode[i] == key) {
      astr_cat_cstr(as, keyname[i]);
      found = TRUE;
      break;
    }

  if (found == FALSE) {
    if (isgraph(key))
      astr_cat_char(as, (int)(key & 0xff));
    else
      astr_cat(as, astr_afmt("<%x>", key));
  }

  return as;
}

/*
 * Convert a key string to its key code
 */
static size_t strtokey(astr buf, size_t *len)
{
  size_t i;

  for (i = 0; i < sizeof(keyname) / sizeof(keyname[0]); i++) {
    size_t keylen = strlen(keyname[i]);
    if (strncmp(astr_cstr(buf), keyname[i], min(keylen, astr_len(buf))) == 0) {
      *len = keylen;
      return keycode[i];
    }
  }

  *len = 1;
  return (size_t)*astr_char(buf, 0);
}

/*
 * Convert a key chord string to its key code
 */
size_t strtochord(astr chord)
{
  size_t key = 0, len = 0, k;

  do {
    size_t l;
    k = strtokey(astr_substr(chord, (ptrdiff_t)len, astr_len(chord) - len), &l);
    key |= k;
    len += l;
  } while (k == KBD_CTRL || k == KBD_META);

  if (len != astr_len(chord))
    key = KBD_NOKEY;

  return key;
}
