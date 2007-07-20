-- Produce opts.texi
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

NAME="mkopts"

require "lib"

h = io.open("opts.texi", "w")
assert(h)

h:write("@c Automatically generated file: DO NOT EDIT!\n")
h:write("@table @samp\n")

for l in io.lines(arg[1]) do
  if string.find(l, "^X%(") then
    assert(loadstring(l))()
    local longname, param, doc = unpack(xarg)
    if doc == "" then
      die("empty docstring for --" .. longname)
    end
    h:write("@item --" .. longname .. "\n" .. doc .. "\n\n")
  end
end

h:write("@end table")
h:close()
