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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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
#include "extern.h"


#define COPYRIGHT_STRING \
  "Copyright (c) 2003-2006 Reuben Thomas <rrt@sc3d.org>\n"\
  "Copyright (c) 2005-2006 Alistair Turbull <apt1002@mupsych.org>\n"\
  "Copyright (c) 1997-2004 Sandro Sigala <sandro@sigala.it>\n"\
  "Copyright (c) 2003-2004 David A. Capello <dacap@users.sourceforge.net>"

/* The window. */
Window win;
/* The buffer. */
Buffer *buf = NULL;

/* The global editor flags. */
int thisflag = 0, lastflag = 0;

static void run(void)
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
  }
}

static void segv_sig_handler(int signo)
{
  (void)signo;
  fprintf(stderr, PACKAGE_NAME " crashed. Please send a bug report to <" PACKAGE_BUGREPORT ">\r\n");
  die(2);
}

static void other_sig_handler(int signo)
{
  (void)signo;
  fprintf(stderr, PACKAGE_NAME " terminated with signal %d\r\n", signo);
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
#define X(longname, doc, opt) \
    {longname, opt, NULL, 0},
#include "tbl_opts.h"
#undef X
    {0, 0, 0, 0}
};

int main(int argc, char **argv)
{
  int longopt;
  bool bflag = false, hflag = false, nflag = false;
  size_t line = 1;
  rblist as;

  GC_INIT();

  init_variables();
  init_kill_ring();
  init_bindings();

  while (getopt_long_only(argc, argv, "", longopts, &longopt) != -1)
    switch (longopt) {
    case 0:
      bflag = true;
      break;
    case 1:
      cmd_eval(rblist_from_string(optarg));
      break;
    case 2:
      if ((as = file_read(rblist_from_string(optarg))))
        cmd_eval(as);
      break;
    case 3:
      nflag = true;
      break;
    case 4:
      fprintf(stderr,
              PACKAGE_STRING "\n"
              COPYRIGHT_STRING "\n"
              PACKAGE_NAME " comes with ABSOLUTELY NO WARRANTY.\n"
              "You may redistribute copies of " PACKAGE_NAME "\n"
              "under the terms of the GNU General Public License.\n"
              "For more information about these matters, see the file named COPYING.\n"
              );
      return 0;
    case 5:
      hflag = true;
      break;
    }
  argc -= optind;
  argv += optind;

  if (hflag || (argc == 0 && optind == 1)) {
    fprintf(stderr,
            "Usage: " PACKAGE " [OPTION-OR-FILENAME]...\n"
            "Run " PACKAGE_NAME ", the lightweight editor.\n"
            "\n");
#define X(longname, doc, opt) \
    fprintf(stderr, "--" longname doc);
#include "tbl_opts.h"
#undef X
    fprintf(stderr,
            "FILE               edit FILE\n"
            "+LINE              set line at which to visit next FILE\n"
            );
  } else {
    signal_init();
    setlocale(LC_ALL, "");

    if (!bflag) {
      /* Initialise window */
      win.fheight = 1;           /* fheight must be > eheight */

      /* Create a single default binding so M-x commands can still be
         issued if the default bindings file can't be loaded. */
      bind_key(strtochord(rblist_from_string("M-x")), F_execute_command);
    }

    /* Open file given on command line. */
    while (*argv) {
      if (**argv == '+')
        line = strtoul(*argv++ + 1, NULL, 10);
      else if (*argv) {
        file_open(rblist_from_string(*argv++));
        break;
      }
    }

    if (!bflag) {
      term_init();
      resize_window(); /* Can't run until there is a buffer */
      if (buf) {
        goto_line(line - 1);
        resync_display();
      }

      /* Load default bindings file. */
      if ((as = file_read(rblist_from_string(PKGDATADIR "/key_bindings.el"))))
        cmd_eval(as);
    }

    /* Load user init file */
    if (!nflag) {
      if ((as = file_read(rblist_from_string(SYSCONFDIR "/" PACKAGE "rc"))))
        cmd_eval(as);
      rblist userrc = get_home_dir();
      if (rblist_length(userrc) > 0) {
        userrc = rblist_concat(userrc, rblist_from_string("/." PACKAGE "rc"));
        if ((as = file_read(userrc)))
          cmd_eval(as);
      }
    }

    if (!bflag) {
      if (buf) {
        /* FIXME: allow help message to be overwritten by errors from
           loading init files */
        CMDCALL(help_about);
        run();
      }

      term_tidy();
      term_close();
    }
  }

  return 0;
}

#ifdef ALLEGRO
END_OF_MAIN()
#endif
