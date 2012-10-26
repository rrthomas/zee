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

RUNTESTS = $(LUA) $(srcdir)/build-aux/run-tests.lua

TERM ?= vt100

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

EXTRA_DIST +=						\
	$(ALL_TESTS)					\
	$(TEST_OUTPUTS)					\
	tests/test.input				\
	tests/run-tests.lua

# FIXME: nproc and xargs --max-procs are GNU-specific
check-local: $(builddir)/src/zee
	echo $(TESTS_FIXED_SCREEN) | COLUMNS=80 LINES=24 $(TESTS_ENVIRONMENT) xargs $(RUNTESTS) > /dev/null
	NPROC=`nproc`; \
	echo $(TESTS_NORMAL) | $(LUA_ENV) $(TESTS_ENVIRONMENT) xargs --max-procs=$$NPROC --max-args=$$(( `echo $(TESTS_NORMAL) | wc -w` / $$NPROC + 1 )) $(RUNTESTS) > /dev/null
