# Top-level Makefile.am
#
# Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = BUGS GNUmakefile maint.mk m4/gnulib-cache.m4

dist_pkgdata_DATA = FAQ

include src/Makefile.am

ChangeLog:
	git2cl > ChangeLog

# FIXME: Re-integrate woger's gnu method and the following with output of make {alpha,beta,stable}

# Wait for a URL to become available:
#   u=http://ftp.gnu.org/gnu/vc-dwim/vc-dwim-1.4.tar
#   for i in $(seq 100); do echo $i;
#     wget -O - $u.xz.sig > /dev/null && break; sleep 10
#   done

# Here's a snippet to download and verify xz and gz signatures:
#   u=http://ftp.gnu.org/gnu/vc-dwim/vc-dwim-1.4.tar
#   for i in xz gz xz.sig gz.sig; do
#     wget --quiet $u.$i || break
#     case $i in *.sig) gpg --verify $(basename $u).$i;; esac
#   done
release:
	agrep -d '^\* Note' $(VERSION) NEWS | tail -n +3 | head -n -1 | grep -v '^\*\*' > release-notes && \
	woger freshmeat,gnu $(PACKAGE) "$(PACKAGE_NAME)" $(VERSION) "the tiny Emacs clone" release-notes && \
	rm -f release-notes
