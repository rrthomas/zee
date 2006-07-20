dnl Process this file with autoconf to produce a configure script.

AC_INIT(zee, 0.6, zee-devel@lists.sourceforge.net)
AM_INIT_AUTOMAKE

dnl create a config.h file (Automake will add -DHAVE_CONFIG_H)
AM_CONFIG_HEADER(config.h)

TEXT_NAME="Zee"
AC_SUBST(TEXT_NAME)
AC_SUBST(VERSION)

dnl Allow GNU functions to be used if present
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

AC_REPLACE_FUNCS(getopt_long_only)

dnl ==================================================================
dnl Checks for header files.
dnl ==================================================================

AC_HEADER_STDC

dnl ==================================================================
dnl Check for PCRE
dnl ==================================================================

AC_CHECK_HEADER(pcre.h, ,
        AC_MSG_FAILURE([cannot find pcre.h]))
AC_CHECK_LIB(pcre, pcre_compile, [LIBS="$LIBS -lpcre"],
        AC_MSG_FAILURE([cannot find libpcre]))

dnl ==================================================================
dnl Debugging
dnl ==================================================================

AC_MSG_CHECKING([if the debugging code should be included])
AC_ARG_ENABLE(debug,
AC_HELP_STRING([--enable-debug], [include debugging code]),
[
if test $enableval = yes; then
        CFLAGS="-g"
        if test "$GCC" = "yes"; then
          CFLAGS="$CFLAGS -ggdb"
        fi
	AC_MSG_RESULT(yes)
	AC_DEFINE(DEBUG, 1, [Define this to include debugging code.])
else
	AC_MSG_RESULT(no)
fi
], [
AC_MSG_RESULT(no)
])

dnl ==================================================================
dnl Extra stuff for GCC: add more compiler warnings, and set C99 mode
dnl ==================================================================

if test "$GCC" = "yes"; then
  CFLAGS="$CFLAGS -std=c99 -Wall -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -pedantic"
fi

dnl ==================================================================
dnl Allegro
dnl ==================================================================

AC_MSG_CHECKING([use allegro library])
AC_ARG_ENABLE(allegro,
AC_HELP_STRING([--enable-allegro], [use allegro interface]),
[
if test $enableval = yes; then
        allegro=true
        AC_MSG_RESULT(yes)
        AC_CHECK_HEADER(allegro.h, , AC_MSG_FAILURE([cannot find allegro.h]))
        AC_DEFINE(ALLEGRO, 1, [Define this to use Allegro library.])
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
dnl gc
dnl ==================================================================

AC_CHECK_HEADERS([gc.h gc/gc.h],
                 [
                    # To test for the different required libs, we have to
                    # overcome autoconf's caching system, so we change the
                    # desired function name.  They're all in libgc.
                    # The "break" will exit from the top level
                    # AC_CHECK_HEADERS.
                    gc_libs=""
                    AC_CHECK_LIB(gc, GC_init,
                                 [gc_ok=yes;
                                  LIBS="-lgc $gc_libs $LIBS";
                                  break], [gc_ok=no], [$gc_libs])
                    gc_libs="-lpthread"
                    AC_CHECK_LIB(gc, GC_malloc,
                                 [gc_ok=yes;
                                  LIBS="-lgc $gc_libs $LIBS";
                                  break], [gc_ok=no], [$gc_libs])
                    gc_libs="-ldl"
                    AC_CHECK_LIB(gc, GC_realloc,
                                 [gc_ok=yes;
                                  LIBS="-lgc $gc_libs $LIBS";
                                  break], [gc_ok=no], [$gc_libs])
                    gc_libs="-lpthread -ldl"
                    AC_CHECK_LIB(gc, GC_free,
                                 [gc_ok=yes;
                                  LIBS="-lgc $gc_libs $LIBS";
                                  break], [gc_ok=no], [$gc_libs])
                    break],
                 [gc_ok=no])
if test "x$gc_ok" != "xyes"; then
        AC_MSG_FAILURE([cannot find libgc])
fi

dnl ==================================================================
dnl Curses
dnl ==================================================================

AC_DEFUN([CHECK_CURSES],
[
        AC_MSG_CHECKING([use curses])
        AC_MSG_RESULT(yes)
        AC_DEFINE(CURSES, 1, [Define this to use curses.])
        AC_CHECK_HEADER(ncurses.h, AC_DEFINE(HAVE_NCURSES_H, 1, [We have the ncurses.h header.]),
        [AC_CHECK_HEADER(curses.h, AC_DEFINE(HAVE_CURSES_H, 1, [We have the curses.h header.]),
        AC_MSG_FAILURE([cannot find either curses.h or ncurses.h]))])
dnl Check if ncurses library is available as libncurses or libcurses
AC_CHECK_LIB(ncurses, getch, [LIBS="$LIBS -lncurses"],
        [AC_CHECK_LIB(curses, getch, [LIBS="$LIBS -lcurses"],
        AC_MSG_FAILURE([cannot find either libncurses or libcurses]))])
])

if test x$allegro = xfalse; then
        curses=true
        CHECK_CURSES
fi

AM_CONDITIONAL(CURSES, test x$curses = xtrue)

dnl ==================================================================
dnl Extra definitions for config.h
dnl ==================================================================

AH_BOTTOM([
#define TEXT_NAME "Zee"
])

rm -f src/paths.h

dnl ==================================================================
dnl Generate makefiles
dnl ==================================================================

AC_OUTPUT(Makefile \
	  doc/Makefile \
	  src/Makefile)
