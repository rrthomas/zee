-- Produce keys.texi
-- Copyright (c) 2007 Reuben Thomas.  All rights reserved.
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

NAME="mkkeys"

require "lib"

h = io.open("keys.texi", "w")
assert(h)

h:write("@c Automatically generated file: DO NOT EDIT!\n")
h:write("@table @key\n")

for l in io.lines(arg[1]) do
  if string.find(l, "^X%(") then
    assert(loadstring(l))()
    local key_sym, key_name, text_name, key_code = unpack(xarg)
    h:write("@item " .. key_name .. "\n" .. text_name .. "\n")
  end
end

h:write("@end table")
h:close()
