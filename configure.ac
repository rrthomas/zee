dnl configure.ac
dnl
dnl Copyright (c) 1997-2012 Free Software Foundation, Inc.
dnl
dnl This file is part of GNU Zile.
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published
dnl by the Free Software Foundation; either version 3, or (at your
dnl option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_PREREQ([2.61])

dnl Initialise autoconf and automake
AC_INIT([Zile], [2.5.0.50], [bug-zile@gnu.org])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror std-options])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Set up gnulib
gl_EARLY
gl_INIT

dnl Lua 5.1
AX_PROG_LUA([501])

dnl help2man
AX_WITH_PROG([HELP2MAN], [help2man], [false])

dnl Emacs >= 23.1 (optional)
AX_WITH_PROG([EMACSPROG], [emacs])
if test -n "$EMACSPROG"; then
  emacs_min_version=23.1
  emacs_version=$($EMACSPROG --version 2>&1 | head -n 1 | cut -d' ' -f3)
  AC_MSG_CHECKING([Emacs version $emacs_version >= $emacs_min_version])
  AX_COMPARE_VERSION([$emacs_version], [ge], [$emacs_min_version],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])
    unset EMACSPROG])
fi

dnl Generate output
AC_CONFIG_FILES([Makefile src/zile.lua])
AC_OUTPUT
