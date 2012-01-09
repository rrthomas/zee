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
  while estr_end_of_line (bp.text, o) < offset do
    pt.n = pt.n + 1
    o = estr_next_line (bp.text, o)
    assert (o)
  end
  pt.o = offset - o
  return pt
end

function goto_offset (o)
  local old_n = get_buffer_pt (cur_bp).n
  cur_bp.o = o
  if get_buffer_pt (cur_bp).n ~= old_n then
    cur_bp.goalc = get_goalc ()
    thisflag.need_resync = true
  end
end

-- Go to coordinates described by pt.
function goto_point (pt)
  goto_offset (point_to_offset (cur_bp, pt));
end
