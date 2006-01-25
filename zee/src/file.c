/* Disk file handling
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2005 Reuben Thomas.
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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <pwd.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <utime.h>

#include "main.h"
#include "extern.h"

static int exist_file(const char *filename)
{
  struct stat st;

  if (stat(filename, &st) == -1)
    if (errno == ENOENT)
      return FALSE;

  return TRUE;
}

/*
 * Safe getcwd
 */
astr agetcwd(void)
{
  size_t len = PATH_MAX;
  char *buf = (char *)zmalloc(len);
  char *res;
  astr as;
  while ((res = getcwd(buf, len)) == NULL && errno == ERANGE) {
    len *= 2;
    buf = zrealloc(buf, len / 2, len);
  }
  /* If there was an error, return the empty string */
  if (res == NULL)
    *buf = '\0';
  as = astr_cpy_cstr(astr_new(), buf);
  free(buf);
  return as;
}

/*
 * This functions does some corrections and expansions to
 * the passed path:
 * - Splits the path into the directory and the filename;
 * - Expands the `~/' and `~name/' expressions;
 * - replaces the `//' with `/' (restarting from the root directory);
 * - removes the `..' and `.' entries.
 */
int expand_path(const char *path, const char *cwdir, astr dir, astr fname)
{
  struct passwd *pw;
  const char *sp = path;

  if (*sp != '/') {
    astr_cat_cstr(dir, cwdir);
    if (astr_len(dir) == 0 || *astr_char(dir, -1) != '/')
      astr_cat_cstr(dir, "/");
  }

  while (*sp != '\0') {
    if (*sp == '/') {
      if (*++sp == '/') {
        /* Got `//'.  Restart from this point. */
        while (*sp == '/')
          sp++;
        astr_truncate(dir, 0);
      }
      astr_cat_char(dir, '/');
    } else if (*sp == '~') {
      if (*(sp + 1) == '/') {
        /* Got `~/'. Restart from this point and insert the user's
           home directory. */
        astr_truncate(dir, 0);
        if ((pw = getpwuid(getuid())) == NULL)
          return FALSE;
        if (strcmp(pw->pw_dir, "/") != 0)
          astr_cat_cstr(dir, pw->pw_dir);
        ++sp;
      } else {
        /* Got `~something'.  Restart from this point and insert that
           user's home directory. */
        astr as = astr_new();
        astr_truncate(dir, 0);
        ++sp;
        while (*sp != '\0' && *sp != '/')
          astr_cat_char(as, *sp++);
        pw = getpwnam(astr_cstr(as));
        astr_delete(as);
        if (pw == NULL)
          return FALSE;
        astr_cat_cstr(dir, pw->pw_dir);
      }
    } else if (*sp == '.') {
      if (*(sp + 1) == '/' || *(sp + 1) == '\0') {
        ++sp;
        if (*sp == '/' && *(sp + 1) != '/')
          ++sp;
      } else if (*(sp + 1) == '.' &&
                 (*(sp + 2) == '/' || *(sp + 2) == '\0')) {
        if (astr_len(dir) >= 1 && *astr_char(dir, -1) == '/')
          astr_truncate(dir, -1);
        while (*astr_char(dir, -1) != '/' &&
               astr_len(dir) >= 1)
          astr_truncate(dir, -1);
        sp += 2;
        if (*sp == '/' && *(sp + 1) != '/')
          ++sp;
      } else
        goto gotfname;
    } else {
      const char *p;
    gotfname:
      p = sp;
      while (*p != '\0' && *p != '/')
        p++;
      if (*p == '\0') {
        /* Final filename. */
        astr_cat_cstr(fname, sp);
        break;
      } else {
        /* Non-final directory. */
        while (*sp != '/')
          astr_cat_char(dir, *sp++);
      }
    }
  }

  if (astr_len(dir) == 0)
    astr_cat_cstr(dir, "/");

  return TRUE;
}

/*
 * Return a `~/foo' like path if the user is under his home directory,
 * and restart from / if // found, else the unmodified path.
 */
astr compact_path(astr path)
{
  astr buf = astr_new();
  struct passwd *pw;
  size_t i;

  if ((pw = getpwuid(getuid())) == NULL) {
    /* User not found in password file. */
    astr_cpy(buf, path);
    return buf;
  }

  /* Replace `/userhome/' (if existent) with `~/'. */
  i = strlen(pw->pw_dir);
  if (!strncmp(pw->pw_dir, astr_cstr(path), i)) {
    astr_cpy_cstr(buf, "~/");
    if (!strcmp(pw->pw_dir, "/"))
      astr_cat_cstr(buf, astr_char(path, 1));
    else
      astr_cat_cstr(buf, astr_char(path, (ptrdiff_t)(i + 1)));
  } else
    astr_cpy(buf, path);

  return buf;
}

