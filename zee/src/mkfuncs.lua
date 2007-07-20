-- Produce funcs.texi and tbl_funcs.h
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

NAME="mkfuncs"

require "lib"

dir = arg[1]
table.remove(arg, 1)

h1 = io.open("funcs.texi", "w")
assert(h1)

h2 = io.open("tbl_funcs.h", "w")
assert(h2)

h1:write("@c Automatically generated file: DO NOT EDIT!\n")
h1:write("@table @code\n")

h2:write("/*\n")
h2:write(" * Automatically generated file: DO NOT EDIT!\n")
h2:write(" * Table of commands (name, doc)\n")
h2:write(" */\n")
h2:write("\n")

for i in ipairs(arg) do
  if arg[i] then
    lines = io.lines(dir .. "/" .. arg[i])
    for l in lines do
      if string.sub(l, 1, 4) == "DEF(" or
        string.sub(l, 1, 8) == "DEF_ARG(" then
        local _, _, name = string.find(l, "%((.-),")
        if name == nil or name == "" then
          die("invalid DEF[_ARG]() syntax `" .. l .. "'")
        end
        
        local state = 0
        local doc = ""
        for l in lines do
          if state == 1 then
            if string.sub(l, 1, 3) == "\")" or
              string.sub(l, 1, 3) == "\"," then
              state = 0
              break
            end
            doc = doc .. l .. "\n"
          elseif string.sub(l, 1, 3) == "\"\\" then
            state = 1
          end
        end

        if doc == "" then
          die("no docstring for " .. name)
        elseif state == 1 then
          die("unterminated docstring for " .. name)
        end

        h1:write("@item " .. name .. "\n" .. doc)
        h2:write("X(" .. name .. ",\n\"\\\n" .. doc .. "\")\n")
      end
    end
  end
end

h1:write("@end table")
h1:close()

h2:close()
