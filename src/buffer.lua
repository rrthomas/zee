-- Buffer-oriented functions
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


-- Buffer methods that know about the gap.

function get_buffer_pre_point (bp)
  return bp.text:sub (1, get_buffer_pt (bp) - 1)
end

function get_buffer_post_point (bp)
  return bp.text:sub (get_buffer_pt (bp) + bp.gap)
end

function get_buffer_pt (bp)
  return bp.pt
end

local function set_buffer_pt (bp, o)
  if o < bp.pt then
    bp.text:move (o + bp.gap, o, bp.pt - o)
    bp.text:set (o, '\0', math.min (bp.pt - o, bp.gap))
  elseif o > bp.pt then
    bp.text:move (bp.pt, bp.pt + bp.gap, o - bp.pt)
    bp.text:set (o + bp.gap - math.min (o - bp.pt, bp.gap), '\0', math.min (o - bp.pt, bp.gap))
  end
  bp.pt = o
end

local function realo_to_o (bp, o)
  if o == nil then
    return o
  elseif o < bp.pt + bp.gap then
    return math.min (o, bp.pt)
  end
  return o - bp.gap
end

local function o_to_realo (bp, o)
  return o < bp.pt and o or o + bp.gap
end

function get_buffer_size (bp)
  return realo_to_o (bp, #bp.text + 1) - 1
end

function buffer_line_len (bp, o)
  o = o or get_buffer_line_o (bp)
  return realo_to_o (bp, bp.text:end_of_line (o_to_realo (bp, o))) -
    realo_to_o (bp, bp.text:start_of_line (o_to_realo (bp, o)))
end

-- Replace `del' chars after point with `as'.
local min_gap = 1024 -- Minimum gap size after resize
local max_gap = 4096 -- Maximum permitted gap size
function replace_astr (del, as)
  if buf.readonly then
    minibuf_error ("File is readonly")
    return false
  end

  local newlen = #as

  undo_save_block (buf.pt, del, newlen)

  -- Adjust gap.
  local oldgap = buf.gap
  local added_gap = 0
  if oldgap + del < newlen then
    -- If gap would vanish, open it to min_gap.
    added_gap = min_gap
    buf.text:insert (buf.pt, (#as + min_gap) - (buf.gap + del))
    buf.gap = min_gap
  elseif oldgap + del > max_gap + newlen then
    -- If gap would be larger than max_gap, restrict it to max_gap.
    buf.text:remove (buf.pt + newlen + max_gap, (oldgap + del) - (max_gap + newlen))
    buf.gap = max_gap
  else
    buf.gap = oldgap + del - newlen
  end

  -- Zero any new bit of gap not produced by insertion.
  if math.max (oldgap, newlen) + added_gap < buf.gap + newlen then
    buf.text:set (buf.pt + math.max (oldgap, newlen) + added_gap, '\0', newlen + buf.gap - math.max (oldgap, newlen) - added_gap)
  end

  -- Insert `newlen' chars.
  buf.text:replace (buf.pt, as)
  buf.pt = buf.pt + newlen -- FIXME: remove this & the next line!
  resync_buffer_line (buf)

  -- Adjust markers.
  for m in pairs (buf.markers) do
    if m.o > buf.pt - newlen then
      m.o = math.max (buf.pt - newlen, m.o + newlen - del)
    end
  end

  buf.modified = true
  if as:next_line (1) then
    thisflag.need_resync = true
  end
  return true
end

function insert_astr (as)
  local ok = replace_astr (0, as)
  return ok
  -- if ok then
  --   goto_offset (buf.pt + #as)
  -- end
end

function insert_string (s)
  return insert_astr (AStr (s))
end

function get_buffer_char (bp, o)
  local n = o_to_realo (bp, o)
  return string.char (bp.text[n])
end

function buffer_prev_line (bp, o)
  return realo_to_o (bp, bp.text:prev_line (o_to_realo (bp, o)))
end

function buffer_next_line (bp, o)
  return realo_to_o (bp, bp.text:next_line (o_to_realo (bp, o)))
end

function buffer_start_of_line (bp, o)
  return realo_to_o (bp, bp.text:start_of_line (o_to_realo (bp, o)))
end

function buffer_end_of_line (bp, o)
  return realo_to_o (bp, bp.text:end_of_line (o_to_realo (bp, o)))
end

function get_buffer_line_o (bp)
  return realo_to_o (bp, bp.text:start_of_line (o_to_realo (bp, bp.pt)))
end

-- Copy a region of text into an AStr.
function get_buffer_region (bp, r)
  local as = AStr (r.finish - r.start)
  if r.start < get_buffer_pt (bp) then
    as:replace (1, bp.text:sub (r.start, math.min (r.finish, get_buffer_pt (bp)) - 1))
  end
  if r.finish > get_buffer_pt (bp) then
    local n = r.start - get_buffer_pt (bp)
    local done = math.max (-n, 0)
    local from = math.max (n, 0)
    as:replace (done + 1, bp.text:sub (get_buffer_pt (bp) + bp.gap + from, bp.gap + r.finish - 1))
  end
  return as
end


-- Buffer methods that don't know about the gap.

-- Insert the character `c' at the current point position
-- into the current buffer.
function insert_char (c)
  local ok = replace_astr (0, AStr (c))
  -- if ok then
  --   move_char (1)
  -- end
end

function get_buffer_line (bp)
  return bp.line
end

function resync_buffer_line (bp)
  local n = 0
  local o = 1
  while buffer_end_of_line (bp, o) and buffer_end_of_line (bp, o) < get_buffer_pt (bp) do
    n = n + 1
    o = buffer_next_line (bp, o)
    assert (o)
  end
  bp.line = n
end

-- Get filename.
function get_buffer_filename (bp)
  return bp.filename
end


function delete_char ()
  execute_command ("edit-select-off")

  if eobp () then
    return minibuf_error ("End of buffer")
  end

  if eolp () then
    thisflag.need_resync = true
  end
  buf.modified = replace_astr (1, AStr (""))
  return buf.modified
end


-- Allocate a new buffer, set the default local variable values, and
-- insert it into the buffer list.
function buffer_new () -- FIXME: Constructor which we can pass other arguments
  return {pt = 1, line = 0, gap = 0, text = AStr (""),
          markers = setmetatable ({}, {__mode = "k"}),
          modified = false}
end

-- Make a region from two offsets
function region_new (o1, o2)
  return {start = math.min (o1, o2), finish = math.max (o1, o2)}
end

function get_region_size (rp)
  return rp.finish - rp.start
end

-- Return the selection
function calculate_the_selection ()
  if not buf.mark then
    return minibuf_error ("There is no selection")
  end

  return region_new (buf.pt, buf.mark.o)
end

function delete_region (r)
  goto_offset (r.start)
  replace_astr (get_region_size (r), AStr (""))
end

function in_region (o, x, r)
  return o + x >= r.start and o + x < r.finish
end


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

Define ("edit-select-on",
[[
Start selecting text.
]],
  function ()
    buf.mark = point_marker ()
  end
)

Define ("edit-select-off",
[[
Stop selecting text.
]],
  function ()
    buf.mark = nil
  end
)

-- Return a safe tab width.
function tab_width ()
  return math.max (tonumber (get_variable ("tab-width")), 1)
end

-- Return a safe indent width.
function indent_width ()
  return math.max (tonumber (get_variable ("indent-width")), 1)
end


-- Basic movement routines

function move_char (dir)
  local ltest, btest, lmove
  if dir >= 0 then
    ltest, btest, lmove = eolp, eobp, "move-start-line"
  else
    ltest, btest, lmove = bolp, bobp, "move-end-line"
  end

  if not ltest () then
    set_buffer_pt (buf, get_buffer_pt (buf) + dir)
  elseif not btest () then
    thisflag.need_resync = true
    bp.line = bp.line + dir
    set_buffer_pt (buf, get_buffer_pt (buf) + dir)
    execute_command (lmove)
  else
    return true
  end
end

-- Get the goal column, expanding tabs.
function get_goalc ()
  return #make_string_printable (get_line (), get_buffer_pt (buf) - get_buffer_line_o (buf))
end

function move_line (n)
  local dir, func = 1, buffer_next_line
  if n < 0 then
    dir, func = -1, buffer_prev_line
    n = -n
  end

  if _last_command ~= "move-next-line" and _last_command ~= "move-previous-line" then
    buf.goalc = get_goalc ()
  end

  while n > 0 do
    local o = func (buf, buf.pt)
    if o == nil then
      break
    end
    set_buffer_pt (buf, o)
    n = n - 1
    buf.line = buf.line + dir
  end

  set_buffer_pt (buf, get_buffer_line_o (buf) + #make_string_printable (get_line (), buf.goalc))
  thisflag.need_resync = true
  return n ~= 0
end

function goto_offset (o)
  local old_lineo = get_buffer_line_o (buf)
  set_buffer_pt (buf, o)
  if get_buffer_line_o (buf) ~= old_lineo then
    buf.goalc = get_goalc ()
    thisflag.need_resync = true
    resync_buffer_line (buf)
  end
end
