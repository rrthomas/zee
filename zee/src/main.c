/* Program invocation, startup and shutdown
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
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

#define DEBUG 1

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#ifdef HAVE_GETOPT_LONG_ONLY
#include <getopt.h>
#else
#include "getopt.h"
#endif
#ifdef ALLEGRO
#include <allegro.h>
#endif

#include "main.h"
#include "paths.h"
#include "extern.h"


#define VERSION_STRING TEXT_NAME " " VERSION

#define COPYRIGHT_STRING \
  "Copyright (c) 2003-2006 Reuben Thomas <rrt@sc3d.org>\n"\
  "Copyright (c) 2005-2006 Alistair Turbull <apt1002@mupsych.org>\n"\
  "Copyright (c) 1997-2004 Sandro Sigala <sandro@sigala.it>\n"\
  "Copyright (c) 2003-2004 David A. Capello <dacap@users.sourceforge.net>"

/* The window. */
Window win;
/* The current buffer; the first buffer in list. */
Buffer *cur_bp = NULL, *head_bp = NULL;

/* The global editor flags. */
int thisflag = 0, lastflag = 0;
/* The universal argument repeat count. */
int last_uniarg = 1;

static void loop(void)
{
  for (;
       !(thisflag & FLAG_QUIT);
       lastflag = thisflag) {
    size_t key;

    if (lastflag & FLAG_NEED_RESYNC)
      resync_display();
    term_display();
    term_refresh();

    thisflag = 0;
    if (lastflag & FLAG_DEFINING_MACRO)
      thisflag |= FLAG_DEFINING_MACRO;

    key = getkey();
    popup_clear();
    minibuf_clear();
    process_key(key);

    if (!(thisflag & FLAG_SET_UNIARG))
      last_uniarg = 1;
  }
}

static char about_minibuf_str[] = "Welcome to " TEXT_NAME "!  To exit press Ctrl-q";

static void segv_sig_handler(int signo)
{
  (void)signo;
  fprintf(stderr, TEXT_NAME " crashed.  Please send a bug report to <" PACKAGE_BUGREPORT ">\r\n");
  die(2);
}

static void other_sig_handler(int signo)
{
  (void)signo;
  fprintf(stderr, TEXT_NAME " terminated with signal %d\r\n", signo);
  die(2);
}

static void signal_init(void)
{
  /* Set up signal handling */
  signal(SIGSEGV, segv_sig_handler);
  signal(SIGHUP, other_sig_handler);
  signal(SIGINT, other_sig_handler);
  signal(SIGQUIT, other_sig_handler);
  signal(SIGTERM, other_sig_handler);
  signal(SIGPIPE, SIG_IGN);
}

/* Options table */
struct option longopts[] = {
    { "batch",        0, NULL, 'b' },
    { "eval",         1, NULL, 'e' },
    { "help",         0, NULL, 'h' },
    { "load",         1, NULL, 'l' },
    { "no-init-file", 0, NULL, 'q' },
    { "version",      0, NULL, 'v' },
    { 0, 0, 0, 0 }
};

static void open_file_at(char *path, size_t lineno)
{
  astr cwd = get_current_dir(FALSE), dir = astr_new(), fname = astr_new();

  /* Check path */
  if (!expand_path(path, astr_cstr(cwd), dir, fname)) {
    fprintf(stderr, PACKAGE_NAME ": %s: invalid filename or path\n", path);
    die(1);
  }
  astr_delete(cwd);

  /* Open file */
  astr_cat_delete(dir, fname);
  file_visit(astr_cstr(dir));
  astr_delete(dir);

  /* Update display */
  if (lineno > 0)
    ngotodown(lineno);
  resync_display();
}

int main(int argc, char **argv)
{
  int c, bflag = FALSE, qflag = FALSE, eflag = FALSE, hflag = FALSE;
  astr as = astr_new();

  while ((c = getopt_long_only(argc, argv, "l:q", longopts, NULL)) != -1)
    switch (c) {
    case 'b':
      bflag = TRUE;
      qflag = TRUE;
      break;
    case 'e':
      {
        astr bs = astr_cpy_cstr(astr_new(), optarg);
        cmd_parse_init(bs);
        cmd_eval();
        cmd_parse_end();
        eflag = TRUE;
      }
      break;
    case 'l':
      cmd_eval_file(optarg);
      eflag = TRUE; /* Loading a file counts as reading an expression. */
      break;
    case 'q':
      qflag = TRUE;
      break;
    case 'v':
      fprintf(stderr,
              VERSION_STRING "\n"
              COPYRIGHT_STRING "\n"
              TEXT_NAME " comes with ABSOLUTELY NO WARRANTY.\n"
              "You may redistribute copies of " TEXT_NAME "\n"
              "under the terms of the GNU General Public License.\n"
              "For more information about these matters, see the file named COPYING.\n"
              );
      return 0;
    case 'h':
      hflag = TRUE;
      break;
    }
  argc -= optind;
  argv += optind;

  if (hflag || (argc == 0 && optind == 1)) {
    fprintf(stderr,
            "Usage: " PACKAGE_NAME " [OPTION-OR-FILENAME]...\n"
            "\n"
            "Run " TEXT_NAME ", the lightweight editor.\n"
            "\n"
            "Initialization options:\n"
            "\n"
            "--batch                do not do interactive display; implies -q\n"
            "--help                 display this help message and exit\n"
            "--no-init-file, -q     do not load ~/." PACKAGE_NAME "\n"
            "--version              display version information and exit\n"
            "\n"
            "Action options:\n"
            "\n"
            "FILE                   visit FILE using file-open\n"
            "+LINE FILE             visit FILE using file-open, then go to line LINE\n"
            "--eval EXPR            evaluate Lisp expression EXPR\n"
            "--load, -l FILE        load file of Lisp code using the load function\n"
            );
    return 0;
  }

  signal_init();

  setlocale(LC_ALL, "");

  if (astr_len(as) > 0)
    printf("%s", astr_cstr(as));

  if (!bflag) {
    if (!qflag) {
      astr as = get_home_dir();
      astr_cat_cstr(as, "/." PACKAGE_NAME);
      cmd_eval_file(astr_cstr(as));
      astr_delete(as);
    }

    term_init();
    init_kill_ring();
    init_bindings();

    /* Initialise window */
    win.fheight = 1;           /* fheight must be > eheight */

    /* Create a single default binding so M-x commands can still be
       issued if the default bindings file can't be loaded. */
    bind_key(strtochord("M-x"), F_execute_command);

    /* Open file given on command line. */
    while (*argv) {
      size_t line = 1;
      if (**argv == '+')
        line = strtoul(*argv++ + 1, NULL, 10);
      else if (*argv)
        open_file_at(*argv++, line - 1);
    }
    resize_window(); /* Can't run until there is at least one buffer */

    /* Write help message, but allow it to be overwritten by errors
       from loading key_bindings.el. */
    minibuf_write(about_minibuf_str);

    /* Load default bindings file. */
    cmd_eval_file(PATH_DATA "/key_bindings.el");

    /* Display help or error message. */
    term_display();

    /* Run the main loop. */
    loop();

    /* Tidy and close the terminal. */
    term_tidy();
    term_close();

    free_bindings();
    free_kill_ring();
  }

  /* Free all the memory allocated. */
  astr_delete(as);
  free_search_history();
  free_macros();
  free_buffers();
  free_minibuf();

  return 0;
}

#ifdef ALLEGRO
END_OF_MAIN()
#endif
