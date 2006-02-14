/* Key chord functions
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "extern.h"

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
static size_t strtokey(const char *buf, size_t *len)
{
  size_t i, buflen = strlen(buf);

  for (i = 0; i < sizeof(keyname) / sizeof(keyname[0]); i++) {
    size_t keylen = strlen(keyname[i]);
    if (strncmp(buf, keyname[i], min(keylen, buflen)) == 0) {
      *len = keylen;
      return keycode[i];
    }
  }

  *len = 1;
  return (size_t)*buf;
}

/*
 * Convert a key chord string to its key code
 */
size_t strtochord(astr chord)
{
  size_t key = 0, len = 0, k;

  do {
    size_t l;
    k = strtokey(astr_char(chord, (ptrdiff_t)len), &l);
    key |= k;
    len += l;
  } while (k == KBD_CTRL || k == KBD_META);

  if (len != astr_len(chord))
    key = KBD_NOKEY;

  return key;
}
