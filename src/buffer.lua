-- Buffer-oriented functions
--
-- Copyright (c) 2010-2012 Free Software Foundation, Inc.
--
-- This file is part of GNU Zile.
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
  return realo_to_o (bp, bp.text:len ())
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

  local newlen = as:len ()

  undo_save_block (cur_bp.pt, del, newlen)

  -- Adjust gap.
  local oldgap = cur_bp.gap
  local added_gap = 0
  if oldgap + del < newlen then
    -- If gap would vanish, open it to min_gap.
    added_gap = min_gap
    cur_bp.text:insert (cur_bp.pt, (as:len () + min_gap) - (cur_bp.gap + del))
    cur_bp.gap = min_gap
  elseif oldgap + del > max_gap + newlen then
    -- If gap would be larger than max_gap, restrict it to max_gap.
    cur_bp.text:remove (cur_bp.pt + newlen + max_gap, (oldgap + del) - (max_gap + newlen))
    cur_bp.gap = max_gap
  else
    cur_bp.gap = oldgap + del - newlen
  end

  -- Zero any new bit of gap not produced by insertion.
  if math.max (oldgap, newlen) + added_gap < cur_bp.gap + newlen then
    cur_bp.text:set (cur_bp.pt + math.max (oldgap, newlen) + added_gap, '\0', newlen + cur_bp.gap - math.max (oldgap, newlen) - added_gap)
  end

  -- Insert `newlen' chars.
  cur_bp.text:replace (cur_bp.pt, tostring (as))
  cur_bp.pt = cur_bp.pt + newlen

  -- Adjust markers.
  for m in pairs (cur_bp.markers) do
    if m.o > cur_bp.pt - newlen then
      m.o = math.max (cur_bp.pt - newlen, m.o + newlen - del)
    end
  end

  cur_bp.modified = true
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
  return bp.text:sub (n, n)
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


-- Buffer methods that don't know about the gap.

-- Copy a region of text into an AStr.
function get_buffer_region (bp, r)
  local s = ""
  if r.start < get_buffer_pt (bp) then
    s = s .. get_buffer_pre_point (bp):sub (r.start, math.min (r.finish, get_buffer_pt (bp)))
  end
  if r.finish > get_buffer_pt (bp) then
    local from = math.max (r.start - get_buffer_pt (bp), 0)
    s = s .. get_buffer_post_point (bp):sub (from + 1, r.finish - get_buffer_pt (bp))
  end
  return AStr (s)
end

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

  cur_bp.modified = true

  return true
end

function insert_buffer (bp)
  -- Copy text to avoid problems when bp == cur_bp.
  insert_astr (AStr (get_buffer_pre_point (bp) .. get_buffer_post_point (bp)))
end


-- The buffer list
buffers = {}

buffer_name_history = history_new ()

-- Allocate a new buffer, set the default local variable values, and
-- insert it into the buffer list.
function buffer_new ()
  local bp = {}

  bp.pt = 1
  bp.gap = 0
  bp.text = AStr ("")
  bp.markers = {}
  bp.dir = posix.getcwd () or ""

  -- Insert into buffer list.
  table.insert (buffers, bp)

  init_buffer (bp)

  return bp
end

-- Initialise a buffer
function init_buffer (bp)
  if get_variable_bool ("auto-fill-mode") then
    bp.autofill = true
  end
end

-- Get filename, or buffer name if nil.
function get_buffer_filename_or_name (bp)
  return bp.filename or bp.name
end

-- Set a new filename, and from it a name, for the buffer.
function set_buffer_names (bp, filename)
  if filename[1] ~= '/' then
    filename = string.format ("%s/%s", posix.getcwd(), filename)
  end
  bp.filename = filename

  local s = posix.basename (filename)
  local name = s
  local i = 2
  while find_buffer (name) do
    name = string.format ("%s<%d>", s, i)
    i = i + 1
  end
  bp.name = name
end

-- Search for a buffer named `name'.
function find_buffer (name)
  for _, bp in ipairs (buffers) do
    if bp.name == name then
      return bp
    end
  end
end

-- Switch to the specified buffer.
function switch_to_buffer (bp)
  assert (cur_wp.bp == cur_bp)

  -- The buffer is the current buffer; return safely.
  if cur_bp == bp then
    return
  end

  -- Set current buffer.
  cur_bp = bp
  cur_wp.bp = cur_bp

  -- Move the buffer to head.
  for i = 1, #buffers do
    if buffers[i] == bp then
      table.remove (buffers, i)
      table.insert (buffers, bp)
      break
    end
  end

  -- Change to buffer's default directory
  posix.chdir (bp.dir)

  thisflag.need_resync = true
end

-- Print an error message into the echo area and return true
-- if the current buffer is readonly; otherwise return false.
function warn_if_readonly_buffer ()
  if cur_bp.readonly then
    minibuf_error (string.format ("Buffer is readonly: %s", cur_bp.name))
    return true
  end

  return false
end

