# Top-level Makefile.am
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

CLOC = cloc --autoconf

ACLOCAL_AMFLAGS = -I m4

LUA_PATH ?= ;
LUA_ENV = LUA_PATH="$(abs_srcdir)/src/?.lua;$(LUA_PATH)"

CLEANFILES =
MAINTAINERCLEANFILES =

install_edit = sed \
	-e 's|^require ("|require ("$(PACKAGE)/|g'

inplace_edit = sed \
	-e 's|@in_tree_lua_path[@]|$(abs_top_srcdir)/src/?.lua;$(abs_top_builddir)/src/?.lua|g'

EXTRA_DIST =						\
	FAQ						\
	GNUmakefile					\
	maint.mk					\
	$(PACKAGE).rockspec.in

DISTCLEANFILES = $(PACKAGE).rockspec

ROCKSPEC = $(PACKAGE)-$(VERSION)-1.rockspec

doc_DATA = AUTHORS FAQ NEWS

include build-aux/Makefile.am
include doc/Makefile.am
include src/Makefile.am
include tests/Makefile.am

loc:
	$(CLOC) $(PRODUCTIONSOURCES) $(srcdir)/build-aux/mkdot$(PACKAGE).lua $(srcdir)/cfg.mk $(srcdir)/bootstrap.conf

loc-tests:
	$(CLOC) $(ALL_TESTS) $(srcdir)/build-aux/run-tests.lua

distcheck-hook: syntax-check

WOGER_ARGS = package=$(PACKAGE) package_name=$(PACKAGE_NAME) version=$(VERSION) notes=release-notes-$(VERSION) description="`LUA_INIT= LUA_PATH='$(abs_srcdir)/?.rockspec.in' $(LUA) -l$(PACKAGE) -e 'print (description.summary)'`" dist_type="tar.gz" github_user=rrthomas

# Depend on distcheck so that 'make release' runs distcheck before
# making the rockspec, so the release archive MD5sum is correct in the
# rockspec
$(ROCKSPEC): $(PACKAGE).rockspec distcheck
	sed -e 's/_MD5_/'`$(MD5SUM) $(distdir).tar.gz | cut -d " " -f 1`'/g' < $(PACKAGE).rockspec > $@

release: $(ROCKSPEC)
	git diff --exit-code && \
	git push && \
	woger github $(WOGER_ARGS) && \
	LUAROCKS_CONFIG=$(abs_srcdir)/luarocks-config.lua luarocks --tree=$(abs_srcdir)/luarocks make $(ROCKSPEC) && \
	woger freecode,lua $(WOGER_ARGS) && \
	git tag -a -m "Release tag" v$(VERSION) && \
	git push --tags
