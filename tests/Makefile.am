# Tests Makefile.am
#
# Copyright (c) 1997-2012 Free Software Foundation, Inc.
#
# This file is part of Zee.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

RUNLUATESTS = $(LUA) $(srcdir)/build-aux/run-tests.lua

TERM ?= vt100

LUA_TESTS =						\
	$(srcdir)/tests/*.lua				\
	$(srcdir)/tests/interactive/*.lua		\
	$(srcdir)/tests/regression/*.lua

LUA_TESTS_FIXED_SCREEN =				\
	$(srcdir)/tests/fixed-screen/*.lua

LUA_TESTS_OUTPUTS =					\
	$(srcdir)/tests/*.output			\
	$(srcdir)/tests/interactive/*.output		\
	$(srcdir)/tests/regression/*.output		\
	$(srcdir)/tests/fixed-screen/*.output

LUA_ALL_TESTS =						\
	$(LUA_TESTS)					\
	$(LUA_TESTS_FIXED_SCREEN)

LUA_TESTS_ENVIRONMENT =					\
	abs_srcdir=$(abs_srcdir)			\
	srcdir=$(srcdir)				\
	abs_builddir=$(abs_builddir)			\
	PACKAGE=$(PACKAGE)				\
	TERM=$(TERM)

EXTRA_DIST +=						\
	$(LUA_ALL_TESTS)				\
	$(LUA_TESTS_OUTPUTS)				\
	tests/test.input				\
	tests/run-lisp-tests.lua

# FIXME: nproc and xargs --max-procs are GNU-specific
check-local: $(builddir)/src/zee
	echo $(LUA_TESTS_FIXED_SCREEN) | COLUMNS=80 LINES=24 $(LUA_TESTS_ENVIRONMENT) xargs $(RUNLUATESTS) > /dev/null
	NPROC=`nproc`; \
	echo $(LUA_TESTS) | $(LUA_ENV) $(LUA_TESTS_ENVIRONMENT) xargs --max-procs=$$NPROC --max-args=$$(( `echo $(LUA_TESTS) | wc -w` / $$NPROC + 1 )) $(RUNLUATESTS) > /dev/null
