/* Key sequences functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2004-2005 Reuben Thomas.
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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "extern.h"

/*
 * Array of key codes
 */
static size_t keycode[] = {
  KBD_CTRL,
  KBD_META,
  KBD_SPC,
  KBD_TAB,
  KBD_RET,
  KBD_PGUP,
  KBD_PGDN,
  KBD_HOME,
  KBD_END,
  KBD_DEL,
  KBD_BS,
  KBD_INS,
  KBD_LEFT,
  KBD_RIGHT,
  KBD_UP,
  KBD_DOWN,
  KBD_F1,
  KBD_F2,
  KBD_F3,
  KBD_F4,
  KBD_F5,
  KBD_F6,
  KBD_F7,
  KBD_F8,
  KBD_F9,
  KBD_F10,
  KBD_F11,
  KBD_F12,
};

/*
 * Array of key names in the same order as codes above
 */
static const char *keyname[] = {
  "C-",
  "M-",
  "SPC",
  "TAB",
  "RET",
  "PGUP",
  "PGDN",
  "HOME",
  "END",
  "DEL",
  "BS",
  "INS",
  "LEFT",
  "RIGHT",
  "UP",
  "DOWN",
  "F1",
  "F2",
  "F3",
  "F4",
  "F5",
  "F6",
  "F7",
  "F8",
  "F9",
  "F10",
  "F11",
  "F12",
};

/*
 * Convert a key chord into its ASCII representation
 */
astr chordtostr(size_t key)
{
  int found;
  size_t i;
  astr as = astr_new();

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
      astr_afmt(as, "<%x>", key);
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
size_t strtochord(const char *buf)
{
  size_t key = 0, len = 0, k;

  do {
    size_t l;
    k = strtokey(buf + len, &l);
    key |= k;
    len += l;
  } while (k == KBD_CTRL || k == KBD_META);

  return key;
}
