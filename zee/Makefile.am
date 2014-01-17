# Source Makefile.am
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

BUILT_SOURCES =						\
	$(PACKAGE)/init.lua				\
	$(PACKAGE)/$(PACKAGE).in

bin_SCRIPTS = $(PACKAGE)/$(PACKAGE)

LUA_RUNTIME =						\
	$(PACKAGE)/astr.lua				\
	$(PACKAGE)/eval.lua				\
	$(PACKAGE)/main.lua				\
	$(PACKAGE)/variables.lua			\
	$(PACKAGE)/move.lua				\
	$(PACKAGE)/buffer.lua				\
	$(PACKAGE)/completion.lua			\
	$(PACKAGE)/shell.lua				\
	$(PACKAGE)/getkey.lua				\
	$(PACKAGE)/help.lua				\
	$(PACKAGE)/file.lua				\
	$(PACKAGE)/cut.lua				\
	$(PACKAGE)/edit.lua				\
	$(PACKAGE)/macro.lua				\
	$(PACKAGE)/minibuf.lua				\
	$(PACKAGE)/search.lua				\
	$(PACKAGE)/undo.lua				\
	$(PACKAGE)/term.lua				\
	$(PACKAGE)/keycode.lua				\
	$(PACKAGE)/bind.lua				\
	$(PACKAGE)/cua_bindings.lua			\
	$(PACKAGE)/term_curses.lua

pkgdata_DATA = $(PACKAGE)/init.lua			\
	$(LUA_RUNTIME)					\
	$(PACKAGE)/emacs_bindings.lua

PRODUCTIONSOURCES =					\
	$(LUA_RUNTIME)					\
	$(top_srcdir)/$(PACKAGE)/$(PACKAGE).in		\
	$(top_srcdir)/$(PACKAGE)/init.lua.in		\
	$(top_srcdir)/$(PACKAGE)/Makefile.am		\
	$(top_srcdir)/tests/Makefile.am			\
	$(top_srcdir)/Makefile.am $(top_srcdir)/configure.ac

$(top_builddir)/$(PACKAGE)/init.lua: $(PACKAGE)/Makefile.am $(top_srcdir)/$(PACKAGE)/init.lua.in
	cat $(top_srcdir)/$(PACKAGE)/init.lua.in > $@
	for i in $(LUA_RUNTIME); do echo 'require ("$(PACKAGE)/'`echo $$i | sed -e 's|^$(PACKAGE)/||;s|\\.lua$$||'`'")' >> $@; done

install-data-hook:
	PACKAGE="$(PACKAGE)" PACKAGE_NAME="$(PACKAGE_NAME)" $(LUA_ENV) $(LUA) $(DESTDIR)$(bindir)/$(PACKAGE) -eval "loadfile ('$(srcdir)/build-aux/mkdot$(PACKAGE).lua') ()" > $@.tmp
	$(INSTALL_DATA) $@.tmp $(DESTDIR)$(pkgdatadir)/dot$(PACKAGE).sample
	rm -f $@.tmp
