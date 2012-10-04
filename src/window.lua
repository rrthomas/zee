-- Window handling functions
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

-- Window table:
-- {
--   topdelta: The top line delta from point.
--   lastpointn: The last point line number.
--   start_column: The start column of the window (>0 if scrolled sideways).
--   fwidth, fheight: The formal width and height of the window.
--   ewidth, eheight: The effective width and height of the window.
-- }

function window_top_visible (wp)
  return offset_to_line (buf, get_buffer_pt (buf)) == wp.topdelta
end

function window_bottom_visible (wp)
  return wp.all_displayed
end

function create_window ()
  local w, h = term_width (), term_height ()
  cur_wp = {topdelta = 0, start_column = 0, lastpointn = 0}
  cur_wp.fwidth = w
  cur_wp.ewidth = w
  -- Save space for minibuffer.
  cur_wp.fheight = h - 1
  -- Save space for status line.
  cur_wp.eheight = cur_wp.fheight - 1
end

function window_resync (wp)
  local n = offset_to_line (buf, get_buffer_pt (buf))
  local delta = n - wp.lastpointn

  if delta ~= 0 then
    if (delta > 0 and wp.topdelta + delta < wp.eheight) or (delta < 0 and wp.topdelta >= -delta) then
      wp.topdelta = wp.topdelta + delta
    elseif n > wp.eheight / 2 then
      wp.topdelta = math.floor (wp.eheight / 2)
    else
      wp.topdelta = n
    end
  end
  wp.lastpointn = n
end