function warn_if_no_mark ()
  if not cur_bp.mark then
    minibuf_error ("The mark is not set now")
    return true
  elseif not cur_bp.mark_active then
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

  return region_new (cur_bp.pt, cur_bp.mark.o)
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

-- Set the specified buffer's temporary flag and move the buffer
-- to the end of the buffer list.
function set_temporary_buffer (bp)
  bp.temporary = true

  for i = 1, #buffers do
    if buffers[i] == bp then
      table.remove (buffers, i)
      break
    end
  end
  table.insert (buffers, 1, bp)
end

function activate_mark ()
  cur_bp.mark_active = true
end

function deactivate_mark ()
  cur_bp.mark_active = false
end

-- Return a safe tab width for the given buffer.
function tab_width (bp)
  return math.max (get_variable_number_bp (bp, "tab-width"), 1)
end

function create_auto_buffer (name)
  local bp = buffer_new ()
  bp.name = name
  bp.needname = true
  bp.temporary = true
  bp.nosave = true
  return bp
end

function create_scratch_buffer ()
  return create_auto_buffer ("*scratch*")
end

-- Remove the specified buffer from the buffer list.
-- Recreate the scratch buffer when required.
function kill_buffer (kill_bp)
  -- Search for windows displaying the buffer to kill.
  for _, wp in ipairs (windows) do
    if wp.bp == kill_bp then
      wp.topdelta = 0
      wp.saved_pt = nil
    end
  end

  -- Remove the buffer from the buffer list.
  local next_bp = buffers[#buffers]
  for i = 1, #buffers do
    if buffers[i] == kill_bp then
      table.remove (buffers, i)
      next_bp = buffers[i > 1 and i - 1 or #buffers]
      if cur_bp == kill_bp then
        cur_bp = next_bp
      end
      break
    end
  end

  -- If no buffers left, recreate scratch buffer and point windows at
  -- it.
  if #buffers == 0 then
    table.insert (buffers, create_scratch_buffer ())
    cur_bp = buffers[1]
    for _, wp in ipairs (windows) do
      wp.bp = cur_bp
    end
  end

  -- Resync windows that need it.
  for _, wp in ipairs (windows) do
    if wp.bp == kill_bp then
      wp.bp = next_bp
      window_resync (wp)
    end
  end
end

function make_buffer_completion ()
  local cp = completion_new ()
  for _, bp in ipairs (buffers) do
    table.insert (cp.completions, bp.name)
  end

  return cp
end

-- Check if the buffer has been modified.  If so, asks the user if
-- he/she wants to save the changes.  If the response is positive, return
-- true, else false.
function check_modified_buffer (bp)
  if bp.modified and not bp.nosave then
    while true do
      local ans = minibuf_read_yesno (string.format ("Buffer %s modified; kill anyway? (yes or no) ", bp.name))
      if ans == nil then
        execute_function ("keyboard-quit")
        return false
      elseif not ans then
        return false
      end
      break
    end
  end

  return true
end


-- Basic movement routines

function move_char (offset)
  local dir, ltest, btest, lmove
  if offset >= 0 then
    dir, ltest, btest, lmove = 1, eolp, eobp, "beginning-of-line"
  else
    dir, ltest, btest, lmove = -1, bolp, bobp, "end-of-line"
  end
  for i = 1, math.abs (offset) do
    if not ltest () then
      set_buffer_pt (cur_bp, get_buffer_pt (cur_bp) + dir)
    elseif not btest () then
      thisflag.need_resync = true
      set_buffer_pt (cur_bp, get_buffer_pt (cur_bp) + dir)
      execute_function (lmove)
    else
      return false
    end
  end

  return true
end

-- Go to the column `goalc'.  Take care of expanding tabulations.
function goto_goalc ()
  local col = 0

  local i = get_buffer_line_o (cur_bp)
  local lim = get_buffer_line_o (cur_bp) + buffer_line_len (cur_bp)
  while i < lim do
    if col == cur_bp.goalc then
      break
    elseif get_buffer_char (cur_bp, i) == '\t' then
      local t = tab_width (cur_bp)
      for w = t - col % t, 1, -1 do
        col = col + 1
        if col == cur_bp.goalc then
          break
        end
      end
    else
      col = col + 1
    end
    i = i + 1
  end

  set_buffer_pt (cur_bp, i)
end

function move_line (n)
  local func = buffer_next_line
  if n < 0 then
    n = -n
    func = buffer_prev_line
  end

  if _last_command ~= "next-line" and _last_command ~= "previous-line" then
    cur_bp.goalc = get_goalc ()
  end

  while n > 0 do
    local o = func (cur_bp, cur_bp.pt)
    if o == nil then
      break
    end
    set_buffer_pt (cur_bp, o)
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
  local old_lineo = get_buffer_line_o (cur_bp)
  set_buffer_pt (cur_bp, o)
  if get_buffer_line_o (cur_bp) ~= old_lineo then
    cur_bp.goalc = get_goalc ()
    thisflag.need_resync = true
  end
end
