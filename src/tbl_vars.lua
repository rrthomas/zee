-- Editor variables
--
-- Copyright (c) 1997-2010, 2012 Free Software Foundation, Inc.
--
-- This file is part of Zee.
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

vars = {}
function Var (name, default_value, docstring)
  vars[name] = {val = default_value, doc = texi (docstring)}
end

Var ("tab-width", 8,
     "Number of spaces displayed for a tab character.")
Var ("indent-tabs-mode", true,
     "If true, insert-tab inserts \"real\" tabs; otherwise, it always inserts\nspaces.")
Var ("wrap-column", 70,
     "Column beyond which wrapping occurs in wrap mode.")
Var ("wrap-mode", false,
     "Whether wrap mode is automatically enabled.")
Var ("caseless-search", true,
     "Whether searching ignores case by default.")
Var ("case-replace", false,
     "Whether `query-replace' should preserve case.")