/*
 * Return the current directory. The current directory is defined to be the one
 * containing the current file, if any, otherwise it is the cwd.
 */
astr get_current_dir(int interactive)
{
  astr buf;

  if (interactive && cur_bp && cur_bp->filename != NULL) {
    /* If the current buffer has a filename, get the current directory
       name from it. */
    char *sep;

    buf = astr_cpy_cstr(astr_new(), cur_bp->filename);
    if ((sep = strrchr(astr_cstr(buf), '/')) != NULL)
      astr_truncate(buf, sep - astr_cstr(buf));
  } else
    /* Get the current directory name from the system. */
    buf = agetcwd();

  if (*astr_char(buf, -1) != '/')
    astr_cat_cstr(buf, "/");

  return buf;
}

/*
 * Get HOME directory.
 */
astr get_home_dir(void)
{
  char *s = getenv("HOME");
  astr as;
  if (s != NULL && strlen(s) < PATH_MAX)
    as = astr_cat_cstr(astr_new(), s);
  else
    as = agetcwd();
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
    if (cur_bp && errno != ENOENT)
      minibuf_write("%s: %s", filename, strerror(errno));
    return NULL;
  } else
    return find_eolstr(*as);
}

/*
 * Read the file contents into a buffer.
 * Return quietly if the file doesn't exist.
 */
void file_open(Buffer *bp, const char *filename)
{
  astr as = NULL, eolstr;

  assert(bp);

  eolstr = file_read(&as, filename);

  if (eolstr == NULL) {
    if (errno != ENOENT)
      bp->flags |= BFLAG_READONLY;
  } else {
    /* Set buffer's EOL string */
    strcpy(bp->eol, astr_cstr(eolstr));

    /* Add lines to buffer */
    line_delete(bp->lines);
    bp->lines = string_to_lines(as, astr_cstr(eolstr), &bp->num_lines);
    bp->pt.p = list_first(bp->lines);

    astr_delete(eolstr);
    astr_delete(as);
  }
}

static int is_regular_file(const char *filename)
{
  struct stat st;

  if (stat(filename, &st) == -1) {
    if (errno == ENOENT)
      return TRUE;
    return FALSE;
  }
  if (S_ISREG(st.st_mode))
    return TRUE;

  return FALSE;
}

/*
 * Load the given file into a new buffer.
 */
int file_visit(const char *filename)
{
  astr as;

  as = make_buffer_name(filename);
  if (astr_len(as) == 0) {
    astr_delete(as);
    return FALSE;
  }

  if (!is_regular_file(filename)) {
    minibuf_error("%s is not a regular file", filename);
    waitkey(WAITKEY_DEFAULT);
    return FALSE;
  }

  cur_bp = create_buffer(astr_cstr(as));
  astr_delete(as);
  cur_bp->filename = zstrdup(filename);

  assert(cur_bp);
  file_open(cur_bp, filename);

  thisflag |= FLAG_NEED_RESYNC;

  return TRUE;
}

static int file_insert(const char *filename)
{
  astr as, eolstr;

  assert(cur_bp);

  if (!exist_file(filename)) {
    minibuf_error("Unable to read file `%s'", filename);
    return FALSE;
  }

  if ((eolstr = file_read(&as, filename)) != NULL) {
    insert_nstring(as, astr_cstr(eolstr), FALSE);
    astr_delete(as);
    astr_delete(eolstr);
  }

  return eolstr != NULL;
}

DEFUN_INT("file-insert", file_insert)
/*+
Insert contents of the user specified file into buffer after point.
Set mark after the inserted text.
+*/
{
  astr ms, buf;

  if ((ok = !warn_if_readonly_buffer())) {
    buf = get_current_dir(TRUE);
    if ((ms = minibuf_read_dir("Insert file: ", astr_cstr(buf))) == NULL)
      ok = cancel();
    astr_delete(buf);

    if (ok) {
      if (astr_len(ms) == 0 || !file_insert(astr_cstr(ms)))
        ok =  FALSE;
      else
        set_mark_command();

      astr_delete(ms);
    }
  }
}
END_DEFUN

static int raw_write_to_disk(Buffer *bp, const char *filename)
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

/*
 * Write the buffer contents to a file.
 */
