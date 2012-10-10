-- Key encoding and decoding functions
--
-- Copyright (c) 2010-2012 Free Software Foundation, Inc.
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


-- Map of key names to code.
local KBD_NONPRINT = 283
local keynametocode = {
  ["SPC"] = string.byte (' '),
}

for i in list.elems {
  "BACKSPACE", "DELETE", "DOWN", "END", "F1", "F10", "F11", "F12",
  "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "HOME", "INSERT",
  "LEFT", "NEXT", "PAGEDOWN", "PAGEUP", "PRIOR", "RET", "RIGHT",
  "TAB", "UP", "\t" } do
  keynametocode[i] = KBD_NONPRINT
end

for i = 0x0, 0x7f do
  if posix.isprint (string.char (i)) and i ~= string.byte ('\\') then
    keynametocode[string.char (i)] = i
  end
end

-- Array of key names
local keyname = set.new {
  "BACKSPACE", "C-", "DELETE", "DOWN", "ESC", "END", "F1", "F10",
  "F11", "F12", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9",
  "HOME", "INSERT", "LEFT", "M-", "NEXT", "PAGEDOWN", "PAGEUP", "RET",
  "RIGHT", "SPC", "TAB", "UP",
}

-- Insert printable characters in the ASCII range.
for i = 0, 0x7f do
  if posix.isprint (string.char (i)) then
    keyname:insert (string.char (i))
  end
end

-- A key code has one `key' and some optional modifiers.
-- For comparisons to work, keycodes are immutable atoms.
local keycode_mt = {
  -- Output the write syntax for this keycode (e.g. C-M-F1).
  __tostring = function (self)
    if not self then
      return "invalid keycode: nil"
    end

    local s = (self.CTRL and "C-" or "") .. (self.META and "M-" or "")

    if not self.key then
      return "invalid keycode: " .. s .. "nil"
    end

    return s .. self.key
  end,

  -- Normalise modifier lookups to uppercase, sans `-' suffix.
  --   hasmodifier = keycode.META or keycode["c"]
  __index = function (self, mod)
    mod = mod:sub (1, 1):upper ()
    return rawget (self, mod)
  end,

  -- Return the immutable atom for this keycode with modifier added.
  --   ctrlkey = "C-" + key
  __add = function (self, mod)
    if "string" == type (self) then mod, self = self, mod end
    mod = mod:sub (1, 1):upper ()
    if self[mod] then return self end
    return keycode (mod .. "-" .. tostring (self))
  end,

  -- Return the immutable atom for this keycode with modifier removed.
  --   withoutmeta = key - "M-"
  __sub = function (self, mod)
    if "string" == type (self) then mod, self = self, mod end
    mod = mod:sub (1, 1):upper ()
    local keystr = tostring (self):gsub (mod .. "%-", "")
    return keycode (keystr)
  end,
}

-- Extract a modifier prefix of a key string.
local modifier_name = set.new {"C-", "M-"}
local function getmodifier (s)
  for match in modifier_name:elems () do
    if match == s:sub (1, #match) then
      return match, s:sub (#match + 1)
    end
  end
  return nil, s
end

-- Convert a single keychord string to its key code.
keycode = memoize (function (chord)
  local key, tail = setmetatable ({}, keycode_mt), chord

  local mod
  repeat
    mod, tail = getmodifier (tail)
    if mod == "C-" then
      key.CTRL = true
    elseif mod == "M-" then
      key.META = true
    end
  until not mod
  if not keyname:member (tail) then return nil end
  key.key = tail
  key.code = keynametocode[tail]

  return key
end)
