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
  if warn_if_readonly_buffer () then
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
  buf.pt = buf.pt + newlen

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
  return replace_astr (0, as)
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
  return replace_astr (0, AStr (c))
end

function delete_char ()
  deactivate_mark ()

  if eobp () then
    return minibuf_error ("End of buffer")
  end

  if warn_if_readonly_buffer () then
    return false
  end

  if eolp () then
    thisflag.need_resync = true
  end
  replace_astr (1, AStr (""))

  buf.modified = true

  return true
end


-- Allocate a new buffer, set the default local variable values, and
-- insert it into the buffer list.
function buffer_new ()
  local bp = {}

  bp.pt = 1
  bp.gap = 0
  bp.text = AStr ("")
  bp.markers = {}
  init_buffer (bp)
  return bp
end

-- Initialise a buffer
function init_buffer (bp)
  if get_variable ("wrap-mode") then
    bp.wrap = true
  end
end

-- Get filename.
function get_buffer_filename (bp)
  return bp.filename
end

-- Set a new filename for the buffer.
function set_buffer_name (bp, filename)
  if filename[1] ~= '/' then
    filename = string.format ("%s/%s", posix.getcwd(), filename)
  end
  bp.filename = filename
end

-- Print an error message into the echo area and return true
-- if the current buffer is readonly; otherwise return false.
function warn_if_readonly_buffer ()
  if buf.readonly then
    minibuf_error ("File is readonly")
    return true
  end

  return false
end

function warn_if_no_mark ()
  if not buf.mark then
    minibuf_error ("The mark is not set now")
    return true
  elseif not buf.mark_active then
    minibuf_error ("The mark is not active now")
    return true
  end
  return false
end

-- Make a region from two offsets
function region_new (o1, o2)
  return {start = math.min (o1, o2), finish = math.max (o1, o2)}
end

function get_region_size (rp)
  return rp.finish - rp.start
end

-- Return the region between point and mark.
function calculate_the_region ()
  if warn_if_no_mark () then
    return nil
  end

  return region_new (buf.pt, buf.mark.o)
end

function delete_region (r)
  if warn_if_readonly_buffer () then
    return false
  end

  local m = point_marker ()
  goto_offset (r.start)
  replace_astr (get_region_size (r), AStr (""))
  goto_offset (m.o)
  unchain_marker (m)

  return true
end

function in_region (o, x, r)
  return o + x >= r.start and o + x < r.finish
end

function activate_mark ()
  buf.mark_active = true
end

function deactivate_mark ()
  buf.mark_active = false
end

-- Return a safe tab width.
function tab_width ()
  return math.max (tonumber (get_variable ("tab-width")), 1)
end


-- Basic movement routines

-- FIXME: Only needs to move ±1
function move_char (offset)
  local dir, ltest, btest, lmove
  if offset >= 0 then
    dir, ltest, btest, lmove = 1, eolp, eobp, "move-start-line"
  else
    dir, ltest, btest, lmove = -1, bolp, bobp, "move-end-line"
  end
  for i = 1, math.abs (offset) do
    if not ltest () then
      set_buffer_pt (buf, get_buffer_pt (buf) + dir)
    elseif not btest () then
      thisflag.need_resync = true
      set_buffer_pt (buf, get_buffer_pt (buf) + dir)
      execute_command (lmove)
    else
      return false
    end
  end

  return true
end

-- Go to the column `goalc'.  Take care of expanding tabulations.
function goto_goalc ()
  local col = 0

  local i = get_buffer_line_o (buf)
  local lim = get_buffer_line_o (buf) + buffer_line_len (buf)
  while i < lim do
    if col == buf.goalc then
      break
    elseif get_buffer_char (buf, i) == '\t' then
      local t = tab_width ()
      for w = t - col % t, 1, -1 do
        col = col + 1
        if col == buf.goalc then
          break
        end
      end
    else
      col = col + 1
    end
    i = i + 1
  end

  set_buffer_pt (buf, i)
end

function move_line (n)
  local func = buffer_next_line
  if n < 0 then
    n = -n
    func = buffer_prev_line
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
  end

  goto_goalc ()
  thisflag.need_resync = true

  return n == 0
end

function offset_to_line (bp, offset)
  local n = 0
  local o = 1
  while buffer_end_of_line (bp, o) and buffer_end_of_line (bp, o) < offset do
    n = n + 1
    o = buffer_next_line (bp, o)
    assert (o)
  end
  return n
end

function goto_offset (o)
  local old_lineo = get_buffer_line_o (buf)
  set_buffer_pt (buf, o)
  if get_buffer_line_o (buf) ~= old_lineo then
    buf.goalc = get_goalc ()
    thisflag.need_resync = true
  end
end
