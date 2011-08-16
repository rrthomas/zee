# Source Makefile.am
#
# Copyright (c) 1997-2011 Free Software Foundation, Inc.
#
# This file is part of GNU Zile.
#
# GNU Zile is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Zile is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Zile; see the file COPYING.  If not, write to the
# Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
# MA 02111-1301, USA.

BUILT_SOURCES = src/loadlua.lua

bin_SCRIPTS = src/zile

src_zile_LUA =						\
	src/lib.lua					\
	src/history.lua					\
	src/lisp.lua					\
	src/tbl_vars.lua				\
	src/variables.lua				\
	src/main.lua					\
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
	src/killring.lua				\
	src/line.lua					\
	src/macro.lua					\
	src/marker.lua					\
	src/minibuf.lua					\
	src/point.lua					\
	src/redisplay.lua				\
	src/registers.lua				\
	src/search.lua					\
	src/undo.lua					\
	src/window.lua					\
	src/term_curses.lua				\
	src/term_minibuf.lua				\
	src/term_redisplay.lua

LUA_RUNTIME =						\
	src/std.lua					\
	src/ldb.lua					\
	src/ldb-config.lua				\
	$(src_zile_LUA)

dist_pkgdata_DATA =					\
	$(LUA_RUNTIME)					\
	src/loadlua.lua					\
	src/default-bindings.el

PRODUCTIONSOURCES =					\
	$(zile_SOURCES)					\
	$(src_zile_LUA)					\
	$(top_srcdir)/src/ldb-config.lua		\
	$(top_srcdir)/src/Makefile.am			\
	$(top_srcdir)/src/*.el				\
	$(top_srcdir)/Makefile.am $(top_srcdir)/configure.ac

LUA_CLIBS = posix curses bit $(LREX)

src/loadlua.lua: Makefile.am
	echo "-- Generated file - do not edit!" > $@
	for i in $(LUA_CLIBS) $(LUA_RUNTIME); do echo 'require "'`echo $$i | sed -e 's|^src/||;s|\\.lua$$||'`'"' >> $@; done

install_edit = sed \
	-e 's|@pkgdatadir[@]|$(pkgdatadir)|g'

inplace_edit = sed \
	-e 's|@pkgdatadir[@]|$(abs_top_srcdir)/src|g'

$(top_builddir)/src/zile: $(top_builddir)/src/zile.lua Makefile
	rm -f $@ $@.tmp
	$(inplace_edit) '$@.lua' >$@.tmp
	mv $@.tmp $@
	chmod +x $@

install-exec-hook:
	$(install_edit) '$(top_builddir)/src/zile.lua' >$@.tmp
	$(INSTALL_SCRIPT) $@.tmp $(DESTDIR)$(bindir)/zile
	rm -f $@.tmp

EXTRA_DIST +=						\
	$(LUA_RUNTIME)					\
	src/zile.lua.in

CLEANFILES +=						\
	src/zile					\
	src/zile.lua

DISTCLEANFILES +=					\
	$(BUILT_SOURCES)
