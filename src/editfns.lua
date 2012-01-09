-- Useful editing functions
--
-- Copyright (c) 2010-2012 Free Software Foundation, Inc.
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

-- Signal an error, and abort any ongoing macro definition.
function ding ()
  if thisflag.defining_macro then
    cancel_kbd_macro ()
  end

  if get_variable_bool ("ring-bell") and cur_wp then
    term_beep ()
  end
end


function is_empty_line ()
  return buffer_line_len (cur_bp) == 0
end

function is_blank_line ()
  return get_buffer_region (cur_bp, {start = get_buffer_line_o (cur_bp), finish = buffer_end_of_line (cur_bp, get_buffer_o (cur_bp))}).s:match ("^%s*$") ~= nil
end

-- Returns the character following point in the current buffer.
function following_char ()
  if eobp () then
    return nil
  elseif eolp () then
    return '\n'
  else
    return get_buffer_char (cur_bp, get_buffer_o (cur_bp))
  end
end

-- Return the character preceding point in the current buffer.
function preceding_char ()
  if bobp () then
    return nil
  elseif bolp () then
    return '\n'
  else
    return get_buffer_char (cur_bp, get_buffer_o (cur_bp) - 1)
  end
end

-- Return true if point is at the beginning of the buffer.
function bobp ()
  return get_buffer_o (cur_bp) == 0
end

-- Return true if point is at the end of the buffer.
function eobp (void)
  return get_buffer_o (cur_bp) == get_buffer_size (cur_bp)
end

-- Return true if point is at the beginning of a line.
function bolp ()
  return get_buffer_o (cur_bp) == get_buffer_line_o (cur_bp)
end

-- Return true if point is at the end of a line.
function eolp ()
  return get_buffer_o (cur_bp) - get_buffer_line_o (cur_bp) == buffer_line_len (cur_bp)
end
