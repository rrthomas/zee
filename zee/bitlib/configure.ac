dnl Process this file with autoconf to produce a configure script.

AC_INIT(bitlib, 22, rrt@sc3d.org)
AM_INIT_AUTOMAKE

dnl Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

dnl Extra CFLAGS if we have gcc
if test "$GCC" = yes; then
    CFLAGS="$CFLAGS -Wall -W -Wmissing-prototypes -Wstrict-prototypes -pedantic"
fi

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h unistd.h byteswap.h)

dnl LUA, LUA_INCLUDES, LUA_LIBS

dnl Arguments
AC_ARG_WITH([lua-prefix],
            [AC_HELP_STRING([--with-lua-prefix=DIR],
                            [Lua files are in DIR])])
AC_ARG_WITH([lua-includes],
            [AC_HELP_STRING([--with-lua-includes=DIR],
                            [Lua include files are in DIR])])
AC_ARG_WITH([lua-libraries],
            [AC_HELP_STRING([--with-lua-libraries=DIR],
                            [Lua library files are in DIR])])
AC_ARG_WITH([lua-suffix],
            [AC_HELP_STRING([--with-lua-suffix=ARG],
                            [Lua binary and library files are suffixed with
                             ARG])])

dnl Find Lua

AC_SUBST(LUA)
AC_SUBST(LUA_INCLUDES)
AC_SUBST(LUA_LIBS)

dnl LUA
if test "x$with_lua_prefix" = x; then
    lua_search_path="$PATH"
else
    lua_search_path="$with_lua_prefix/bin"
fi
if test "x$LUA" = x; then
    AC_PATH_PROG([LUA], [lua$with_lua_suffix], [], [$lua_search_path])
    test "x$LUA" = x && AC_MSG_FAILURE([*** Can't build without Lua])
fi

dnl lua_version
AC_MSG_CHECKING([for Lua version >= 5.0])
lua_version=$($LUA -v 2>&1 | head -n 1 | cut -d' ' -f2)
case $lua_version in
5.*)
    AC_MSG_RESULT([yes (found $lua_version)])
    ;;
*)
    AC_MSG_RESULT([no (found $lua_version)])
    AC_MSG_FAILURE([*** Can't build without Lua])
    ;;
esac
dnl

dnl LUA_INCLUDES
if test "x$with_lua_includes" != x; then
    LUA_INCLUDES="-I$with_lua_includes"
elif test "x$with_lua_prefix" != x; then
    LUA_INCLUDES="-I$with_lua_prefix/include"
fi
CPPFLAGS="$CPPFLAGS $LUA_INCLUDES"
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
AC_MSG_CHECKING([for liblua version >= 5.0])
CFLAGS="$CFLAGS $LUA_INCLUDES"
LIBS="$LIBS $LUA_LIBS"
AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <lua.h>
#include <stdlib.h>
#include <stdio.h>
int main()
{
    printf("(found %s, %d)... ", LUA_VERSION, LUA_VERSION_NUM);
    if (LUA_VERSION_NUM >= 500)
        exit(EXIT_SUCCESS);
    exit(EXIT_FAILURE);
}

]])],
              [AC_MSG_RESULT([yes])],
              [AC_MSG_RESULT([no])])


dnl Generate output files.
AC_CONFIG_FILES(Makefile)
AC_OUTPUT
