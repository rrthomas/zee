-- Keyboard functions
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

keycode = {}
function X(key_sym, key_name, text_name, key_code)
  table.insert(keycode, key_sym)
end
eval(key_tbl)

keyname = {}
function X(key_sym, key_name, text_name, key_code)
  table.insert(keycode, key_name)
end
eval(key_tbl)

-- Convert a key chord into its ASCII representation
function chordtostr(key)
  local i
  local s = ""

  if bit.band(key, KBD_CTRL) then
    s = s .. "Ctrl-"
  end
  if bit.band(key, KBD_META) then
    s = s .. "Alt-"
  end
  key = bit.band(key, bit.bnot(bit.bor(KBD_CTRL, KBD_META)))

  local found = false
  for i, v in ipairs(keycode) do
    if v == key then
      s = s .. keyname[i]
      found = true
      break
    end
  end

  if not found then
    if string.match(string.char(key), "%C") then
      s = s .. string.char(key);
    else
      s = s .. string.format("<%x>", key);
    end
  end

  return s
end
