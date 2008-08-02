/* Disk file handling
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

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "rbacc.h"
#include "term.h"
#include "extern.h"


/*
 * Get HOME directory.
 * If none, or too long, return blank string.
 */
rblist get_home_dir(void)
{
  char *s = getenv("HOME");

  if (s && strlen(s) < PATH_MAX)
    return rblist_from_string(s);
  else
    return rblist_empty;
}

/*
 * Read the file contents into a string.
 * Return quietly if the file can't be read.
 */
rblist file_read(rblist filename)
{
  FILE *fp;
  if ((fp = fopen(rblist_to_string(filename), "r")) == NULL) {
    return NULL;
  } else {
    rbacc rba = rbacc_add_file(rbacc_new(), fp);
    fclose(fp);
    return rbacc_to_rblist(rba);
  }
}

/*
 * Read the file contents into a buffer.
 * Return quietly if the file can't be read.
 */
void file_open(rblist filename)
{
  buffer_new();
  buf->filename = filename;

  rblist rbl;
  if ((rbl = file_read(buf->filename)) != NULL) {
    // Add lines to buffer
    buf->lines = rbl;
  } else if (errno != ENOENT) {
    buf->flags |= BFLAG_READONLY;
  }
}

/*
 * Write the contents of buffer bp to file filename.
 * The filename is passed separately so a name other than buf->filename
 * can be used, e.g. in an emergency by die.
 * Returns true on success, false on failure.
 */
static bool buffer_write(Buffer *bp, rblist filename)
{
  FILE *fp;
  size_t len = rblist_length(buf->lines);

  assert(bp);

  if ((fp = fopen(rblist_to_string(filename), "w")) == NULL) {
    return false;
  }

  // Save all the lines.
  if (fwrite(rblist_to_string(buf->lines), sizeof(char), len, fp) < len) {
    fclose(fp);
    return false;
  }

  return fclose(fp) == 0;
}

DEF(file_save,
"\
Save buffer in visited file.\
")
{
  if (buffer_write(buf, buf->filename) == false) {
    minibuf_error(rblist_to_string(rblist_fmt("%s: %s", buf->filename, strerror(errno))));
  } else {
    minibuf_write(rblist_to_string(rblist_fmt("Wrote `%r'", buf->filename)));
    buf->flags &= ~BFLAG_MODIFIED;

    if (buf->last_undop)
      undo_reset_unmodified(buf->last_undop);
  }
}
END_DEF

DEF(file_quit,
"\
Quit, unless there are unsaved changes.\
")
{
  if (buf->flags & BFLAG_MODIFIED) {
    minibuf_error("Unsaved changes; use `file_save' or `edit_revert'");
    ok = false;
  }

  if (ok)
    thisflag |= FLAG_QUIT;
}
END_DEF

/*
 * Function called on unexpected error or crash (SIGSEGV).
 * Attempts to save buffer if modified.
 */
// FIXME: Reset the terminal?
void die(int exitcode)
{
  static bool already_dying = 0;

  if (already_dying)
    fprintf(stderr, "die() called recursively; aborting.\r\n");
  else if (buf && buf->flags & BFLAG_MODIFIED) {
    rblist rbl = rblist_empty;
    already_dying = true;
    fprintf(stderr, "Trying to save modified buffer...\r\n");
    if (buf->filename)
      rbl = buf->filename;
    rbl = rblist_concat(rbl, rblist_from_string("." PACKAGE "SAVE"));
    fprintf(stderr, "Saving %s...\r\n", rblist_to_string(rbl));
    buffer_write(buf, rbl);
    term_close();
  }
  exit(exitcode);
}
