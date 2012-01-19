-- Getting and ungetting key strokes
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

local _last_key

-- Return last key pressed
function lastkey ()
  return _last_key
end

-- Get a keystroke, waiting for up to GETKEY_DELAY ms, and translate
-- it into a keycode.
function getkey (delay)
  _last_key = term_getkey (delay)

  if thisflag.defining_macro then
    add_key_to_cmd (_last_key)
  end

  return _last_key
end

function getkey_unfiltered (delay)
  local c = term_getkey_unfiltered (delay)
  _last_key = c
  if thisflag.defining_macro then
    add_key_to_cmd (c)
  end
  return c
end

-- Wait for GETKEY_DELAYED ms or until a key is pressed.
-- The key is then available with getkey.
function waitkey ()
  ungetkey (getkey (GETKEY_DELAYED))
end

-- Push a key into the input buffer.
function pushkey (key)
  term_ungetkey (key)
end

-- Unget a key as if it had not been fetched.
function ungetkey (key)
  pushkey (key)

  if thisflag.defining_macro then
    remove_key_from_cmd ()
  end
end
