-- Key encoding and decoding functions
--
-- Copyright (c) 2010-2011 Free Software Foundation, Inc.
--
-- This file is part of GNU Zile.
--
-- GNU Zile is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3, or (at your option)
-- any later version.
--
-- GNU Zile is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with GNU Zile; see the file COPYING.  If not, write to the
-- Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
-- MA 02111-1301, USA.

-- Common non-alphanumeric keys.
local KBD_CANCEL = 257
local KBD_TAB = 258
local KBD_RET = 259
local KBD_PGUP = 260
local KBD_PGDN = 261
local KBD_HOME = 262
local KBD_END = 263
local KBD_DEL = 264
local KBD_BS = 265
local KBD_INS = 266
local KBD_LEFT = 267
local KBD_RIGHT = 268
local KBD_UP = 269
local KBD_DOWN = 270
local KBD_F1 = 272
local KBD_F2 = 273
local KBD_F3 = 274
local KBD_F4 = 275
local KBD_F5 = 276
local KBD_F6 = 277
local KBD_F7 = 278
local KBD_F8 = 279
local KBD_F9 = 280
local KBD_F10 = 281
local KBD_F11 = 282
local KBD_F12 = 283

local codetoname = {
  [KBD_PGUP]  = "<prior>",
  [KBD_PGDN]  = "<next>",
  [KBD_HOME]  = "<home>",
  [KBD_END]   = "<end>",
  [KBD_DEL]   = "<delete>",
  [KBD_BS]    = "<backspace>",
  [KBD_INS]   = "<insert>",
  [KBD_LEFT]  = "<left>",
  [KBD_RIGHT] = "<right>",
  [KBD_UP]    = "<up>",
  [KBD_DOWN]  = "<down>",
  [KBD_RET]   = "<RET>",
  [KBD_TAB]   = "<TAB>",
  [KBD_F1]    = "<f1>",
  [KBD_F2]    = "<f2>",
  [KBD_F3]    = "<f3>",
  [KBD_F4]    = "<f4>",
  [KBD_F5]    = "<f5>",
  [KBD_F6]    = "<f6>",
  [KBD_F7]    = "<f7>",
  [KBD_F8]    = "<f8>",
  [KBD_F9]    = "<f9>",
  [KBD_F10]   = "<f10>",
  [KBD_F11]   = "<f11>",
  [KBD_F12]   = "<f12>",
  [string.byte(' ')] = "SPC",
  [string.byte('\t')] = "TAB",
}

-- Convert a key chord into its ASCII representation
function chordtodesc (key)
  local s = ""

  if bit.band (key, KBD_CTRL) ~= 0 then
    s = s .. "C-"
  end
  if bit.band (key, KBD_META) ~= 0 then
    s = s .. "M-"
  end
  key = bit.band (key, bit.bnot (bit.bor (KBD_CTRL, KBD_META)))

  if codetoname[key] then
    s = s .. codetoname[key]
  elseif key <= 0xff and posix.isgraph (string.char (key)) then
    s = s .. string.char (key)
  else
    s = s .. string.format ("<%x>", key)
  end

  return s
end


-- Convert a key code sequence into a descriptive string.
function keyvectodesc (keys)
  return table.concat (list.map (chordtodesc, keys), " ")
end


-- Array of key names
local keynametocode_map = {
  ["\\BACKSPACE"] = KBD_BS,
  ["\\C-"] = KBD_CTRL,
  ["\\DELETE"] = KBD_DEL,
  ["\\DOWN"] = KBD_DOWN,
  ["\\e"] = 27, -- Escape or ^[
  ["\\END"] = KBD_END,
  ["\\F1"] = KBD_F1,
  ["\\F10"] = KBD_F10,
  ["\\F11"] = KBD_F11,
  ["\\F12"] = KBD_F12,
  ["\\F2"] = KBD_F2,
  ["\\F3"] = KBD_F3,
  ["\\F4"] = KBD_F4,
  ["\\F5"] = KBD_F5,
  ["\\F6"] = KBD_F6,
  ["\\F7"] = KBD_F7,
  ["\\F8"] = KBD_F8,
  ["\\F9"] = KBD_F9,
  ["\\HOME"] = KBD_HOME,
  ["\\INSERT"] = KBD_INS,
  ["\\LEFT"] = KBD_LEFT,
  ["\\M-"] = KBD_META,
  ["\\NEXT"] = KBD_PGDN,
  ["\\PAGEDOWN"] = KBD_PGDN,
  ["\\PAGEUP"] = KBD_PGUP,
  ["\\PRIOR"] = KBD_PGUP,
  ["\\r"] = KBD_RET, -- FIXME: Kludge to make keystrings work in both Emacs and Zile.
  ["\\RET"] = KBD_RET,
  ["\\RIGHT"] = KBD_RIGHT,
  ["\\SPC"] = string.byte (' '),
  ["\\t"] = KBD_TAB,
  ["\\TAB"] = KBD_TAB,
  ["\\UP"] = KBD_UP,
  ["\t"] = string.byte ('\t'),
  ["\\\\"] = string.byte ('\\'),
}

-- Convert a prefix of a key string to its key code.
local function strtokey (s)
  if s[1] == '\\' then
    local p
    for i in pairs (keynametocode_map) do
      if i == string.sub (s, 1, #i) then
        p = i
        break
      end
    end
    if p then
      return string.sub (s, #p + 1), keynametocode_map [p]
    end
    return "", nil
  end

  return string.sub (s, 2), string.byte (s)
end

local function strtochord (s)
  local key = 0

  local k
  repeat
    s, k = strtokey (s)
    if k == nil then
      return "", nil
    end
    key = bit.bor (key, k)
  until k ~= KBD_CTRL and k ~= KBD_META

  return s, key
end

-- Convert a keychord string to its key code.
function keycode (s)
  local _, key = strtochord (s)
  return key
end

-- Convert a key sequence string into a key code sequence, or nil if
-- it can't be converted.
function keystrtovec (s)
  local keys = {}

  while s ~= "" do
    local code
    s, code = strtochord (s)
    if code == nil then
      return nil
    end
    table.insert (keys, code)
  end

  return keys
end
