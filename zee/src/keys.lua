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

keyname = {[-1] = "NOKEY"}
function X(key_sym, key_name, text_name, key_code)
  keyname[key_code] = key_name
end
loadstring(key_tbl)()

keycode = {NOKEY = -1}
function X(key_sym, key_name, text_name, key_code)
  keycode[key_name] = key_code
end
loadstring(key_tbl)()

-- Convert a key chord into its ASCII representation
function chordtostr(key)
  local i
  local s = ""

  if bit.band(key, keycode["Ctrl-"]) ~= 0 then
    s = s .. "Ctrl-"
  end
  if bit.band(key, keycode["Alt-"]) ~= 0 then
    s = s .. "Alt-"
  end
  key = bit.band(key, bit.bnot(bit.bor(keycode["Ctrl-"], keycode["Alt-"])))

  if keyname[key] then
    s = s .. keyname[key]
  else
    if string.match(string.char(key), "%C") then
      s = s .. string.char(key);
    else
      s = s .. string.format("<%x>", key);
    end
  end

  return s
end

-- Find a key name as a prefix of `s', returning its keycode and the
-- number of characters consumed
local function strtokey(s)
  for _, v in pairs(keyname) do
    if #s >= #v and string.sub(s, 1, #v) == v then
      return keycode[v], #v
    end
  end

  return string.byte(s, 1), 1
end

-- Convert a key chord string to its key code
function strtochord(chord)
  local key, len = 0, 0
  local k
  repeat
    local l
    k, l = strtokey(string.sub(chord, len + 1, #chord))
    key = bit.bor(key, k)
    len = len + l
  until k ~= keycode["Ctrl-"] and k ~= keycode["Alt-"]

  if len ~= #chord then
    key = keycode["NOKEY"]
  end

  return key
end
