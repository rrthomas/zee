# Top-level Makefile.am
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

CLOC = cloc --force-lang="Lua",in

ACLOCAL_AMFLAGS = -I m4

CLEANFILES =
DISTCLEANFILES =
MAINTAINERCLEANFILES =

install_edit = sed \
	-e 's|@pkgdatadir[@]|$(pkgdatadir)|g' \
	-e 's|@pkgdocdir[@]|$(docdir)|g'

inplace_edit = sed \
	-e 's|@pkgdatadir[@]|$(abs_top_srcdir)/src|g' \
	-e 's|@pkgdocdir[@]|$(abs_top_srcdir)|g'

EXTRA_DIST =						\
	FAQ						\
	GNUmakefile					\
	build-aux/zile-help2man-wrapper			\
	maint.mk

doc_DATA = AUTHORS FAQ NEWS

include doc/Makefile.am
include src/Makefile.am
include tests/Makefile.am

loc:
	$(CLOC) $(PRODUCTIONSOURCES)

loc-tests:
	$(CLOC) $(TESTSOURCES)

loc-all:
	$(CLOC) $(TESTSOURCES) $(PRODUCTIONSOURCES)

ChangeLog:
	git2cl > ChangeLog

release:
	agrep -d '^\* Note' $(VERSION) NEWS | tail -n +3 | head -n -1 | grep -v '^\*\*' > release-notes && \
	woger freshmeat,gnu upload=alpha.gnu.org package=$(PACKAGE) package_name="$(PACKAGE_NAME)" version=$(VERSION) description="the tiny Emacs clone" notes=release-notes dist_type="tar.gz" && \
	rm -f release-notes
