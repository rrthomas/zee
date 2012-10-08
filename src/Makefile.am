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

BUILT_SOURCES = src/loadlua.lua

bin_SCRIPTS = src/$(PACKAGE)

src_@PACKAGE@_LUA =					\
	src/lib.lua					\
	src/astr.lua					\
	src/history.lua					\
	src/eval.lua					\
	src/main.lua					\
	src/variables.lua				\
	src/basic.lua					\
	src/bind.lua					\
	src/buffer.lua					\
	src/completion.lua				\
	src/editfns.lua					\
	src/funcs.lua					\
	src/getkey.lua					\
	src/help.lua					\
	src/file.lua					\
	src/keycode.lua					\
	src/deletebuffer.lua				\
	src/line.lua					\
	src/macro.lua					\
	src/marker.lua					\
	src/minibuf.lua					\
	src/search.lua					\
	src/undo.lua					\
	src/window.lua					\
	src/term.lua					\
	src/term_curses.lua				\
	src/term_minibuf.lua

LUA_RUNTIME =						\
	$(src_$(PACKAGE)_LUA)

dist_pkgdata_DATA =					\
	$(LUA_RUNTIME)					\
	src/loadlua.lua

PRODUCTIONSOURCES =					\
	$(src_$(PACKAGE)_LUA)				\
	$(top_srcdir)/src/$(PACKAGE).lua		\
	$(top_srcdir)/src/loadlua.lua			\
	$(top_srcdir)/src/Makefile.am			\
	$(top_srcdir)/Makefile.am $(top_srcdir)/configure.ac

src/loadlua.lua: src/Makefile.am src/loadlua.lua.in
	cp src/loadlua.lua.in $@
	for i in $(LUA_RUNTIME); do echo 'require "'`echo $$i | sed -e 's|^src/||;s|\\.lua$$||'`'"' >> $@; done

$(top_builddir)/src/$(PACKAGE): $(PRODUCTIONSOURCES) Makefile
	rm -f $@ $@.tmp
	$(inplace_edit) '$@.lua' >$@.tmp
	mv $@.tmp $@
	chmod +x $@
	$(top_builddir)/src/$(PACKAGE) --version >/dev/null || rm $@

src/dot$(PACKAGE).sample: src/variables.lua build-aux/mkdot$(PACKAGE).lua $(top_builddir)/src/$(PACKAGE)
	PACKAGE="$(PACKAGE)" $(builddir)/src/$(PACKAGE) -eval "loadfile'"$(srcdir)/build-aux/mkdot$(PACKAGE).lua"' ()"

install-exec-hook:
	$(install_edit) '$(top_builddir)/src/$(PACKAGE).lua' >$@.tmp
	$(INSTALL_SCRIPT) $@.tmp $(DESTDIR)$(bindir)/$(PACKAGE)
	rm -f $@.tmp

EXTRA_DIST +=						\
	$(LUA_RUNTIME)					\
	src/$(PACKAGE).lua.in					\
	src/loadlua.lua.in				\
	src/dot$(PACKAGE).sample

CLEANFILES +=						\
	src/$(PACKAGE)					\
	src/$(PACKAGE).lua

DISTCLEANFILES +=					\
	$(BUILT_SOURCES)
