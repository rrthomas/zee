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

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "extern.h"


/*
 * Get HOME directory.
 * If none, or invalid, return blank string.
 */
astr get_home_dir(void)
{
  char *s = getenv("HOME");
  astr as;

  if (s && strlen(s) < PATH_MAX)
    as = astr_new(s);
  else
    as = astr_new("");

  return as;
}

/*
 * Read the file contents into a string.
 * Return quietly if the file can't be read.
 */
astr file_read(astr filename)
{
  FILE *fp;
  if ((fp = fopen(astr_cstr(filename), "r")) == NULL)
    return NULL;
  else {
    astr as = astr_fread(fp);
    fclose(fp);
    return as;
  }
}

/*
 * Read the file contents into a buffer.
 * Return quietly if the file can't be read.
 */
void file_open(astr filename)
{
  astr as;

  buffer_new();
  buf->filename = astr_dup(filename);

  if ((as = file_read(filename)) == NULL) {
    if (errno != ENOENT)
      buf->flags |= BFLAG_READONLY;
  } else {
    /* Add lines to buffer */
    buf->lines = string_to_lines(as, &buf->num_lines);
    buf->pt.p = list_first(buf->lines);
  }

  thisflag |= FLAG_NEED_RESYNC;
}

/*
 * Write the contents of buffer bp to file filename.
 * The filename is passed separately so a name other than buf->filename
 * can be used, e.g. in an emergency by die.
 */
static int buffer_write(Buffer *bp, astr filename)
{
  FILE *fp;
  Line *lp;

  assert(bp);

  if ((fp = fopen(astr_cstr(filename), "w")) == NULL)
    return FALSE;

  /* Save all the lines. */
  for (lp = list_next(bp->lines); lp != bp->lines; lp = list_next(lp)) {
    if (fwrite(astr_cstr(lp->item), sizeof(char), astr_len(lp->item), fp) < astr_len(lp->item) ||
        (list_next(lp) != bp->lines && putc('\n', fp) == EOF)) {
      fclose(fp);
      return FALSE;
    }
  }

  return fclose(fp) == 0;
}

DEF(file_save,
"\
Save buffer in visited file.\
")
{
  if (buffer_write(buf, buf->filename) == FALSE) {
    minibuf_error(astr_afmt("%s: %s", buf->filename, strerror(errno)));
  } else {
    minibuf_write(astr_afmt("Wrote `%s'", astr_cstr(buf->filename)));
    buf->flags &= ~BFLAG_MODIFIED;

    if (buf->last_undop)
      undo_reset_unmodified(buf->last_undop);
  }
}
END_DEF

DEF(file_quit,
"\
Offer to save the buffer if there are unsaved changes, then quit.\
")
{
  if (buf->flags & BFLAG_MODIFIED) {
    int ans;

    if ((ans = minibuf_read_yesno(astr_new("Unsaved changes; exit anyway? (yes or no) "))) == -1)
      ok = CMDCALL(edit_select_off);
    else if (!ans)
      ok = FALSE;
  }

  if (ok)
    thisflag |= FLAG_QUIT;
}
END_DEF

/*
 * Function called on unexpected error or crash (SIGSEGV).
 * Attempts to save buffer if modified.
 */
void die(int exitcode)
{
  static int already_dying = 0;

  if (already_dying)
    fprintf(stderr, "die() called recursively; aborting.\r\n");
  else if (buf && buf->flags & BFLAG_MODIFIED) {
    astr as = astr_new("");
    already_dying = TRUE;
    fprintf(stderr, "Trying to save modified buffer...\r\n");
    if (buf->filename)
      as = astr_dup(buf->filename);
    astr_cat(as, astr_new("." PACKAGE_NAME "SAVE"));
    fprintf(stderr, "Saving %s...\r\n", astr_cstr(as));
    buffer_write(buf, as);
    term_close();
  }
  exit(exitcode);
}