static void write_file(Buffer *bp, astr ms)
{
  if (raw_write_to_disk(bp, astr_cstr(ms)) == FALSE) {
    minibuf_error("%s: %s", astr_cstr(ms), strerror(errno));
  } else {
    Undo *up;

    minibuf_write("Wrote %s", astr_cstr(ms));
    bp->flags &= ~BFLAG_MODIFIED;

    /* Set unchanged flags to FALSE except for the
       last undo action, which is set to TRUE. */
    up = bp->last_undop;
    if (up)
      up->unchanged = TRUE;
    for (up = up->next; up; up = up->next)
      up->unchanged = FALSE;
  }
}

static int file_save(Buffer *bp)
{
  char *fname = bp->filename != NULL ? bp->filename : bp->name;
  astr ms;

  if (!(bp->flags & BFLAG_MODIFIED))
    minibuf_write("(No changes need to be saved)");
  else {
    if (bp->flags & BFLAG_NEEDNAME) {
      if ((ms = minibuf_read_dir("File to save in: ", fname)) == NULL)
        return cancel();
      if (astr_len(ms) == 0) {
        astr_delete(ms);
        return FALSE;
      }

      set_buffer_filename(bp, astr_cstr(ms));

      bp->flags &= ~BFLAG_NEEDNAME;
    } else {
      ms = astr_new();
      astr_cpy_cstr(ms, bp->filename);
    }

    write_file(bp, ms);

    astr_delete(ms);
  }

  return TRUE;
}

DEFUN_INT("file-save", file_save)
/*+
Save current buffer in visited file if modified.
+*/
{
  assert(cur_bp);
  ok = file_save(cur_bp);
}
END_DEFUN

DEFUN_INT("file-save-as", file_save_as)
/*+
Write current buffer into the user specified file.
Makes buffer visit that file, and marks it not modified.
+*/
{
  char *fname;
  astr ms;

  assert(cur_bp);

  fname = cur_bp->filename != NULL ? cur_bp->filename : cur_bp->name;

  if ((ms = minibuf_read_dir("Write file: ", fname)) == NULL)
    ok = cancel();
  else if (astr_len(ms) == 0)
    ok = FALSE;

  if (ok) {
    set_buffer_filename(cur_bp, astr_cstr(ms));

    cur_bp->flags &= ~BFLAG_NEEDNAME;

    write_file(cur_bp, ms);
  }

  if (ms)
    astr_delete(ms);
}
END_DEFUN

DEFUN_INT("file-quit", file_quit)
/*+
Offer to save each buffer, then kill this process.
+*/
{
  int ans, i = 0;

  if (cur_bp->flags & BFLAG_MODIFIED && !(cur_bp->flags & BFLAG_NEEDNAME))
    ++i;

  if (i > 0) {
    if ((ans = minibuf_read_yesno("Modified buffers exist; exit anyway? (yes or no) ", "")) == -1)
      ok = cancel();
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
  else if (cur_bp && cur_bp->flags & BFLAG_MODIFIED) {
    astr buf = astr_new();
    already_dying = TRUE;
    fprintf(stderr, "Trying to save modified buffer...\r\n");
    if (cur_bp->filename != NULL)
      astr_cpy_cstr(buf, cur_bp->filename);
    else
      astr_cpy_cstr(buf, cur_bp->name);
    astr_cat_cstr(buf, "." PACKAGE_NAME "SAVE");
    fprintf(stderr, "Saving %s...\r\n", astr_cstr(buf));
    raw_write_to_disk(cur_bp, astr_cstr(buf));
    astr_delete(buf);
    term_close();
  }
  exit(exitcode);
}

DEFUN_INT("file-change-directory", file_change_directory)
/*+
Make DIR the current directory.
+*/
{
  astr ms, buf;

  buf = get_current_dir(TRUE);
  if ((ms = minibuf_read_dir("Change default directory: ",
                             astr_cstr(buf))) == NULL)
    ok = cancel();
  astr_delete(buf);

  if (ok)
    if (astr_len(ms) > 0) {
      struct stat st;
      if (stat(astr_cstr(ms), &st) != 0 || !S_ISDIR(st.st_mode)) {
        minibuf_error("`%s' is not a directory", astr_cstr(ms));
        ok = FALSE;
      } else if (chdir(astr_cstr(ms)) == -1) {
        minibuf_write("%s: %s", astr_cstr(ms), strerror(errno));
        ok = FALSE;
      }

      astr_delete(ms);
    }
}
END_DEFUN
