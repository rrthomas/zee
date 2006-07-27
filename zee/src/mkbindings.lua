-- Produce default_bindings.c
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

NAME="mkbindings"

dofile("lib.lua")

h = io.open("default_bindings.c", "w")
assert(h)

for l in io.lines() do
  if string.find(l, "^key_bind") then
    local _, _, keystroke, cmd = string.find(l, "key_bind (%S+) (%S+)")
    if keystroke == nil or cmd == nil then
      die("bad binding: " .. l)
    end
    h:write("bind_key(strtochord(rblist_from_string(\"" .. keystroke .. "\")), F_" .. cmd .. ");\n")
  end
end

h:close()
