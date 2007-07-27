dnl Process this file with autoconf to produce a configure script.

AC_INIT(Zee, 0.7, zee-devel@lists.sourceforge.net)
AM_INIT_AUTOMAKE

dnl create a config.h file (Automake will add -DHAVE_CONFIG_H)
AM_CONFIG_HEADER(config.h)

dnl Allow GNU functions to be used if present
AC_GNU_SOURCE

dnl Find programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_REPLACE_FUNCS(getopt_long_only)
AC_HEADER_STDC

dnl ==================================================================
dnl Debugging
dnl ==================================================================

AC_MSG_CHECKING([if the debugging code should be included])
AC_ARG_ENABLE(debug,
        AS_HELP_STRING([--enable-debug], [include debugging code]),
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
        ],
        [AC_MSG_RESULT(no)])

dnl ==================================================================
dnl Extra stuff for GCC: add more compiler warnings, and set C99 mode
dnl ==================================================================

if test "$GCC" = "yes"; then
  CFLAGS="$CFLAGS -std=c99 -Wall -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -pedantic"
fi

dnl ==================================================================
dnl Profiling
dnl ==================================================================

AC_MSG_CHECKING([whether profiling is required])
AC_ARG_ENABLE(profiling,
        AS_HELP_STRING([--enable-profiling], [include profiling code]),
        [
        if test $enableval = yes; then
            if test "$GCC" = "yes"; then
                  CFLAGS="$CFLAGS -pg"
            fi
            AC_MSG_RESULT(yes)
        else
            AC_MSG_RESULT(no)
        fi
        ],
        [AC_MSG_RESULT(no)])

dnl ==================================================================
dnl Lua
dnl ==================================================================

AC_SUBST([LUA])
AC_SUBST([LUAC])
AC_SUBST([LUA_INCLUDES])
AC_SUBST([LUA_LIBS])

dnl Arguments
AC_ARG_WITH([lua-prefix],
            [AS_HELP_STRING([--with-lua-prefix=DIR],
                            [Lua files are in DIR])])
AC_ARG_WITH([lua-includes],
            [AS_HELP_STRING([--with-lua-includes=DIR],
                            [Lua include files are in DIR])])
AC_ARG_WITH([lua-libraries],
            [AS_HELP_STRING([--with-lua-libraries=DIR],
                            [Lua library files are in DIR])])
AC_ARG_WITH([lua-suffix],
            [AS_HELP_STRING([--with-lua-suffix=ARG],
                            [Lua binary and library files are suffixed with
                             ARG])])

dnl LUA
if test "x$with_lua_prefix" = x; then
    lua_search_path="$PATH"
else
    lua_search_path="$with_lua_prefix/bin"
fi
if test "x$LUA" = x; then
    AC_PATH_PROG([LUA], [lua$with_lua_suffix], [], [$lua_search_path])
    test "x$LUA" = x && AC_MSG_FAILURE([*** Can't build Zee without Lua 5.1])
fi

dnl lua_version
AC_MSG_CHECKING([for Lua version = 5.1.x])
lua_version=$($LUA -v 2>&1 | head -n 1 | cut -d' ' -f2)
case $lua_version in
5.1*)
    AC_MSG_RESULT([yes (found $lua_version)])
    ;;
*)
    AC_MSG_RESULT([no (found $lua_version)])
    AC_MSG_FAILURE([*** can't build Zee with this version of Lua])
    ;;
esac

dnl LUA_INCLUDES
if test "x$with_lua_includes" != x; then
    LUA_INCLUDES="-I$with_lua_includes"
elif test "x$with_lua_prefix" != x; then
    LUA_INCLUDES="-I$with_lua_prefix/include"
fi
CFLAGS="$CFLAGS $LUA_INCLUDES"
AC_CHECK_HEADERS([lua.h lualib.h], [],
                 [AC_MSG_FAILURE([*** missing headers lua.h or lualib.h.
                  *** install the Lua dev package or check --with-lua-includes option.])], [])

dnl LUA_LIBS
if test "x$with_lua_libraries" != x; then
    LUA_LIBS="-L$with_lua_libraries"
elif test "x$with_lua_prefix" != x; then
    LUA_LIBS="-L$with_lua_prefix/lib"
fi
AC_CHECK_LIB([m], [exp], [lua_extra_libs="$lua_extra_libs -lm"], [])
AC_CHECK_LIB([dl], [dlopen], [lua_extra_libs="$lua_extra_libs -ldl"], [])
AC_CHECK_LIB([lua$with_lua_suffix],
             [lua_call],
             [LUA_LIBS="$LUA_LIBS -llua$with_lua_suffix $lua_extra_libs"],
             [AC_MSG_FAILURE([*** Can't find lua_call in lua$with_lua_suffix.
                  *** Check for liblua installation or --with-lua-libraries or --with-lua-suffix options])],
             [$LUA_LIBS $lua_extra_libs])

dnl liblua_version
AC_MSG_CHECKING([for liblua version 5.1.x])
LIBS="$LIBS $LUA_LIBS"
AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <lua.h>
#include <stdlib.h>
#include <stdio.h>
int main()
{
    printf("(found %s, %d)... ", LUA_VERSION, LUA_VERSION_NUM);
    if (LUA_VERSION_NUM == 501)
        exit(EXIT_SUCCESS);
    exit(EXIT_FAILURE);
}

]])],
              [AC_MSG_RESULT([yes])],
              [AC_MSG_RESULT([no])
               AC_MSG_FAILURE([*** liblua is not version 5.1.
                  *** Check for liblua installation or --with-lua-libraries or --with-lua-suffix options])])

dnl ==================================================================
dnl libgc (BDW garbage collector)
dnl ==================================================================

if test "x$enable_debug" = "no"; then
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
fi

dnl ==================================================================
dnl PCRE
dnl ==================================================================

AC_CHECK_HEADER(pcre.h, ,
        AC_MSG_FAILURE([cannot find pcre.h]))
AC_CHECK_LIB(pcre, pcre_compile, [LIBS="$LIBS -lpcre"],
        AC_MSG_FAILURE([cannot find libpcre]))

dnl ==================================================================
dnl Allegro
dnl ==================================================================

AC_MSG_CHECKING([use allegro library])
AC_ARG_ENABLE(allegro,
              AS_HELP_STRING([--enable-allegro], [use allegro interface]),
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
dnl Generate makefiles
dnl ==================================================================

AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT
