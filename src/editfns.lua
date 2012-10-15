-- Useful editing functions
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

-- Signal an error, and abort any ongoing macro definition.
function ding ()
  if thisflag.defining_macro then
    cancel_macro_definition ()
  end

  if win then
    term_beep ()
  end
  -- enable call chaining with `return ding ()'
  return true
end


function get_line ()
  return get_buffer_region (buf, region_new (get_buffer_line_o (buf), buffer_end_of_line (buf, get_buffer_pt (buf))))
end

function is_empty_line ()
  return buffer_line_len (buf) == 0
end

function is_blank_line ()
  return regex_match (get_line (), "^[ \t]*$")
end

-- Returns the character following the cursor.
function following_char ()
  if end_of_buffer () then
    return nil
  elseif end_of_line () then
    return '\n'
  else
    return get_buffer_char (buf, get_buffer_pt (buf))
  end
end

-- Return the character preceding the cursor.
function preceding_char ()
  if beginning_of_buffer () then
    return nil
  elseif beginning_of_line () then
    return '\n'
  else
    return get_buffer_char (buf, get_buffer_pt (buf) - 1)
  end
end

-- Return true if cursor is at the beginning of the buffer.
function beginning_of_buffer ()
  return get_buffer_pt (buf) == 1
end

-- Return true if cursor is at the end of the buffer.
function end_of_buffer (void)
  return get_buffer_pt (buf) > get_buffer_size (buf)
end

-- Return true if cursor is at the beginning of a line.
function beginning_of_line ()
  return get_buffer_pt (buf) == get_buffer_line_o (buf)
end

-- Return true if cursor is at the end of a line.
function end_of_line ()
  return get_buffer_pt (buf) - get_buffer_line_o (buf) == buffer_line_len (buf)
end
