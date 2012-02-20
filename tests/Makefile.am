# Tests Makefile.am
#
# Copyright (c) 1997-2012 Free Software Foundation, Inc.
#
# This file is part of GNU Zile.
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

RUNLISPTESTS = $(LUA) $(srcdir)/tests/run-lisp-tests.lua

TERM ?= vt100

LISP_TESTS =						\
	$(srcdir)/tests/*.el				\
	$(srcdir)/tests/interactive/*.el

LISP_TESTS_ZILE_ONLY =					\
	$(srcdir)/tests/regression/*.el			\
	$(srcdir)/tests/zile-only/*.el			\
	$(srcdir)/tests/zile-only/interactive/*.el

LISP_TESTS_ZILE_ONLY_FIXED_SCREEN =			\
	$(srcdir)/tests/zile-only/fixed-screen/*.el

LISP_TESTS_OUTPUTS =					\
	$(srcdir)/tests/*.output			\
	$(srcdir)/tests/interactive/*.output		\
	$(srcdir)/tests/regression/*.output		\
	$(srcdir)/tests/zile-only/*.output		\
	$(srcdir)/tests/zile-only/interactive/*.output	\
	$(srcdir)/tests/zile-only/fixed-screen/*.output

LISP_ALL_TESTS =					\
	$(LISP_TESTS)					\
	$(LISP_TESTS_ZILE_ONLY)				\
	$(LISP_TESTS_ZILE_ONLY_FIXED_SCREEN)

LISP_TESTS_ENVIRONMENT =				\
	abs_srcdir=$(abs_srcdir)			\
	srcdir=$(srcdir)				\
	abs_builddir=$(abs_builddir)			\
	TERM=$(TERM)

EXTRA_DIST +=						\
	$(LISP_ALL_TESTS)				\
	$(LISP_TESTS_OUTPUTS)				\
	tests/test.input				\
	tests/run-lisp-tests.lua

check-local: $(builddir)/src/zile
	echo $(LISP_TESTS) | $(LUA_ENV) EMACSPROG="$(EMACSPROG)" $(LISP_TESTS_ENVIRONMENT) xargs $(RUNLISPTESTS)
	echo $(LISP_TESTS_ZILE_ONLY) | $(LUA_ENV) EMACSPROG= $(LISP_TESTS_ENVIRONMENT) xargs $(RUNLISPTESTS)
	echo $(LISP_TESTS_ZILE_ONLY_FIXED_SCREEN) | COLUMNS=80 LINES=24 $(LISP_TESTS_ENVIRONMENT) EMACSPROG= xargs $(RUNLISPTESTS)
	$(LISP_TESTS_ENVIRONMENT) $(builddir)/src/zile --unknown-flag --load $(srcdir)/tests/quit.el
