dnl Process this file with autoconf to produce a configure script.

dnl FIXME: Make zee.pkg get version number from here
AC_INIT(zee, 0.2, zee-devel@lists.sourceforge.net)
AM_INIT_AUTOMAKE

dnl create a config.h file (Automake will add -DHAVE_CONFIG_H)
AM_CONFIG_HEADER(config.h)

TEXT_NAME="Zee"
AC_SUBST(TEXT_NAME)
AC_SUBST(VERSION)

dnl Allow GNU functions to be used if present (e.g. vasprintf)
AC_GNU_SOURCE

dnl ==================================================================
dnl Checks for programs.
dnl ==================================================================

AC_PROG_CC
BFD_CC_FOR_BUILD
AC_PROG_INSTALL
AC_PATH_PROG(MAKEINFO, makeinfo)

dnl ==================================================================
dnl Checks for functions.
dnl ==================================================================

AC_REPLACE_FUNCS(vasprintf strrstr getopt_long_only)
AC_CHECK_FUNCS(getcwd fchmod fchown sigaction)

dnl ==================================================================
dnl Checks for header files.
dnl ==================================================================

AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h)

dnl ==================================================================
dnl Check for GNU Regex library
dnl ==================================================================

AC_MSG_CHECKING([for GNU regex.h])
AC_TRY_COMPILE([
#include <sys/types.h>
#include <regex.h>
], [
struct re_pattern_buffer pattern;
struct re_registers search_regs;
re_set_syntax(RE_SYNTAX_EMACS);
re_compile_pattern(".*", 2, &pattern);
re_search(&pattern, "test", 4, 0, 4, &search_regs);
], have_regex_h=true, have_regex_h=false)

if test x$have_regex_h = xtrue ; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_REGEX_H, 1, [Define this to use GNU regex.h.])
else
	AC_MSG_RESULT(no)
fi

AM_CONDITIONAL(HAVE_REGEX_H, test x$have_regex_h = xtrue)

dnl ==================================================================
dnl Turn on additional compiler warnings
dnl ==================================================================
if test "$GCC" = "yes"; then
  CFLAGS="$CFLAGS -Wall -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -pedantic"
fi

dnl ==================================================================
dnl Allegro
dnl ==================================================================

AC_DEFUN([zee_CHECK_ALLEGRO],
[
        AC_CHECK_HEADER(allegro.h,
			AC_DEFINE(HAVE_ALLEGRO_H, 1, [We have the allegro.h header.]),
			AC_MSG_FAILURE([cannot find allegro.h]))
        AC_DEFINE(ALLEGRO, 1, [Define this to use Allegro library.])
])

AC_MSG_CHECKING([use allegro library])
AC_ARG_ENABLE(allegro,
AC_HELP_STRING([--enable-allegro], [use allegro interface]),
[
if test $enableval = yes; then
        allegro=true
        AC_MSG_RESULT(yes)
	zee_CHECK_ALLEGRO
else
        allegro=false
        AC_MSG_RESULT(no)
fi
],[
        allegro=false
        AC_MSG_RESULT(no)
])

AM_CONDITIONAL(ALLEGRO, test x$allegro = xtrue)

dnl ==================================================================
dnl epocemx
dnl ==================================================================

AC_MSG_CHECKING([use epocemx])
AC_ARG_ENABLE(epocemx,
AC_HELP_STRING([--enable-epocemx], [use epocemx interface]),
[
if test $enableval = yes; then
        epocemx=true
        AC_MSG_RESULT(yes)
else
        epocemx=false
        AC_MSG_RESULT(no)
fi
],[
        epocemx=false
        AC_MSG_RESULT(no)
])

AM_CONDITIONAL(EPOCEMX, test x$epocemx = xtrue)

dnl ==================================================================
dnl Curses
dnl ==================================================================

AC_DEFUN([CHECK_CURSES],
[
        AC_MSG_RESULT(yes)
        AC_DEFINE(CURSES, 1, [Define this to use curses.])
        AC_CHECK_HEADER(ncurses.h,AC_DEFINE(HAVE_NCURSES_H, 1, [We have the ncurses.h header.]),
[AC_CHECK_HEADER(curses.h,AC_DEFINE(HAVE_CURSES_H, 1, [We have the curses.h header.]),
        AC_MSG_FAILURE([cannot find either curses.h or ncurses.h]))])
        dnl Check if ncurses library is available as libncurses or libcurses
        AC_CHECK_LIB(ncurses, getch, [LIBS="$LIBS -lncurses"],
        [AC_CHECK_LIB(curses, getch, [LIBS="$LIBS -lcurses"],
AC_MSG_FAILURE([cannot find either libncurses or libcurses]))])
])

AC_MSG_CHECKING([use curses])
AC_ARG_ENABLE(curses,
AC_HELP_STRING([--disable-curses], [don't use curses interface]),
[
if test $enableval = yes; then
        curses=true
        CHECK_CURSES
else
        curses=false
        AC_MSG_RESULT(no)
        AC_CHECK_HEADERS([termcap.h] [termios.h])
        AC_CHECK_LIB(termcap, tgetstr, [LIBS="$LIBS -ltermcap"],
AC_MSG_FAILURE([cannot find libtermcap]))
fi
],[
        curses=true
        CHECK_CURSES
])

AM_CONDITIONAL(CURSES, test x$curses = xtrue)

dnl ==================================================================
dnl Termcap
dnl ==================================================================

if test x$curses = xfalse && test x$allegro = xfalse && test x$epocemx = xfalse; then
        AC_CHECK_HEADERS([termcap.h] [termios.h])
        AC_CHECK_LIB(termcap, tgetstr, [LIBS="$LIBS -ltermcap"],
		AC_CHECK_LIB(ncurses, tgetstr, [LIBS="$LIBS -lncurses"],
                        AC_MSG_FAILURE([cannot find termcap])))
fi

dnl ==================================================================
dnl Extra definitions for config.h
dnl ==================================================================

AH_BOTTOM([
#ifndef HAVE_STRRSTR
extern char *strrstr(const char *, const char *);
#endif

#define TEXT_NAME "Zee"
])

AC_DEFINE_UNQUOTED(CONFIGURE_DATE, ["`date '+%a %b %d %Y'`"],
		   [The date of compilation.])
AC_DEFINE_UNQUOTED(CONFIGURE_HOST, ["`hostname`"],
		   [The host of compilation.])

rm -f src/paths.h

dnl ==================================================================
dnl Generate makefiles
dnl ==================================================================

AC_OUTPUT(Makefile \
	  doc/Makefile \
	  src/Makefile)
