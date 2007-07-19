/* Program invocation, startup and shutdown
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2004-2007 Reuben Thomas.
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
#include "config.h"             // Re-include to override overridden
                                // autotools macros
#endif

#include "main.h"
#include "term.h"
#include "extern.h"


#define COPYRIGHT_STRING \
  "Copyright (c) 2003-2007 Reuben Thomas <rrt@sc3d.org>\n"\
  "Copyright (c) 2005-2006 Alistair Turbull <apt1002@mupsych.org>\n"\
  "Copyright (c) 1997-2004 Sandro Sigala <sandro@sigala.it>\n"\
  "Copyright (c) 2004 David A. Capello <dacap@users.sourceforge.net>"

Window win;                     // the window
Buffer *buf = NULL;             // the buffer

int thisflag = 0, lastflag = 0; // the global editor flags

static void run(void)
{
  for (;
       !(thisflag & FLAG_QUIT);
       lastflag = thisflag) {
    if (lastflag & FLAG_NEED_RESYNC)
      resync_display();
    term_display();
    term_refresh();

    thisflag = 0;
    if (lastflag & FLAG_DEFINING_MACRO)
      thisflag |= FLAG_DEFINING_MACRO;

    size_t key = getkey();
    popup_clear();
    minibuf_clear();
    process_key(key);
  }
}

static void segv_sig_handler(int signo)
{
  (void)signo;
  fprintf(stderr, PACKAGE_NAME " crashed. Please send a bug report to " PACKAGE_BUGREPORT "\r\n");
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
  signal(SIGSEGV, segv_sig_handler);
  signal(SIGHUP, other_sig_handler);
  signal(SIGINT, other_sig_handler);
  signal(SIGQUIT, other_sig_handler);
  signal(SIGTERM, other_sig_handler);
  signal(SIGPIPE, SIG_IGN);
}

// Options table
struct option longopts[] = {
#define X(longname, opt, doc) \
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
      // FIXME: Make a file_load command
      if ((as = file_read(rblist_from_string(optarg))))
        cmd_eval(as);
      break;
    case 3:
      nflag = true;
      break;
    case 4:
      hflag = true;
      break;
    case 5:
      fprintf(stderr,
              PACKAGE_STRING "\n"
              COPYRIGHT_STRING "\n"
              PACKAGE_NAME " comes with ABSOLUTELY NO WARRANTY.\n"
              "You may redistribute copies of " PACKAGE_NAME "\n"
              "under the terms of the GNU General Public License.\n"
              "For more information about these matters, see the file named COPYING.\n"
              );
      return 0;
    }
  argc -= optind;
  argv += optind;

  if (hflag || (argc == 0 && optind == 1)) {
    fprintf(stderr,
            "Usage: " PACKAGE " [OPTION...] [+LINE] FILE\n"
            "Run " PACKAGE_NAME ", the editor.\n"
            "\n");
#define X(longname, opt, doc) \
    fprintf(stderr, "--" longname doc);
#include "tbl_opts.h"
#undef X
    fprintf(stderr,
            "FILE               edit FILE\n"
            "+LINE              start editing FILE at line LINE\n"
            );
  } else {
    signal_init();
    setlocale(LC_ALL, "");

    // Open file given on command line.
    while (*argv) {
      if (**argv == '+')
        line = strtoul(*argv++ + 1, NULL, 10);
      else if (*argv) {
        file_open(rblist_from_string(*argv++));
        break;
      }
    }

    if (!bflag) {
      win.fheight = 1; // Initialise window; need fheight > eheight
      term_init();
      resize_window(); // Can't run until there is a buffer
      if (buf && line > 0) {
        goto_line(line - 1);
        resync_display();
      }
    }

    // Load system init file.
    if ((as = file_read(rblist_from_string(SYSCONFDIR "/" PACKAGE "rc"))))
      cmd_eval(as);

    // Load user init file if not disabled.
    if (!nflag) {
      rblist home = get_home_dir();
      if (rblist_length(home) > 0 &&
          (as = file_read(rblist_concat(home, rblist_from_string("/." PACKAGE "rc")))))
        cmd_eval(as);
    }

    if (!bflag) {
      if (buf) {
        // FIXME: allow help message to be overwritten by errors from loading init files
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
