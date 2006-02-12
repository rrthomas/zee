/* Disk file handling
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

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"

/*
 * Get HOME directory.
 * If none, return blank string.
 */
astr get_home_dir(void)
{
  char *s = getenv("HOME");
  astr as = astr_new();

  if (s != NULL && strlen(s) < PATH_MAX)
    as = astr_cpy_cstr(as, s);
  return as;
}

/*
 * Find EOL string of a file's contents.
 */
static astr find_eolstr(astr as)
{
  char c = '\0';
  size_t i;
  astr eolstr = astr_new();

  assert(as);

  astr_cat_char(eolstr, '\n');

  for (i = 0; i < astr_len(as); i++) {
    c = *astr_char(as, (ptrdiff_t)i);
    if (c == '\n' || c == '\r')
      break;
  }

  if (c == '\n' || c == '\r') {
    astr_ncpy(eolstr, &c, 1);
    if (i < astr_len(as) - 1) {
      char c2 = *astr_char(as, (ptrdiff_t)(i + 1));
      if ((c2 == '\n' || c2 == '\r') && c != c2)
        astr_cat_char(eolstr, c2);
    }
  }

  return eolstr;
}

/*
 * Read the file contents into a string.
 * Return quietly if the file doesn't exist.
 */
astr file_read(astr *as, const char *filename)
{
  int ok = TRUE;
  FILE *fp;

  if ((fp = fopen(filename, "r")) == NULL)
    ok = FALSE;
  else {
    *as = astr_fread(fp);
    ok = fclose(fp) == 0;
  }

  if (ok == FALSE) {
    /* FIXME: Check terminal is initialised */
    if (errno != ENOENT)
      minibuf_write("%s: %s", filename, strerror(errno));
    return NULL;
  } else
    return find_eolstr(*as);
}

/*
 * Read the file contents into the buffer.
 * Return quietly if the file doesn't exist.
 */
void file_open(const char *filename)
{
  astr as = NULL, eolstr;

  buffer_new();
  buf.filename = zstrdup(filename);

  eolstr = file_read(&as, filename);

  if (eolstr == NULL) {
    if (errno != ENOENT)
      buf.flags |= BFLAG_READONLY;
  } else {
    strcpy(buf.eol, astr_cstr(eolstr));

    /* Add lines to buffer */
    buf.lines = string_to_lines(as, astr_cstr(eolstr), &buf.num_lines);
    buf.pt.p = list_first(buf.lines);
  }

  thisflag |= FLAG_NEED_RESYNC;
}

/*
 * Write the contents of buffer bp to file filename.
 * The filename is passed separately so a name other than buf.filename
 * can be used, e.g. in an emergency by die.
 */
static int buffer_write(Buffer *bp, const char *filename)
{
  size_t eol_len;
  FILE *fp;
  Line *lp;

  assert(bp);

  if ((fp = fopen(filename, "w")) == NULL)
    return FALSE;

  eol_len = strlen(bp->eol);

  /* Save all the lines. */
  for (lp = list_next(bp->lines); lp != bp->lines; lp = list_next(lp)) {
    if (fwrite(astr_cstr(lp->item), sizeof(char), astr_len(lp->item), fp) < astr_len(lp->item) ||
        (list_next(lp) != bp->lines && fwrite(bp->eol, sizeof(char), eol_len, fp) < eol_len)) {
      fclose(fp);
      return FALSE;
    }
  }

  return fclose(fp) == 0;
}

DEFUN("file-save", file_save)
/*+
Save buffer in visited file.
+*/
{
  if (buffer_write(&buf, buf.filename) == FALSE) {
    minibuf_error("%s: %s", buf.filename, strerror(errno));
  } else {
    Undo *up;

    minibuf_write("Wrote %s", buf.filename);
    buf.flags &= ~BFLAG_MODIFIED;

    /* Set unchanged flags to FALSE except for the
       last undo action, which is set to TRUE. */
    up = buf.last_undop;
    if (up)
      up->unchanged = TRUE;
    for (up = up->next; up; up = up->next)
      up->unchanged = FALSE;
  }
}
END_DEFUN

DEFUN("file-quit", file_quit)
/*+
Offer to the buffer, then quit.
+*/
{
  if (buf.flags & BFLAG_MODIFIED) {
    int ans;

    if ((ans = minibuf_read_yesno("Unsaved changes; exit anyway? (yes or no) ", "")) == -1)
      ok = FUNCALL(cancel);
    else if (!ans)
      ok = FALSE;
  }

  if (ok)
    thisflag |= FLAG_QUIT;
}
END_DEFUN

/*
 * Function called on unexpected error or crash (SIGSEGV).
 * Attempts to save buffer if modified.
 */
void die(int exitcode)
{
  static int already_dying;

  if (already_dying)
    fprintf(stderr, "die() called recursively; aborting.\r\n");
  else if (buf.flags & BFLAG_MODIFIED) {
    astr as = astr_new();
    already_dying = TRUE;
    fprintf(stderr, "Trying to save modified buffer...\r\n");
    if (buf.filename)
      astr_cpy_cstr(as, buf.filename);
    astr_cat_cstr(as, "." PACKAGE_NAME "SAVE");
    fprintf(stderr, "Saving %s...\r\n", astr_cstr(as));
    buffer_write(&buf, astr_cstr(as));
    term_close();
  }
  exit(exitcode);
}
