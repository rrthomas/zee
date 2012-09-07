-- Zile variables
--
-- Copyright (c) 1997-2010, 2012 Free Software Foundation, Inc.
--
-- This file is part of GNU Zile.
--
-- This program is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3, or (at your option)
-- any later version.
--
-- This program is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.

X ("standard-indent", "4", false, "Default number of columns for margin-changing functions to indent.")
X ("tab-width", "8", true, "Distance between tab stops (for display of tab characters), in columns.")
X ("tab-always-indent", "t", false, "Controls the operation of the @kbd{TAB} key.\nIf @samp{t}, hitting @kbd{TAB} always just indents the current line.\nIf @samp{nil}, hitting @kbd{TAB} indents the current line if point is at the\nleft margin or in the line's indentation, otherwise it inserts a\n\"real\" TAB character.")
X ("indent-tabs-mode", "t", true, "If non-nil, insert-tab inserts \"real\" tabs; otherwise, it always inserts\nspaces.")
X ("fill-column", "70", true, "Column beyond which automatic line-wrapping should happen.\nAutomatically becomes buffer-local when set in any fashion.")
X ("auto-fill-mode", "nil", false, "If non-nil, Auto Fill Mode is automatically enabled.")
X ("kill-whole-line", "nil", false, "If non-nil, `kill-line' with no arg at beg of line kills the whole line.")
X ("case-fold-search", "t", true, "Non-nil means searches ignore case.")
X ("case-replace", "t", false, "Non-nil means `query-replace' should preserve case in replacements.")
X ("ring-bell", "t", false, "Non-nil means ring the terminal bell on any error.")
