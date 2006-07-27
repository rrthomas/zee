-- Produce vars.texi and zeerc
-- Copyright (c) 2006 Reuben Thomas.  All rights reserved.
--
-- This file is part of Zee.
--
-- Zee is free software; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free
-- Software Foundation; either version 2, or (at your option) any later
-- version.
--
-- Zee is distributed in the hope that it will be useful, but WITHOUT ANY
-- WARRANTY; without even the implied warranty of MERCHANTABILITY or
-- FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
-- for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with Zee; see the file COPYING.  If not, write to the Free
-- Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
-- 02111-1301, USA.

NAME="mkvars"

dofile("lib.lua")

h1 = io.open("vars.texi", "w")
assert(h1)

h2 = io.open(PACKAGE .. "rc", "w")
assert(h2)

h1:write("@c Automatically generated file: DO NOT EDIT!\n")
h1:write("@table @code\n")

h2:write("# ." .. PACKAGE .. " configuration\n")

for l in io.lines("tbl_vars.h") do
  if string.find(l, "^X%(") then
    assert(loadstring(l))()
    local name, type, defval, doc = unpack(xarg)
    if doc == "" then
      die("empty docstring for " .. name)
    end

    h1:write("@item " .. name .. "\n" .. doc .. "\n")

    h2:write("\n# " .. doc .. " [default: " .. defval .. "]\n")
    h2:write("set_variable " .. name .. " " .. defval .. "\n")
  end
end

h1:write("@end table")
h1:close()

h2:close()
