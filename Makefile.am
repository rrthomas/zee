# Top-level Makefile.am
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

CLOC = cloc --autoconf

ACLOCAL_AMFLAGS = -I m4

LUA_PATH ?= ;
LUA_ENV = LUA_INIT= LUA_INIT_5_2= LUA_PATH="$(abs_srcdir)/src/?.lua;$(LUA_PATH)"

CLEANFILES =
MAINTAINERCLEANFILES =

install_edit = sed \
	-e 's|^require ("|require ("$(PACKAGE)/|g'

EXTRA_DIST =						\
	FAQ						\
	GNUmakefile					\
	maint.mk					\
	$(PACKAGE).rockspec.in				\
	$(PACKAGE)-git.rockspec.in

ROCKSPEC = $(PACKAGE)-$(VERSION)-1.rockspec

doc_DATA = FAQ

include build-aux/Makefile.am
include src/Makefile.am
include tests/Makefile.am

loc:
	$(CLOC) $(PRODUCTIONSOURCES) $(srcdir)/build-aux/mkdot$(PACKAGE).lua

loc-tests:
	$(CLOC) $(ALL_TESTS) $(srcdir)/build-aux/run-tests.lua

WOGER_ARGS = package=$(PACKAGE) package_name=$(PACKAGE_NAME) version=$(VERSION) notes=release-notes-$(VERSION) description="`LUA_INIT= LUA_PATH='$(abs_srcdir)/?.rockspec.in' $(LUA) -l$(PACKAGE) -e 'print (description.summary)'`" dist_type="tar.gz" github_user=rrthomas

$(ROCKSPEC): $(PACKAGE).rockspec
	cp $(PACKAGE).rockspec $@

release: $(ROCKSPEC)
	git diff --exit-code && \
	git push && \
	woger github $(WOGER_ARGS) && \
	LUAROCKS_CONFIG=$(abs_srcdir)/luarocks-config.lua luarocks --tree=$(abs_srcdir)/luarocks make $(ROCKSPEC) && \
	woger freecode,lua $(WOGER_ARGS) && \
	git tag -a -m "Release tag" v$(VERSION) && \
	git push --tags
