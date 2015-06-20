dnl configure.ac
dnl
dnl Copyright (c) 1997-2015 Free Software Foundation, Inc.
dnl
dnl This file is part of Zee.
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
AC_INIT([Zee], [0.7.1], [zee-devel@lists.sourceforge.net])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror std-options foreign silent-rules])

dnl Lua
AX_PROG_LUA

dnl Generate output
AC_CONFIG_FILES([Makefile zee/zee luarocks-config.lua], [chmod +x zee/zee])
AC_OUTPUT
