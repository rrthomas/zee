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
	-e 's|@pkgdatadir[@]|$(PKGDATA_PATH)|g'		\
	-e 's|^require ("|require ("$(PACKAGE)/|g'

inplace_edit = sed \
	-e 's|@pkgdatadir[@]|$(abs_top_srcdir)/src|g'

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

## ------------- ##
## Distribution. ##
## ------------- ##

gitlog_to_changelog	= $(srcdir)/build-aux/gitlog-to-changelog
git_log_fix		= $(srcdir)/build-aux/git-log-fix
changelog		= $(srcdir)/ChangeLog

# Changes before this date are from before Lua was used.
changelog_start_date = 2009-03-30
$(changelog):
	$(AM_V_GEN)if test -d '$(srcdir)/.git'; then \
	  $(gitlog_to_changelog) --amend=$(git_log_fix) \
	      --since=$(changelog_start_date) > '$@T'; \
	  rm -f '$@'; mv '$@T' '$@'; \
	fi

dist-hook: $(changelog)

distcheck-hook: syntax-check

WOGER_ARGS = package=$(PACKAGE) version=$(VERSION) notes=release-notes dist_type="tar.gz"

# Depend on distcheck so that 'make release' runs distcheck before
# making the rockspec, so the release archive MD5sum is correct in the
# rockspec
$(ROCKSPEC): $(PACKAGE).rockspec distcheck
	sed -e 's/_MD5_/'`$(MD5SUM) $(distdir).tar.gz | cut -d " " -f 1`'/g' < $(PACKAGE).rockspec > $@

release: $(ROCKSPEC)
	agrep -d '^\* Note' $(VERSION) NEWS | tail -n +3 | head -n -1 | grep -v '^\*\*' > release-notes && \
	woger github $(WOGER_ARGS) && \
	LUAROCKS_CONFIG=$(abs_srcdir)/luarocks-config.lua luarocks --tree=$(abs_srcdir)/luarocks make $(ROCKSPEC) && \
	woger freecode $(WOGER_ARGS) && \
	rm -f release-notes
