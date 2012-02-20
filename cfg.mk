# Configuration for maintainer-makefile
#
# Copyright (c) 2011-2012 Free Software Foundation, Inc.
#
# This file is part of GNU Zile.
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

# Set format of NEWS
old_NEWS_hash := 40a4c6ac8e6b133a341fecbce7f875a6

# Don't check test outputs
VC_LIST_ALWAYS_EXCLUDE_REGEX = \.(output|diff)$$

# Use dashes instead of lists when updating copyright headers
update-copyright-env = UPDATE_COPYRIGHT_USE_INTERVALS=1

# Trailing spaces are required for correct diff syntax
exclude_file_name_regexp--sc_trailing_blank = \.diff$

# Lua-ish, not GNU-ish, error messages in some internal files
exclude_file_name_regexp--sc_unmarked_diagnostics = ^src/(std|ldb)\.lua$$

# We aren't internationalised
translation_project_ = ""

# Pig & Whistle
exclude_file_name_regexp--sc_prohibit_doubled_word = \.lua$$

local-checks-to-skip = \
	sc_bindtextdomain \
	sc_error_message_period \
	sc_error_message_uppercase

# Rationale:
#
# sc_bindtextdomain: Emacs isn't internationalised
# sc_error_message_{period,uppercase}: Emacs does these
