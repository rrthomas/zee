-- Marker facility functions
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


-- Marker datatype

local function marker_new (o)
  local marker = {o = o}
  buf.markers[marker] = true
  return marker
end

function copy_marker (m)
  return marker_new (m.o)
end

function point_marker ()
  return marker_new (get_buffer_pt (buf))
end

function unchain_marker (marker)
  buf.markers[marker] = nil
end

-- Set the mark to point.
function set_mark ()
  if buf.mark then
    unchain_marker (buf.mark)
  end
  buf.mark = point_marker ()
end
