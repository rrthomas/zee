/* Program invocation, startup and shutdown
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

#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <signal.h>
#include <gc/gc.h>
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
/* The buffer. */
Buffer buf;

/* The global editor flags. */
int thisflag = 0, lastflag = 0;
/* The universal argument repeat count. */
int uniarg = 1;

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
      uniarg = 1;
  }
}

/* FIXME: Add keystroke dynamically to message, and if none exists,
   write "Press Alt-x, type `file_quit' and press RETURN". */
static char about_minibuf_str[] = "Welcome to " VERSION_STRING "!  To exit type Ctrl-q";

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
#define X(longname, doc, shortname, opt) \
    { longname, opt, NULL, shortname },
#include "tbl_opts.h"
#undef X
    { 0, 0, 0, 0 }
};

int main(int argc, char **argv)
{
  int c, bflag = FALSE, qflag = FALSE, hflag = FALSE;
  size_t line = 1;

  GC_INIT();

  init_variables();
  init_kill_ring();
  init_bindings();

#include "optstring.h"
  while ((c = getopt_long_only(argc, argv, OPTSTRING, longopts, NULL)) != -1)
    switch (c) {
    case 'b':
      bflag = TRUE;
      break;
    case 'e':
      cmd_parse_init(astr_new(optarg));
      cmd_eval();
      cmd_parse_end();
      break;
    case 'l':
      cmd_eval_file(astr_new(optarg));
      break;
    case 'n':
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
            "Run " TEXT_NAME ", the lightweight editor.\n"
            "\n");
#define X(longname, doc, shortname, opt) \
    fprintf(stderr, \
            "--" longname ", -%c" doc, shortname);
#include "tbl_opts.h"
#undef X
    fprintf(stderr,
            "FILE                   edit FILE\n"
            "+LINE                  set line at which to visit next FILE\n"
            );
  }

  signal_init();

  setlocale(LC_ALL, "");

  if (!bflag) {
    /* Initialise window */
    win.fheight = 1;           /* fheight must be > eheight */

    /* Create a single default binding so M-x commands can still be
       issued if the default bindings file can't be loaded. */
    bind_key(strtochord(astr_new("M-x")), F_execute_command);
  }

  /* Open file given on command line. */
  while (*argv) {
    if (**argv == '+')
      line = strtoul(*argv++ + 1, NULL, 10);
    else if (*argv)
      file_open(astr_new(*argv++));
  }

  /* FIXME: here and below, need better condition than buf.lines */
  if (!bflag && buf.lines) {
    term_init();
    resize_window(); /* Can't run until there is a buffer */
    goto_line(line - 1);
    resync_display();

    /* Write help message, but allow it to be overwritten by errors
       from loading init files */
    minibuf_write(astr_new(about_minibuf_str));

    /* Load default bindings file. */
    cmd_eval_file(astr_new(PATH_DATA "/key_bindings.el"));
  }

  /* Load user init file */
  if (!qflag) {
    astr as = get_home_dir();
    if (astr_len(as) > 0) {
      astr_cat(as, astr_new("/." PACKAGE_NAME));
      cmd_eval_file(as);
    }
  }

  if (buf.lines) {
    /* Display help or error message. */
    term_display();

    /* Run the main loop. */
    loop();

    term_tidy();
    term_close();
  }

  return 0;
}

#ifdef ALLEGRO
END_OF_MAIN()
#endif
