-- Marker facility functions
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


-- Marker datatype

local function marker_new (bp, o)
  local marker = {bp = bp, o = o}
  bp.markers[marker] = true
  return marker
end

-- FIXME: Replace with table.clone
function copy_marker (m)
  return marker_new (m.bp, m.o)
end

function point_marker ()
  return marker_new (cur_bp, get_buffer_o (cur_bp))
end

function unchain_marker (marker)
  if marker.bp then
    marker.bp.markers[marker] = nil
  end
end


-- Mark ring

local mark_ring = {} -- Mark ring.

-- Push the current mark to the mark-ring.
function push_mark ()
  -- Save the mark.
  if cur_bp.mark then
    table.insert (mark_ring, copy_marker (cur_bp.mark))
  else
    -- Save an invalid mark.
    table.insert (mark_ring, marker_new (cur_bp, 0))
  end
end

-- Pop a mark from the mark-ring and make it the current mark.
function pop_mark ()
  local m = mark_ring[#mark_ring]

  -- Replace the mark.
  if m.bp.mark then
    unchain_marker (m.bp.mark)
  end
  m.bp.mark = copy_marker (m)

  table.remove (mark_ring, #mark_ring)
  unchain_marker (m)
end

-- Set the mark to point.
function set_mark ()
  if cur_bp.mark then
    unchain_marker (cur_bp.mark)
  end
  cur_bp.mark = point_marker ()
end
