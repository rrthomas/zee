# Configuration for maintainer-makefile
#
# Copyright (c) 2011-2012 Free Software Foundation, Inc.
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

GNULIB_SRCDIR ?= $(srcdir)/gnulib
gnulib_dir = $(GNULIB_SRCDIR)

# Don't check test outputs or diffs
VC_LIST_ALWAYS_EXCLUDE_REGEX = \.(output|diff)$$

# Use dashes instead of lists when updating copyright headers
update-copyright-env = UPDATE_COPYRIGHT_USE_INTERVALS=1

# We aren't internationalised
translation_project_ = ""

# Allow errors to start with a capital (they are displayed on a
# separate line, interactively)
local-checks-to-skip = \
	sc_error_message_uppercase	\
	sc_immutable_NEWS
