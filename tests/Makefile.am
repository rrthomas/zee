# Tests Makefile.am
#
# Copyright (c) 1997-2014 Free Software Foundation, Inc.
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

RUNTESTS = $(LUA) $(srcdir)/build-aux/run-tests.lua

TERM ?= vt100

# FIXME: Make interactive tests look up keybindings so they are resistant to default bindings changing
TESTS_NORMAL =						\
	$(srcdir)/tests/*.lua				\
	$(srcdir)/tests/interactive/*.lua		\
	$(srcdir)/tests/regression/*.lua

TESTS_FIXED_SCREEN =					\
	$(srcdir)/tests/fixed-screen/*.lua

TEST_OUTPUTS =						\
	$(srcdir)/tests/*.output			\
	$(srcdir)/tests/interactive/*.output		\
	$(srcdir)/tests/regression/*.output		\
	$(srcdir)/tests/fixed-screen/*.output

ALL_TESTS =						\
	$(TESTS_NORMAL)					\
	$(TESTS_FIXED_SCREEN)

TESTS_ENVIRONMENT =					\
	abs_srcdir=$(abs_srcdir)			\
	srcdir=$(srcdir)				\
	abs_builddir=$(abs_builddir)			\
	PACKAGE=$(PACKAGE)				\
	TERM=$(TERM)

# Use to find which test hangs:
check-debug:
	$(LUA_ENV) $(TESTS_ENVIRONMENT) $(RUNTESTS) $(TESTS_NORMAL) > /dev/null

# FIXME: nproc and xargs --max-procs are GNU-specific
check-local: $(builddir)/zee/zee
	COLUMNS=80 LINES=24 $(TESTS_ENVIRONMENT) $(RUNTESTS) $(TESTS_FIXED_SCREEN) > /dev/null
	NPROC=`nproc`; \
	echo $(TESTS_NORMAL) | $(LUA_ENV) $(TESTS_ENVIRONMENT) xargs --max-procs=$$NPROC --max-args=$$(( `echo $(TESTS_NORMAL) | wc -w` / $$NPROC + 1 )) $(RUNTESTS) > /dev/null
