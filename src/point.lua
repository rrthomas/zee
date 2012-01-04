-- Point facility functions
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

function offset_to_point (bp, offset)
  local pt = {n = 0}
  local o = 0
  while estr_end_of_line (get_buffer_text (bp), o) < offset do
    pt.n = pt.n + 1
    o = estr_next_line (get_buffer_text (bp), o)
  end
  pt.o = offset - o
  return pt
end

function point_min ()
  return {n = 0, o = 0}
end

function point_max ()
  return offset_to_point (cur_bp, get_buffer_size (cur_bp))
end

function line_beginning_position (count)
  -- Copy current point position without offset (beginning of line).
  local o = get_buffer_o (cur_bp)

  count = count - 1
  while count < 0 and o > 0 do
    o = estr_prev_line (get_buffer_text (cur_bp), o)
    count = count + 1
  end

  while count > 0 and o do
    o = estr_next_line (get_buffer_text (cur_bp), o)
    count = count - 1
  end

  return offset_to_point (cur_bp, o)
end

function line_end_position (count)
  local pt = line_beginning_position (count)
  pt.o = get_buffer_line_len (cur_bp)
  return pt
end

-- Go to coordinates described by pt
function goto_point (pt)
  if cur_bp.pt.n > pt.n then
    repeat
      execute_function ("previous-line")
    until cur_bp.pt.n == pt.n
  elseif cur_bp.pt.n < pt.n then
    repeat
      execute_function ("next-line")
    until cur_bp.pt.n == pt.n
  end

  if cur_bp.pt.o > pt.o then
    repeat
      execute_function ("backward-char")
    until cur_bp.pt.o == pt.o
  elseif cur_bp.pt.o < pt.o then
    repeat
      execute_function ("forward-char")
    until cur_bp.pt.o == pt.o
  end
end
