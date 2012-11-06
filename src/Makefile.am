# Source Makefile.am
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

doc_DATA += src/dot$(PACKAGE).sample

BUILT_SOURCES =						\
	src/loadprog.lua				\
	src/$(PACKAGE).lua

bin_SCRIPTS = src/$(PACKAGE)

LUA_RUNTIME =						\
	src/astr.lua					\
	src/eval.lua					\
	src/main.lua					\
	src/variables.lua				\
	src/move.lua					\
	src/buffer.lua					\
	src/completion.lua				\
	src/shell.lua					\
	src/getkey.lua					\
	src/help.lua					\
	src/file.lua					\
	src/cut.lua					\
	src/edit.lua					\
	src/macro.lua					\
	src/minibuf.lua					\
	src/search.lua					\
	src/undo.lua					\
	src/term.lua					\
	src/keycode.lua					\
	src/bind.lua					\
	src/cua_bindings.lua				\
	src/term_curses.lua

dist_pkgdata_DATA = $(LUA_RUNTIME)

pkgdata_DATA = src/loadprog.lua

PRODUCTIONSOURCES =					\
	$(LUA_RUNTIME)					\
	$(top_srcdir)/src/$(PACKAGE).lua		\
	$(top_srcdir)/src/loadprog.lua.in		\
	$(top_srcdir)/src/Makefile.am			\
	$(top_srcdir)/tests/Makefile.am			\
	$(top_srcdir)/build-aux/Makefile.am		\
	$(top_srcdir)/Makefile.am $(top_srcdir)/configure.ac

$(top_builddir)/src/loadprog.lua: src/Makefile.am $(top_srcdir)/src/loadprog.lua.in
	cat $(top_srcdir)/src/loadprog.lua.in > $@
	$(LUA) -e "require 'std'; loadfile ('$(PACKAGE).rockspec') (); local modules = {stdlib = 'std', luaposix = 'posix', ['lrexlib-gnu'] = 'rex_gnu'}; list.map (function (s) local l, v = s:match ('([%w-_]+) >= ([%d%.]+)'); if l then local name = modules[l] or l; print (name .. ' = require_version (\"' .. name .. '\", \"' .. v .. '\")') end end, dependencies)" >> $@
	for i in $(LUA_RUNTIME); do echo 'require ("'`echo $$i | sed -e 's|^src/||;s|\\.lua$$||'`'")' >> $@; done

$(top_builddir)/src/$(PACKAGE): $(PRODUCTIONSOURCES) $(BUILT_SOURCES) Makefile
	rm -f $@ $@.tmp
	$(inplace_edit) '$@.lua' >$@.tmp
	mv $@.tmp $@
	chmod +x $@
	$(top_builddir)/src/$(PACKAGE) --version >/dev/null || ( cat $@ && rm $@ )

$(top_builddir)/src/dot$(PACKAGE).sample: src/variables.lua build-aux/mkdot$(PACKAGE).lua $(top_builddir)/src/$(PACKAGE)
	PACKAGE="$(PACKAGE)" PACKAGE_NAME="$(PACKAGE_NAME)" $(builddir)/src/$(PACKAGE) -eval "loadfile ('$(srcdir)/build-aux/mkdot$(PACKAGE).lua') ()"

install-exec-hook:
	$(install_edit) '$(top_builddir)/src/$(PACKAGE).lua' >$@.tmp
	$(INSTALL_SCRIPT) $@.tmp $(DESTDIR)$(bindir)/$(PACKAGE)
	rm -f $@.tmp

install-data-hook:
	$(install_edit) '$(top_builddir)/src/loadprog.lua' >$@.tmp
	$(INSTALL_DATA) $@.tmp $(DESTDIR)$(pkgdatadir)/loadprog.lua
	rm -f $@.tmp

EXTRA_DIST +=						\
	$(LUA_RUNTIME)					\
	src/$(PACKAGE).lua.in				\
	src/loadprog.lua.in

CLEANFILES +=						\
	src/$(PACKAGE)

DISTCLEANFILES +=					\
	$(BUILT_SOURCES)				\
	src/dot$(PACKAGE).sample
